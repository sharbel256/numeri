#pragma once

#include <model.hpp>
#include <nlohmann/json.hpp>

#include <functional>
#include <memory>
#include <string>

class IPlugin {
public:
  virtual ~IPlugin() = default;
  virtual std::string getName() const = 0;
  virtual void execute() = 0;
  virtual void init(const trading::PluginConfig& config, const nlohmann::json& params) = 0;
  // Request the plugin to stop and clean up async resources. Plugins should return from execute()
  // shortly after stop() is called.
  virtual void stop() = 0;
};

using PluginCreateFunc = IPlugin* (*)();
using PluginDestroyFunc = void (*)(IPlugin*);

#define EXPORT_PLUGIN(PluginClass)                                                                   \
  extern "C" {                                                                                       \
  IPlugin* createPlugin() {                                                                          \
    return new PluginClass();                                                                        \
  }                                                                                                  \
  void destroyPlugin(IPlugin* plugin) {                                                              \
    delete plugin;                                                                                   \
  }                                                                                                  \
  }