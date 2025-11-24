cmake_minimum_required(VERSION 3.14)

enable_testing()

find_package(GTest REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/numeri/include)

add_executable(ringbuffer_test test_ringbuffer.cpp)
target_link_libraries(ringbuffer_test PRIVATE GTest::gtest_main)
target_link_libraries(ringbuffer_test PRIVATE OpenSSL::SSL OpenSSL::Crypto)
add_test(NAME RingBufferTest COMMAND ringbuffer_test)

add_executable(test_plugin1 test_plugin1.cpp)
target_link_libraries(test_plugin1 PRIVATE GTest::gtest_main)
target_link_libraries(test_plugin1 PRIVATE OpenSSL::SSL OpenSSL::Crypto)
add_test(NAME TestPlugin1 COMMAND test_plugin1)