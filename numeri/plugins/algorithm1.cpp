#include <boost/lockfree/queue.hpp>
#include <model.hpp>
#include <plugin_interface.hpp>

#include <chrono>
#include <iostream>
#include <thread>

using namespace trading;

// forward-declare the consumer helper so Algorithm1 can call it from execute()
void on_orderbook_event(const trading::OrderBookSnapshot& snap);

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
    trading::OrderBookSnapshot snap;
    while (running_.load(std::memory_order_acquire)) {
      // Drain available snapshots in a tight loop
      while (l2_out->pop(snap)) {
        on_orderbook_event(snap);
      }

      // Backoff briefly to avoid spinning when idle; tune as needed
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

  void stop() override {
    running_.store(false, std::memory_order_release);
  }

private:
  boost::lockfree::queue<trading::OrderBookSnapshot>* l2_out;
  boost::lockfree::queue<trading::Metric*>* metrics_out;
  std::atomic<bool> running_{true};
};

// Safe consumer helper (uses the fast/slow path described in design notes)
void on_orderbook_event(const trading::OrderBookSnapshot& snap) {
  // Fast path: snapshot points to an active book with matching version
  const trading::OrderBook* book = snap.book;
  if (book != nullptr && book->version == snap.version) {
    double bid = book->best_bid();
    double ask = book->best_ask();
    std::cout << "fast-path bid=" << bid << " ask=" << ask << std::endl;
    return;
  }

  // Slow path: fallback to the global latest book
  const trading::OrderBook* latest = trading::current.load(std::memory_order_acquire);
  if (!latest)
    return;
  double bid = latest->best_bid();
  double ask = latest->best_ask();
  std::cout << "slow-path bid=" << bid << " ask=" << ask << std::endl;
}

EXPORT_PLUGIN(Algorithm1)