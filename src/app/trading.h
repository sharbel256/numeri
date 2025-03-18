#include "httpclient.h"
#include "websocketclient.h"
#include "orderbook.h"
#include <memory>
#include <mutex>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <jwt-cpp/jwt.h>

class Trading {
public:
    std::mutex orderBooksMutex;

    Trading(net::io_context& ioc_, std::atomic<bool>& shutdownFlag_);

    void getAccounts();
    void startWebsocket();
    void shutdown();

    std::map<std::string, std::shared_ptr<trading::OrderBook>>& getOrderbooks();

    // New API functions
    std::string previewOrder(const std::string& requestBody);
    std::string createOrder(const std::string& queryParams = "");
    std::string getOrder(const std::string& orderId);
    std::string listOrders(const std::string& queryParams = "");
    std::string listFills(const std::string& queryParams = "");
    std::string batchCancelOrder(const std::string& requestBody);
    std::string editOrder(const std::string& requestBody);
    std::string editOrderPreview(const std::string& requestBody);
    std::string closePosition(const std::string& requestBody);
    

private:
    std::atomic<bool>& shutdownFlag;
    net::io_context& ioc;
    ssl::context ctx;

    std::shared_ptr<HTTPClient> http_client;
    std::shared_ptr<WebSocketClient> websocket_client;

    std::map<std::string, std::shared_ptr<trading::OrderBook>> orderBooks;
    
    void processLoginData(const std::string& response);
    void processData(const std::string& data);
    std::string getTimestamp();
    std::string calculateSignature(const std::string& message, const std::string& secretKey);
    std::string create_jwt(std::string method, std::string path);
    std::string websocket_jwt();

    // Helper to generate auth headers
    std::map<std::string,std::string> generateAuthHeaders(
        const std::string& method, 
        const std::string& path, 
        const std::string& body = ""
    );
};

using json = nlohmann::json;

Trading::Trading(net::io_context& ioc_, std::atomic<bool>& shutdownFlag_)
: ioc(ioc_), shutdownFlag(shutdownFlag_),ctx(ssl::context::tlsv13_client) {
    ctx.set_default_verify_paths();
    ctx.set_verify_mode(ssl::verify_peer);
    http_client = std::make_shared<HTTPClient>(ioc, ctx);
}

std::map<std::string, std::shared_ptr<trading::OrderBook>>& Trading::getOrderbooks() {
    std::cout << "Trading::getOrderbooks()" << std::endl;
    return orderBooks;
}

void Trading::getAccounts() {
    auto host = "api.coinbase.com";
    auto port = "443";

    try {
        std::string method = "GET";
        std::string requestPath = "/api/v3/brokerage/accounts";
        std::string body = "";

        auto headers = generateAuthHeaders(method, requestPath, body);

        std::string response = http_client->get(host, port, requestPath, headers);
        processLoginData(response);
    } catch (std::exception const& e) {
        std::cerr << "Error in login: " << e.what() << std::endl;
    }
}

void Trading::startWebsocket()
{
    std::cout << "Trading::startWebsocket()" << std::endl;
    auto host = "advanced-trade-ws.coinbase.com";
    auto port = "443";

    // std::string apiKey = std::getenv("COINBASE_API_KEY") ? std::getenv("COINBASE_API_KEY") : "";
    // std::string secretKey = std::getenv("COINBASE_SECRET_KEY") ? std::getenv("COINBASE_SECRET_KEY") : "";

    std::string type = "subscribe";
    std::vector<std::string> product_ids = {"BTC-USD"};
    std::string channel = "level2";
    // std::string timestamp = getTimestamp();

    // std::string message = timestamp + channel + product_ids[0];
    // std::string signature = calculateSignature(message, secretKey);

    std::string jwt_token = websocket_jwt();

    json j;
    j["type"] = type;
    j["product_ids"] = product_ids;
    j["channel"] = channel;
    // j["signature"] = signature;
    // j["api_key"] = apiKey;
    // j["timestamp"] = timestamp;

    auto text = j.dump();

    try {
        ssl::context ctx{ssl::context::tlsv13_client};
        ctx.set_default_verify_paths();

        websocket_client = std::make_shared<WebSocketClient>(ioc, ctx);

        // Add the JWT token to WebSocket headers
        websocket_client->setHeaders({
            {"Authorization", "Bearer " + jwt_token}
        });

        websocket_client->setReadCallback([this](const std::string& data) {
            processData(data);
        });

        websocket_client->run(host, port, text);
        std::cout << "websocket client running" << std::endl;

    } catch (std::exception const& e) {
        std::cerr << "Error in startWebsocket: " << e.what() << std::endl;
    }
}

