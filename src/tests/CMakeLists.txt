# File: my_project/src/tests/CMakeLists.txt

# Enable testing
enable_testing()

# Add the test executable
add_executable(OrderBookTest OrderBookTest.cpp)

# Include directories
target_include_directories(OrderBookTest PRIVATE
    ${CMAKE_SOURCE_DIR}/app
    ${CMAKE_SOURCE_DIR}/service
)

# Link libraries
target_link_libraries(OrderBookTest PRIVATE
    orderbook          # Your orderbook library
    service_lib        # If needed
    gtest              # Google Test library
    gtest_main         # Google Test main function
)


# Add second test executable
add_executable(OrderBookSnapshotTest OrderBookSnapshotTest.cpp)

target_include_directories(OrderBookSnapshotTest PRIVATE
    ${CMAKE_SOURCE_DIR}/app
    ${CMAKE_SOURCE_DIR}/service
)

# Link libraries
target_link_libraries(OrderBookSnapshotTest PRIVATE
    orderbook          # Your orderbook library
    service_lib        # If needed
    gtest              # Google Test library
    gtest_main         # Google Test main function
    pthread
)


# Add the test
add_test(NAME OrderBookTest COMMAND OrderBookTest)
add_test(NAME OrderBookSnapshotTest COMMAND OrderBookSnapshotTest)
