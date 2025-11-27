#pragma once

#include <boost/lockfree/queue.hpp>
#include <nlohmann/json.hpp>
#include <websocket_client.hpp>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <map>
#include <type_traits>

namespace net = boost::asio;
namespace ssl = boost::asio::ssl;

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
  alignas(64) std::map<double, double, std::greater<double>> bids;
  alignas(64) std::map<double, double> asks;

  uint64_t sequence{0};
  uint64_t version{0};
  std::chrono::steady_clock::time_point last_update;
  size_t max_depth{100};

  double best_bid() const {
    return bids.empty() ? 0.0 : bids.begin()->first;
  }
  double best_ask() const {
    return asks.empty() ? 0.0 : asks.begin()->first;
  }

  void reset() {
    bids.clear();
    asks.clear();
    sequence = 0;
    version = 0;
  }
};

struct OrderBookSnapshot {
  const OrderBook* book{nullptr};
  uint64_t version{0};
  uint64_t timestamp_ns{0};
};

static_assert(std::is_trivially_copyable_v<OrderBookSnapshot>,
              "OrderBookSnapshot must be trivially copyable for lockfree queues");

// Global pointer to the currently published OrderBook. Consumers may fallback
// to this when a snapshot's backing buffer was recycled. Use an inline
// variable so the symbol is available to all translation units (including
// plugins built as shared libraries) without needing a separate .cpp file.
inline std::atomic<const OrderBook*> current{nullptr};

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
  boost::lockfree::queue<trading::OrderBookSnapshot>* l2_out = nullptr;
  boost::lockfree::queue<trading::Metric*>* metrics_out = nullptr;
  boost::lockfree::queue<trading::OrderRequest*>* order_out = nullptr;
  boost::lockfree::queue<trading::OrderFill*>* fill_in = nullptr;

  nlohmann::json params;

  std::shared_ptr<boost::asio::io_context> ioc;
  std::shared_ptr<boost::asio::ssl::context> ssl_ctx;
};

} // namespace trading