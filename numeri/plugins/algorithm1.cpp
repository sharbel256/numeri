#include <boost/lockfree/queue.hpp>
#include <model.hpp>
#include <plugin_interface.hpp>

#include <chrono>
#include <iostream>
#include <thread>

using namespace trading;

class Algorithm1 : public IPlugin {
public:
  std::string getName() const override {
    return "Algorithm1";
  }

  void init(const trading::PluginConfig& config, const nlohmann::json& params) override {
    this->l2_out = config.l2_out;
    this->metrics_out = config.metrics_out;
    return;
  }

  void execute() override {
    while (running_.load(std::memory_order_acquire)) {
      trading::OrderBookReady* event;
      if (l2_out->pop(event)) {
        std::cout << "received message: " << event->timestamp_ns << std::endl;
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  void stop() override {
    running_.store(false, std::memory_order_release);
  }

private:
  boost::lockfree::queue<trading::OrderBookReady*>* l2_out;
  boost::lockfree::queue<trading::Metric*>* metrics_out;
  std::atomic<bool> running_{true};
};

EXPORT_PLUGIN(Algorithm1)