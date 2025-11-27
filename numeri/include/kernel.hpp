#pragma once
#include <boost/lockfree/queue.hpp>
#include <dlfcn.h>
#include <model.hpp>
#include <nlohmann/json.hpp>
#include <plugin_interface.hpp>
#include <sys/event.h>
#include <sys/types.h>
#include <websocket_client.hpp>

#include <fstream>
#include <iostream>
#include <thread>

namespace net = boost::asio;
namespace ssl = boost::asio::ssl;

class Kernel {
 public:
  Kernel();
  ~Kernel();
  void start();
  void stop();
  void watch_config();

private:
  void load_config();
  void init_queues();
  void start_ingestion();
  void start_execution();
  void start_algorithm();
  void check_for_config_updates();

  nlohmann::json config_;
  std::string config_path_;
  std::filesystem::file_time_type config_last_modified_;

  std::atomic<bool> running_{true};
  std::thread config_watcher_;
  std::unordered_map<std::string, IPlugin*> execution_engines;
  std::unordered_map<std::string, IPlugin*> algorithms;
  std::unordered_map<std::string, IPlugin*> ingestors;

  std::unique_ptr<boost::lockfree::queue<trading::OrderBookSnapshot>> l2_broadcast_queue;
  std::unique_ptr<boost::lockfree::queue<trading::Metric*>> metrics_queue;
  std::unique_ptr<boost::lockfree::queue<trading::OrderRequest*>> order_queue;
  std::unique_ptr<boost::lockfree::queue<trading::OrderFill*>> fill_queue;

  // plugin worker threads (value-type threads are safer than owning raw pointers)
  std::vector<std::thread> threads;

  // io_context and ssl::context will be owned by Kernel and shared with plugins
  std::shared_ptr<net::io_context> ioc;
  std::shared_ptr<ssl::context> ctx;

  using work_guard_t = boost::asio::executor_work_guard<net::io_context::executor_type>;
  std::unique_ptr<work_guard_t> ioc_work_guard;
  std::vector<std::thread> ioc_threads;
};