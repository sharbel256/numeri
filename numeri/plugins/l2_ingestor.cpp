#include <boost/lockfree/queue.hpp>
#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/traits.h>
#include <model.hpp>
#include <nlohmann/json.hpp>
#include <plugin_interface.hpp>
#include <websocket_client.hpp>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>
#include <vector>

using namespace trading;
using json = nlohmann::json;

class L2Ingestor : public IPlugin {
public:
  std::string getName() const override {
    return "l2_ingestor";
  }

  void init(const trading::PluginConfig& config, const nlohmann::json& params) override {
    std::cout << "initialized l2 ingestor with params " << params << std::endl;
    this->l2_out = config.l2_out;
    // store shared io_context and ssl context provided by Kernel
    this->ioc_ = config.ioc;
    this->ssl_ctx_ = config.ssl_ctx;

    // read plugin params
    this->host = params.value("host", std::string(""));
    this->port = params.value("port", std::string(""));
    if (params.contains("products")) {
      this->products = params["products"].get<std::vector<std::string>>();
    }
    this->type = params.value("type", std::string(""));
    this->channel = params.value("channel", std::string(""));
    return;
  }

  void execute() override {
    start_websocket();
    // while (running_.load(std::memory_order_acquire)) {
    //   heartbeat();
    //   std::this_thread::sleep_for(std::chrono::seconds(1));
    // }
  }

  void stop() override {
    running_.store(false, std::memory_order_release);
    if (websocket_client) {
      try {
        websocket_client->close();
      } catch (...) {}
    }
  }

  void heartbeat() {
    auto* event = new trading::OrderBookReady{.book = nullptr,
                                              .version = active->version.load(),
                                              .timestamp_ns = now_ns()};
    while (!l2_out->push(event)) {
      std::cout << "[ingestor]: l2_out full" << std::endl;
      OrderBookReady* event;
      l2_out->pop(event);
    }
  }

  void start_websocket() {
    std::cout << "start_websocket()" << std::endl;
    std::string jwt_token = websocket_jwt();
    json j;
    j["type"] = this->type;
    j["product_ids"] = this->products;
    j["channel"] = this->channel;

    std::cout << "Hello" << std::endl;
    auto text = j.dump();

    try {
      // use the shared ssl context from Kernel if provided, otherwise create a local one
      if (!ssl_ctx_) {
        ssl_ctx_ = std::make_shared<ssl::context>(ssl::context::tlsv13_client);
        ssl_ctx_->set_default_verify_paths();
      }

      if (!ioc_) {
        throw std::runtime_error("io_context not provided to plugin");
      }

      websocket_client = std::make_shared<WebSocketClient>(*ioc_, *ssl_ctx_);

      // Add the JWT token to WebSocket headers
      websocket_client->setHeaders({{"Authorization", "Bearer " + jwt_token}});

      websocket_client->setReadCallback([this](const std::string& data) { processData(data); });

      websocket_client->run(this->host, this->port, text);
      std::cout << "websocket client running" << std::endl;

    } catch (std::exception const& e) {
      std::cerr << "Error in startWebsocket: " << e.what() << std::endl;
    }
  }

  void processData(const std::string& data) {
    auto doc = json::parse(data);
    std::cout << doc << std::endl;
  }

  std::string websocket_jwt() {
    // Set request parameters
    std::string key_name = std::getenv("COINBASE_API_KEY") ? std::getenv("COINBASE_API_KEY") : "";
    std::string key_secret =
        std::getenv("COINBASE_SECRET_KEY") ? std::getenv("COINBASE_SECRET_KEY") : "";
    std::cout << "Hello 1" << std::endl;

    if (key_secret.empty()) {
      throw std::runtime_error("Secret key is not set in the environment variables.");
    }

    std::cout << "Hello 2" << std::endl;
    // create timestamp
    auto now = std::chrono::system_clock::now();

    // Create JWT token
    auto token = jwt::create<jwt::traits::nlohmann_json>()
                     .set_subject(key_name)
                     .set_issuer("cdp")
                     .set_not_before(now)
                     .set_expires_at(now + std::chrono::seconds{120})
                     .set_header_claim("kid", key_name)
                     .sign(jwt::algorithm::es256(key_name, key_secret));

    std::cout << "Hello 3" << std::endl;

    return token;
  };

  inline uint64_t now_ns() {
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
  }

private:
  std::string host;
  std::string port;
  std::string type;
  std::string channel;
  std::vector<std::string> products;
  int window_seconds;

  trading::OrderBook book_a, book_b;
  trading::OrderBook* active = &book_a;
  trading::OrderBook* inactive = &book_b;
  boost::lockfree::queue<trading::OrderBookReady*>* l2_out;

  std::shared_ptr<WebSocketClient> websocket_client;
  std::shared_ptr<boost::asio::io_context> ioc_;
  std::shared_ptr<boost::asio::ssl::context> ssl_ctx_;
  std::atomic<bool> running_{true};
};

EXPORT_PLUGIN(L2Ingestor)