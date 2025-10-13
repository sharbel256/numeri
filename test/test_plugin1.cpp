#include <gtest/gtest.h>
#include <dlfcn.h>
#include <memory_resource>
#include <plugin_interface.hpp>


TEST(PluginTest, LoadPlugin1) {
    void* handle = dlopen("../numeri/plugins/libplugin1.dylib", RTLD_LAZY);
    ASSERT_NE(handle, nullptr) << "Failed to load plugin1: " << dlerror();
    
    auto createFunc = (PluginCreateFunc)dlsym(handle, "createPlugin");
    ASSERT_NE(createFunc, nullptr) << "Failed to find createPlugin symbol: " << dlerror();
    
    auto plugin = createFunc();
    ASSERT_NE(plugin, nullptr);
    EXPECT_EQ(plugin->getName(), "Plugin1");
    
    // Test execution
    EXPECT_NO_THROW(plugin->execute());
    
    // Cleanup
    auto destroyFunc = (PluginDestroyFunc)dlsym(handle, "destroyPlugin");
    ASSERT_NE(destroyFunc, nullptr) << "Failed to find destroyPlugin symbol: " << dlerror();
    destroyFunc(plugin);
    dlclose(handle);
}