void Trading::processLoginData(const std::string& response)
{
    json jsonResponse = json::parse(response);
    auto accountsArray = jsonResponse["accounts"];
    
    // Access balances as needed
    for (auto& accountObject : accountsArray) {
        auto balanceObject = accountObject["available_balance"];
        std::string balance = balanceObject["value"];
        std::cout << "Account: " << accountObject["currency"] << " Balance: " << balance << std::endl;
    }
}

void Trading::processData(const std::string& data)
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

                // Parse event time
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
                
                std::lock_guard<std::mutex> lock(orderBooksMutex);
                if (orderBooks.find(productId) != orderBooks.end()) {
                    orderBooks[productId]->update(entry);
                }
            }
        }
    } catch (const json::parse_error& e) {
        std::cerr << "Failed to parse JSON data: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error processing data: " << e.what() << std::endl;
    }
}

std::string Trading::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration);

    double seconds = micros.count() / 1e6; // Converting microseconds to seconds.

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(0) << seconds;
    return ss.str();
}

std::string Trading::calculateSignature(const std::string& message, const std::string& secretKey)
{
    // Just delegate to the HTTPClient's method if you want consistency
    // Or implement similarly here.
    unsigned char hmacResult[EVP_MAX_MD_SIZE];
    unsigned int hmacLength;

    HMAC(EVP_sha256(), secretKey.c_str(), (int)secretKey.size(),
         reinterpret_cast<const unsigned char*>(message.c_str()), (int)message.size(),
         hmacResult, &hmacLength);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for(unsigned int i = 0; i < hmacLength; ++i)
        ss << std::setw(2) << (unsigned)hmacResult[i];

    return ss.str();
}


std::map<std::string,std::string> Trading::generateAuthHeaders(
    const std::string& method, 
    const std::string& path, 
    const std::string& body
) {
    std::map<std::string,std::string> headers = {
        {"Authorization", "Bearer " + create_jwt(method, path)},
        {"Content-Type", "application/json"}
    };
    return headers;
}

std::string Trading::create_jwt(std::string method, std::string path) {
    // Set request parameters
    std::string key_name = std::getenv("COINBASE_API_KEY") ? std::getenv("COINBASE_API_KEY") : "";
    std::string key_secret = std::getenv("COINBASE_SECRET_KEY") ? std::getenv("COINBASE_SECRET_KEY") : "";
    std::string url = "api.coinbase.com";
    std::string uri = method + " " + url + path;

    // Generate a random nonce
    unsigned char nonce_raw[16];
    RAND_bytes(nonce_raw, sizeof(nonce_raw));
    std::string nonce(reinterpret_cast<char*>(nonce_raw), sizeof(nonce_raw));

    // create timestamp
    auto now = std::chrono::system_clock::now();

    // Create JWT token
    auto token = jwt::create()
        .set_subject(key_name)
        .set_issuer("cdp")
        .set_not_before(now)
        .set_expires_at(now + std::chrono::seconds{120})
        .set_payload_claim("uri", jwt::claim(uri))
        .set_header_claim("kid", jwt::claim(key_name))
        .set_header_claim("nonce", jwt::claim(nonce))
        .sign(jwt::algorithm::es256(key_name, key_secret));
    return token;
};

std::string Trading::websocket_jwt() {
    // Set request parameters
    std::string key_name = std::getenv("COINBASE_API_KEY") ? std::getenv("COINBASE_API_KEY") : "";
    std::string key_secret = std::getenv("COINBASE_SECRET_KEY") ? std::getenv("COINBASE_SECRET_KEY") : "";

    if (key_secret.empty()) {
        throw std::runtime_error("Secret key is not set in the environment variables.");
    }

    // Generate a random nonce
    unsigned char nonce_raw[16];
    RAND_bytes(nonce_raw, sizeof(nonce_raw));
    std::string nonce(reinterpret_cast<char*>(nonce_raw), sizeof(nonce_raw));

    // create timestamp
    auto now = std::chrono::system_clock::now();

    // Create JWT token
    auto token = jwt::create()
        .set_subject(key_name)
        .set_issuer("cdp")
        .set_not_before(now)
        .set_expires_at(now + std::chrono::seconds{120})
        .set_header_claim("kid", jwt::claim(key_name))
        .set_header_claim("nonce", jwt::claim(nonce))
        .sign(jwt::algorithm::es256(key_name, key_secret));

    return token;
};


