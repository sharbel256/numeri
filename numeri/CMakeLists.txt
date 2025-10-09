
add_library(kernel INTERFACE
    include/model.hpp
    include/ring_buffer.hpp
)

target_include_directories(kernel INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(numeri main.cpp)
target_link_libraries(numeri PRIVATE
    kernel
)