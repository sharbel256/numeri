#include <kernel.hpp>

Kernel::Kernel() {
  load_config();
}

Kernel::~Kernel() {
  if (config_watcher_.joinable()) {
    config_watcher_.join();
  }
}

void Kernel::start() {
  config_watcher_ = std::thread(&Kernel::watch_config, this);
  std::cout << "started watcher thread" << std::endl;
}

void Kernel::stop() {
  running_ = false;
}

void Kernel::load_config() {
  try {
    config_path_ = std::getenv("NUMERI_CONFIG_PATH") ? std::getenv("NUMERI_CONFIG_PATH") : "";

    std::ifstream file(config_path_);
    if (!file.is_open()) {
      std::cerr << "Config file not found: " << config_path_ << std::endl;
      return;
    }

    config_ = nlohmann::json::parse(file);
    for (auto& [key, value] : config_.items()) {
      std::cout << key << " " << value << std::endl;
    }

    config_last_modified_ = std::filesystem::last_write_time(config_path_);

    bool autoload = config_["plugins"]["autoload"];
    std::vector<std::filesystem::path> plugin_files;

    for (auto path : plugin_files) {
      std::cout << "path: " << path << std::endl;
    }
    
  } catch (const std::exception& e) {
    std::cerr << "Error loading config: " << e.what() << std::endl;
    throw e;
  }
}

void Kernel::watch_config() {
  while (running_) {
    check_for_config_updates();
    std::this_thread::sleep_for(std::chrono::seconds(3));
  }
}

void Kernel::check_for_config_updates() {
  auto current_modified = std::filesystem::last_write_time(config_path_);

  if (current_modified > config_last_modified_) {
    try {
      std::fstream file(config_path_);
      nlohmann::json new_config = nlohmann::json::parse(file);

      config_ = std::move(new_config);
      std::cout << "config was updated" << std::endl;
      config_last_modified_ = current_modified;
    } catch (const std::exception& e) {
      std::cerr << "error updating config: " << e.what() << std::endl;
    }
  }
}