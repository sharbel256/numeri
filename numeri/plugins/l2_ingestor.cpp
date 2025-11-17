#include <boost/lockfree/queue.hpp>
#include <model.hpp>
#include <plugin_interface.hpp>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

using namespace trading;

class L2Ingestor : public IPlugin {
public:
  std::string getName() const override {
    return "L2 ingestor";
  }

  void init(const trading::PluginConfig& config, const nlohmann::json& params) override {
    std::cout << "initialized l2 ingestor with params " << params << std::endl;
    this->l2_out = config.l2_out;
    return;
  }

  void execute() override {
    trading::OrderBookReady book;
    while (true) {
      std::cout << "Hello from L2 ingestor!" << std::endl;
      heartbeat();
      std::this_thread::sleep_for(std::chrono::seconds(1));
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

  inline uint64_t now_ns() {
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
  }

private:
  trading::OrderBook book_a, book_b;
  trading::OrderBook* active = &book_a;
  trading::OrderBook* inactive = &book_b;
  boost::lockfree::queue<trading::OrderBookReady*>* l2_out;
};

EXPORT_PLUGIN(L2Ingestor)