void Trading::shutdown()
{
    // Here we don't have async clients anymore for HTTP, just close the WebSocket if needed.
    if (websocket_client) {
        websocket_client->close();
    }

    shutdownFlag = true;

    std::lock_guard<std::mutex> lock(orderBooksMutex);
    for (auto& pair : orderBooks) {
        pair.second->stopSnapshotThread();
    }
}


// ---------------------------------------------------------
// New Endpoint Functions
// ---------------------------------------------------------

// Preview an order (POST)
std::string Trading::previewOrder(const std::string& requestBody) {
    std::string method = "POST";
    std::string path = "/api/v3/brokerage/orders/preview";
    auto headers = generateAuthHeaders(method, path, requestBody);
    std::cout << "preview order" << std::endl;
    return http_client->post("api.coinbase.com", "443", path, requestBody, headers);
}

// Create an order (POST) - per user’s request
std::string Trading::createOrder(const std::string& requestBody) {
    std::string method = "POST";
    std::string path = "/api/v3/brokerage/orders";
    auto headers = generateAuthHeaders(method, path, requestBody);
    std::cout << "Creating order" << std::endl;
    if (!http_client) {
        throw std::runtime_error("http_client is null");
    }
    return http_client->post("api.coinbase.com", "443", path, requestBody, headers);
}

// Get order (GET)
std::string Trading::getOrder(const std::string& orderId) {
    std::string method = "GET";
    std::string path = "/api/v3/brokerage/orders/historical/" + orderId;
    auto headers = generateAuthHeaders(method, path);
    return http_client->get("api.coinbase.com", "443", path, headers);
}

// List orders (GET)
std::string Trading::listOrders(const std::string& queryParams) {
    std::string method = "GET";
    std::string path = "/api/v3/brokerage/orders/historical/batch";
    std::string fullPath = path;
    if (!queryParams.empty()) {
        fullPath += "?" + queryParams;
    }
    auto headers = generateAuthHeaders(method, fullPath);
    return http_client->get("api.coinbase.com", "443", fullPath, headers);
}

// List fills (GET)
std::string Trading::listFills(const std::string& queryParams) {
    std::string method = "GET";
    std::string path = "/api/v3/brokerage/orders/historical/fills";
    std::string fullPath = path;
    if (!queryParams.empty()) {
        fullPath += "?" + queryParams;
    }
    auto headers = generateAuthHeaders(method, fullPath);
    return http_client->get("api.coinbase.com", "443", fullPath, headers);
}

// Batch cancel order (POST)
std::string Trading::batchCancelOrder(const std::string& requestBody) {
    std::string method = "POST";
    std::string path = "/api/v3/brokerage/orders/batch_cancel";
    auto headers = generateAuthHeaders(method, path, requestBody);
    return http_client->post("api.coinbase.com", "443", path, requestBody, headers);
}

// Edit order (POST)
std::string Trading::editOrder(const std::string& requestBody) {
    std::string method = "POST";
    std::string path = "/api/v3/brokerage/orders/edit";
    auto headers = generateAuthHeaders(method, path, requestBody);
    return http_client->post("api.coinbase.com", "443", path, requestBody, headers);
}

// Edit order preview (POST)
std::string Trading::editOrderPreview(const std::string& requestBody) {
    std::string method = "POST";
    std::string path = "/api/v3/brokerage/orders/edit_preview";
    auto headers = generateAuthHeaders(method, path, requestBody);
    return http_client->post("api.coinbase.com", "443", path, requestBody, headers);
}

// Close position (POST)
std::string Trading::closePosition(const std::string& requestBody) {
    std::string method = "POST";
    std::string path = "/api/v3/brokerage/orders/close_position";
    auto headers = generateAuthHeaders(method, path, requestBody);
    return http_client->post("api.coinbase.com", "443", path, requestBody, headers);
}