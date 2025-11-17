find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(Boost 1.53 REQUIRED COMPONENTS atomic)

add_library(kernel INTERFACE
    include/model.hpp
    include/ring_buffer.hpp
    include/plugin_interface.hpp
    include/kernel.hpp
)

target_include_directories(kernel INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(numeri main.cpp src/kernel.cpp)
target_link_libraries(numeri PRIVATE
    kernel
    nlohmann_json::nlohmann_json
    Boost::atomic
)

add_subdirectory(plugins)