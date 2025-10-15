#include <kernel.hpp>

Kernel::Kernel() {
  load_config();

}

void Kernel::load_config() {
  try {
    std::string config_path =
      std::getenv("NUMERI_CONFIG_PATH") ? std::getenv("NUMERI_CONFIG_PATH") : "";

    std::ifstream file(config_path);
    if (!file.is_open()) {
      std::cerr << "Config file not found: " << config_path << std::endl;
      return;
    }

    config_ = nlohmann::json::parse(file);
    for (auto& [key, value] : config_.items()) {
      std::cout << key << " " << value << std::endl;
    }

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