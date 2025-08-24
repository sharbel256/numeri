cmake_minimum_required(VERSION 3.14)

enable_testing()

find_package(GTest REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/src/app)

add_executable(snapshot_test test_orderbook_snapshot.cpp)
add_executable(orderbook_test test_orderbook.cpp)

target_link_libraries(snapshot_test PRIVATE 
    GTest::gtest 
    GTest::gtest_main 
    orderbook
)


target_link_libraries(orderbook_test PRIVATE 
    GTest::gtest 
    GTest::gtest_main 
    orderbook
)

add_test(NAME SnapshotTest COMMAND snapshot_test)
add_test(NAME OrderbookTest COMMAND orderbook_test)