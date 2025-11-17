#pragma once

#include <boost/lockfree/queue.hpp>
#include <nlohmann/json.hpp>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <map>

namespace trading {

enum class Side { Buy, Sell };
enum class Status { Ack, Fill, Reject, Cancel };
enum class MetricType { MidPrice, Imbalance, VWAP, Depth10 };

struct Message {
  uint32_t type; // e.g., ORDERBOOK_DELTA, ORDER_REQUEST
  uint64_t sender_id;
  std::byte payload[128];
};

struct Plugin {
  using Handler = void (*)(const Message&);
  virtual void init() = 0;
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual void handle_message(const Message&) = 0;
  virtual ~Plugin() = default;
};

struct OrderBook {
  size_t max_depth = 100;
  std::chrono::milliseconds time_window{500};

  std::map<double, double, std::greater<double>> bids;
  std::map<double, double> asks;

  uint64_t sequence = 0;
  std::atomic<uint64_t> version{0};
  std::chrono::steady_clock::time_point last_update;

  double best_bid() const {
    return bids.empty() ? 0.0 : bids.begin()->first;
  }
  double best_ask() const {
    return asks.empty() ? 0.0 : asks.begin()->first;
  }
};

struct OrderBookReady {
  std::shared_ptr<OrderBook> book;
  uint64_t version;
  uint64_t timestamp_ns;
};

struct OrderFill {
  std::string client_order_id;
  std::string exchange_order_id;
  Status status;
  double filled_qty;
  double filled_price;
  int64_t timestamp_ns;
};

struct OrderRequest {
  std::string symbol;
  Side side;
  double price;
  double qty;
  std::string client_order_id;
  int64_t timestamp_ns;
};

struct Metric {
  std::string symbol;
  MetricType type;
  double value;
  int64_t timestamp_ns;
};

struct PluginConfig {
  boost::lockfree::queue<trading::OrderBookReady*>* l2_out = nullptr;
  boost::lockfree::queue<trading::Metric*>* metrics_out = nullptr;
  boost::lockfree::queue<trading::OrderRequest*>* order_out = nullptr;
  boost::lockfree::queue<trading::OrderFill*>* fill_in = nullptr;

  nlohmann::json params;
};

} // namespace trading