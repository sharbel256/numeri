#include <iostream>
#include "websocketclient.h"
#include "httpclient.h"
#include "orderbook.h"
#include <iostream>
#include <string>
#include <thread>
#include <nlohmann/json.hpp>
#include <atomic>
#include <mutex>

using json = nlohmann::json;
using namespace std;

class Trading {
  private:
    std::atomic<bool>& shutdownFlag;
    net::io_context& ioc;
    net::io_context ws_ioc;
    net::io_context http_ioc;

    std::unique_ptr<std::thread> ws_io_thread;
    std::unique_ptr<std::thread> http_io_thread;

    std::shared_ptr<WebSocketClient> websocket_client;
    std::shared_ptr<HTTPClient> http_client;

    std::map<std::string, std::shared_ptr<trading::OrderBook>> orderBooks;

    
  public:
    std::mutex orderBooksMutex;
    // Modified constructor
    Trading(net::io_context& ioc_, std::atomic<bool>& shutdownFlag_)
        : ioc(ioc_), shutdownFlag(shutdownFlag_) {}

    // get orderbooks
    std::map<std::string, std::shared_ptr<trading::OrderBook>>& getOrderbooks() {
        std::cout << "Trading::getOrderbooks()" << std::endl;
        return orderBooks;
    }

    void login() {
        auto host = "api.coinbase.com";
        auto port = "https";

        try {
            // The SSL context is required, and holds certificates
            ssl::context ctx{ssl::context::tlsv13_client};
            ctx.set_default_verify_paths();

            http_client = std::make_shared<HTTPClient>(net::make_strand(ioc), ctx);

            http_client->setReadCallback([this](const std::string& data) {
                processLoginData(data);
            });

            http_client->run(host, port);
            cout << "http client running" << endl;
            // http_io_thread = boost::make_unique<std::thread>([this](){ http_ioc.run(); });

        } catch (std::exception const& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    void liveFunction()
    {
        std::cout << "Trading::liveFunction()" << std::endl;
        auto host = "advanced-trade-ws.coinbase.com";
        auto port = "443";

        std::string apiKey = std::getenv("COINBASE_API_KEY");
        std::string secretKey = std::getenv("COINBASE_SECRET_KEY");

        std::string type = "subscribe";
        std::vector<std::string> product_ids = {"BTC-USD"};
        std::string channel = "level2";
        std::string timestamp = getTimestamp();

        std::string message = timestamp + channel + product_ids[0];
        std::string signature = calculateSignature(message, secretKey);

        nlohmann::json j;
        j["type"] = type;
        j["product_ids"] = product_ids;
        j["channel"] = channel;
        j["signature"] = signature;
        j["api_key"] = apiKey;
        j["timestamp"] = timestamp;

        auto text = j.dump();
        std::cout << text << std::endl;

        try {
            // The SSL context is required, and holds certificates
            ssl::context ctx{ssl::context::tlsv13_client};
            ctx.set_default_verify_paths();

            websocket_client = std::make_shared<WebSocketClient>(ioc, ctx);

            websocket_client->setReadCallback([this](const std::string& data) {
                processData(data);
            });

            websocket_client->run(host, port, text);
            std::cout << "websocket client running" << std::endl;
            // ws_io_thread = boost::make_unique<std::thread>([this](){ ws_ioc.run(); });

        } catch (std::exception const& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    void processLoginData(const std::string& response)
    {
        std::cout << "processing login data" << std::endl;
        json jsonResponse = json::parse(response);
        auto accountsArray = jsonResponse["accounts"];
        
        // Accessing the first account (ETH)
        auto accountObject = accountsArray[0];
        auto balanceObject = accountObject["available_balance"];
        std::string balance = balanceObject["value"];
        
        // Accessing the second account (cash)
        auto accountObject1 = accountsArray[1];
        balanceObject = accountObject1["available_balance"];
        balance = balanceObject["value"];
        
        // Accessing the third account (BTC)
        auto accountObject2 = accountsArray[2];
        balanceObject = accountObject2["available_balance"];
        balance = balanceObject["value"];
    }


    void processData(const std::string& data)
    {
        try {
            json doc = json::parse(data);
            std::string channel = doc["channel"];
            if (channel != "l2_data") {
                return; // We're only interested in l2_data channel
            }
            
            auto events = doc["events"];
            for (const auto& event : events) {
                if (event["type"] != "update") {
                    continue;
                }
                
                std::string productId = event["product_id"];
                auto updates = event["updates"];
                
                for (const auto& update : updates) {
                    trading::OrderBookEntry entry;
                    entry.side = update["side"];
                    
                    // Parse ISO 8601 date string to std::chrono::system_clock::time_point
                    std::string eventTimeStr = update["event_time"].get<std::string>();
                    std::istringstream ss(eventTimeStr);
                    ss.imbue(std::locale("C"));
                    std::tm tm = {};
                    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
                    if (ss.fail()) {
                        std::cerr << "Failed to parse date: " << eventTimeStr << std::endl;
                        continue;
                    }
                    entry.eventTime = std::chrono::system_clock::from_time_t(std::mktime(&tm));

                    entry.priceLevel = std::stod(update["price_level"].get<std::string>());
                    entry.quantity = std::stod(update["new_quantity"].get<std::string>());
                    
                    orderBooks[productId]->update(entry);
                }
            }
        } catch (const json::parse_error& e) {
            std::cerr << "Failed to parse JSON data: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error processing data: " << e.what() << std::endl;
        }
    }

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration);

        double seconds = micros.count() / 1e6; // Converting microseconds to seconds.

        std::ostringstream ss;
        ss << std::fixed << std::setprecision(0) << seconds; // Set precision to 6 decimal places.

        return ss.str();
    }

    std::string calculateSignature(const std::string& message, const std::string& secretKey)
    {
        unsigned char hmacResult[EVP_MAX_MD_SIZE];
        unsigned int hmacLength;

        HMAC(EVP_sha256(), secretKey.c_str(), secretKey.length(),
            reinterpret_cast<const unsigned char*>(message.c_str()), message.length(), 
            hmacResult, &hmacLength);

        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for(unsigned int i = 0; i < hmacLength; ++i)
            ss << std::setw(2) << static_cast<unsigned>(hmacResult[i]);

        return ss.str();
    }

    // New shutdown method
    void shutdown()
    {
        // Close the clients
        if (websocket_client) {
            websocket_client->close();
        }
        if (http_client) {
            http_client->close();
        }

        // Set the shutdown flag
        shutdownFlag = true;

        // Stop all order books
        {
            std::lock_guard<std::mutex> lock(orderBooksMutex);
            for (auto& pair : orderBooks) {
                pair.second->stopSnapshotThread();
                // Optional: Reset shared_ptrs if no longer needed
                // pair.second.reset();
            }
        }
    }
};