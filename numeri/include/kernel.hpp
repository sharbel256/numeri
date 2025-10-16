#pragma once
#include <dlfcn.h>
#include <sys/event.h>
#include <sys/types.h>

#include <iostream>
#include <fstream>

#include <thread>
#include <nlohmann/json.hpp>

class Kernel {
 public:
  Kernel();
  void start();
  void stop();
  void watch_config();

private:
  void load_config();
  void check_for_config_updates();

  nlohmann::json config_;
  std::string config_path_;
  std::filesystem::file_time_type config_last_modified_;

  std::atomic<bool> running_{true};
  std::jthread config_watcher_;
};