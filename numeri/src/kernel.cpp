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
  // create shared io_context and ssl::context and run the io_context on a small thread pool
  ioc = std::make_shared<net::io_context>();
  ctx = std::make_shared<ssl::context>(ssl::context::sslv23_client);

  // keep io_context alive
  ioc_work_guard = std::make_unique<work_guard_t>(boost::asio::make_work_guard(*ioc));

  unsigned int n = std::max<unsigned int>(1, std::thread::hardware_concurrency());
  for (unsigned int i = 0; i < n; ++i) {
    ioc_threads.emplace_back([this] { ioc->run(); });
  }

  init_queues();
  start_ingestion();
  start_execution();
  start_algorithm();

  config_watcher_ = std::thread(&Kernel::watch_config, this);
  std::cout << "started watcher thread" << std::endl;
}

void Kernel::stop() {
  // signal stop
  running_ = false;
  // 1) signal plugins to stop so they can close sockets and exit their execute loops
  for (auto& [name, p] : ingestors) {
    if (p)
      p->stop();
  }
  for (auto& [name, p] : algorithms) {
    if (p)
      p->stop();
  }
  for (auto& [name, p] : execution_engines) {
    if (p)
      p->stop();
  }

  // 2) join plugin threads
  for (auto& t : threads) {
    if (t.joinable())
      t.join();
  }
  threads.clear();

  // 3) stop asio: reset guard then stop the context and join threads
  if (ioc_work_guard) {
    ioc_work_guard.reset();
  }

  if (ioc) {
    ioc->stop();
  }

  for (auto& t : ioc_threads) {
    if (t.joinable())
      t.join();
  }
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

void Kernel::init_queues() {
  auto make_queue = []<typename T>(size_t cap) {
    return std::make_unique<boost::lockfree::queue<T>>(cap);
  };

  auto metadata = config_["metadata"];

  l2_broadcast_queue =
      make_queue.operator()<trading::OrderBookSnapshot>(metadata["l2_broadcast_buffer"]);
  metrics_queue = make_queue.operator()<trading::Metric*>(metadata["metrics_buffer"]);
  order_queue = make_queue.operator()<trading::OrderRequest*>(metadata["order_buffer"]);
  fill_queue = make_queue.operator()<trading::OrderFill*>(metadata["fill_buffer"]);
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

void Kernel::start_ingestion() {
  for (auto& plugin : config_["data_sources"]) {
    std::string plugin_name = plugin["name"];
    std::string plugin_path = plugin["file"];
    std::cout << "starting " << plugin_name << std::endl;
    void* handle = dlopen(plugin_path.c_str(), RTLD_LAZY);
    if (handle == nullptr) {
      std::cerr << "Failed to load plugin: " << plugin["name"] << dlerror();
    }

    auto createFunc = (PluginCreateFunc)dlsym(handle, "createPlugin");
    if (createFunc == nullptr) {
      std::cerr << "Failed to find plugin symbol: " << plugin["name"] << dlerror();
    }

    auto loaded_plugin = createFunc();

    ingestors[plugin["name"]] = loaded_plugin;

    trading::PluginConfig cfg;
    cfg.l2_out = l2_broadcast_queue.get();
    cfg.metrics_out = metrics_queue.get();
    cfg.order_out = order_queue.get();
    cfg.fill_in = fill_queue.get();
    // pass shared io_context and ssl context to the plugin
    cfg.ioc = ioc;
    cfg.ssl_ctx = ctx;

    loaded_plugin->init(cfg, plugin["params"]);
  }

  for (auto& [name, p] : ingestors) {
    threads.emplace_back([p]() { p->execute(); });
  }
}

void Kernel::start_algorithm() {
  for (auto& plugin : config_["algorithms"]) {
    std::string plugin_name = plugin["name"];
    std::string plugin_path = plugin["file"];
    std::cout << "starting " << plugin_name << std::endl;
    void* handle = dlopen(plugin_path.c_str(), RTLD_LAZY);
    if (handle == nullptr) {
      std::cerr << "Failed to load plugin: " << plugin["name"] << dlerror();
    }

    auto createFunc = (PluginCreateFunc)dlsym(handle, "createPlugin");
    if (createFunc == nullptr) {
      std::cerr << "Failed to find plugin symbol: " << plugin["name"] << dlerror();
    }

    auto loaded_plugin = createFunc();

    algorithms[plugin["name"]] = loaded_plugin;
    trading::PluginConfig cfg;
    cfg.l2_out = l2_broadcast_queue.get();
    cfg.metrics_out = metrics_queue.get();
    // pass shared io_context and ssl context to the plugin
    cfg.ioc = ioc;
    cfg.ssl_ctx = ctx;

    loaded_plugin->init(cfg, plugin["params"]);
  }

  for (auto& [name, p] : algorithms) {
    threads.emplace_back([p]() { p->execute(); });
  }
}

void Kernel::start_execution() {
}