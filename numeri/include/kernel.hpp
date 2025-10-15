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


 private:
  void load_config();

  // std::jthread config_watcher_thread_; not using yet
  nlohmann::json config_;
};