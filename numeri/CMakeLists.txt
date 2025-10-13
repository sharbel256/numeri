add_library(kernel INTERFACE
    include/model.hpp
    include/ring_buffer.hpp
    include/plugin_interface.hpp
)

target_include_directories(kernel INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(numeri main.cpp)
target_link_libraries(numeri PRIVATE
    kernel
)

add_subdirectory(plugins)