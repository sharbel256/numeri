#pragma once
#include <boost/lockfree/queue.hpp>
#include <dlfcn.h>
#include <model.hpp>
#include <nlohmann/json.hpp>
#include <plugin_interface.hpp>
#include <sys/event.h>
#include <sys/types.h>

#include <fstream>
#include <iostream>
#include <thread>

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

  std::unique_ptr<boost::lockfree::queue<trading::OrderBookReady*>> l2_broadcast_queue;
  std::unique_ptr<boost::lockfree::queue<trading::Metric*>> metrics_queue;
  std::unique_ptr<boost::lockfree::queue<trading::OrderRequest*>> order_queue;
  std::unique_ptr<boost::lockfree::queue<trading::OrderFill*>> fill_queue;

  std::vector<std::thread*> threads;
};