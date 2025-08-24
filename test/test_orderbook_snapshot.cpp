// File: OrderBookSnapshotTest.cpp

#include <gtest/gtest.h>
#include <thread>
#include "orderbook.h"

using namespace trading;

OrderBookEntry createEntry(const std::string& side, double priceLevel, double quantity, int millisecondsAgo = 0) {
    OrderBookEntry entry;
    entry.side = side;
    entry.priceLevel = priceLevel;
    entry.quantity = quantity;
    entry.eventTime = std::chrono::system_clock::now() - std::chrono::milliseconds(millisecondsAgo);
    return entry;
}

TEST(OrderBookSnapshotTest, TestSnapshotConsistency) {
    OrderBook ob;
    ob.setWindowMillis(10000); // 10 seconds window

    // Add some entries
    ob.update(createEntry("bid", 100.0, 1.0));
    ob.update(createEntry("ask", 101.0, 1.5));

    // Get snapshot
    auto snapshot = ob.getSnapshot();

    // Verify snapshot content
    ASSERT_EQ(snapshot->bids.size(), 1);
    ASSERT_EQ(snapshot->asks.size(), 1);

    auto bestBid = snapshot->bids.rbegin();
    ASSERT_DOUBLE_EQ(bestBid->priceLevel, 100.0);
    ASSERT_DOUBLE_EQ(bestBid->quantity, 1.0);

    auto bestAsk = snapshot->asks.begin();
    ASSERT_DOUBLE_EQ(bestAsk->priceLevel, 101.0);
    ASSERT_DOUBLE_EQ(bestAsk->quantity, 1.5);
}

TEST(OrderBookSnapshotTest, TestLatencyMeasurement) {
    OrderBook ob;

    // Update order book and create snapshot
    ob.update(createEntry("bid", 100.0, 1.0));
    auto snapshot = ob.getSnapshot();

    // Wait for a while
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Measure latency
    auto now = std::chrono::system_clock::now();
    auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(now - snapshot->timestamp).count();

    // Since we waited for 100 ms, latency should be at least 100 ms
    ASSERT_GE(latency, 100);

    std::cout << "Snapshot latency: " << latency << " ms" << std::endl;
}

TEST(OrderBookSnapshotTest, TestConcurrentAccess) {
    OrderBook ob;
    ob.setWindowMillis(10000); // 10 seconds window

    // Writer thread function
    auto writerFunc = [&ob]() {
        for (int i = 0; i < 50; ++i) {
            ob.update(createEntry("bid", 100.0 + i * 0.1, 1.0));
            ob.update(createEntry("ask", 101.0 + i * 0.1, 1.5));
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };

    // Reader thread function
    auto readerFunc = [&ob](int threadId) {
        for (int i = 0; i < 20; ++i) {
            auto snapshot = ob.getSnapshot();
            // Perform some calculations
            if (!snapshot->bids.empty()) {
                auto bestBid = snapshot->bids.rbegin();
                // Simulate processing
            }
            if (!snapshot->asks.empty()) {
                auto bestAsk = snapshot->asks.begin();
                // Simulate processing
            }

            // Measure latency
            auto now = std::chrono::system_clock::now();
            auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(now - snapshot->timestamp).count();

            std::cout << "Reader " << threadId << " - Snapshot latency: " << latency << " ms" << std::endl;

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    };

    // Start writer thread
    std::thread writerThread(writerFunc);

    // Start reader threads
    std::thread readerThread1(readerFunc, 1);
    std::thread readerThread2(readerFunc, 2);

    // Join threads
    writerThread.join();
    readerThread1.join();
    readerThread2.join();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
