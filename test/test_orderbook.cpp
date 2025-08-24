// File: OrderBookTest.cpp

#include <gtest/gtest.h>
#include "orderbook.h"
#include <chrono>
#include <thread>


using namespace trading;
using namespace std;

// Updated createEntry function
OrderBookEntry createEntry(const std::string& side, double priceLevel, double quantity, int millisAgo = 0) {
    OrderBookEntry entry;
    entry.side = side;
    entry.priceLevel = priceLevel;
    entry.quantity = quantity;
    entry.eventTime = std::chrono::system_clock::now() - std::chrono::milliseconds(millisAgo);
    return entry;
}

// Test suite for OrderBook
TEST(OrderBookTest, EmptyOrderBook) {
    OrderBook ob;
    auto snapshot = ob.getSnapshot();
    
    EXPECT_EQ(snapshot->bestBid, 0);
    EXPECT_EQ(snapshot->bestAsk, std::numeric_limits<double>::max());
    EXPECT_EQ(snapshot->spread, std::numeric_limits<double>::max());
}

TEST(OrderBookTest, SingleBid) {
    OrderBook ob;
    auto entry = createEntry("bid", 100.0, 1.0);
    ob.update(entry);

    auto snapshot = ob.getSnapshot();
    EXPECT_EQ(snapshot->bestBid, 100.0);
    EXPECT_EQ(snapshot->bestAsk, std::numeric_limits<double>::max());
    EXPECT_EQ(snapshot->spread, std::numeric_limits<double>::max() - 100.0);
}

TEST(OrderBookTest, SingleAsk) {
    OrderBook ob;
    auto entry = createEntry("ask", 105.0, 1.0);
    ob.update(entry);

    auto snapshot = ob.getSnapshot();

    EXPECT_EQ(snapshot->bestBid, 0);
    EXPECT_EQ(snapshot->bestAsk, 105.0);
    EXPECT_EQ(snapshot->spread, 105.0);
}

TEST(OrderBookTest, BidAndAsk) {
    OrderBook ob;
    auto bidEntry = createEntry("bid", 100.0, 1.0);
    auto askEntry = createEntry("ask", 105.0, 1.0);
    ob.update(bidEntry);
    ob.update(askEntry);

    auto snapshot = ob.getSnapshot();
    EXPECT_EQ(snapshot->bestBid, 100.0);
    EXPECT_EQ(snapshot->bestAsk, 105.0);
    EXPECT_EQ(snapshot->spread, 5.0);
}

TEST(OrderBookTest, MultipleBidsAndAsks) {
    OrderBook ob;
    ob.update(createEntry("bid", 100.0, 1.0));
    ob.update(createEntry("bid", 101.0, 2.0));
    ob.update(createEntry("bid", 99.0, 1.5));
    ob.update(createEntry("ask", 105.0, 1.0));
    ob.update(createEntry("ask", 104.0, 2.0));
    ob.update(createEntry("ask", 106.0, 1.5));

    auto snapshot = ob.getSnapshot();

    EXPECT_EQ(snapshot->bestBid, 101.0);
    EXPECT_EQ(snapshot->bestAsk, 104.0);
    EXPECT_EQ(snapshot->spread, 3.0);

    // Get top 2 bids
    auto topBids = ob.getTopLevels(2, true);
    ASSERT_EQ(topBids.size(), 2);
    EXPECT_EQ(topBids[0].priceLevel, 101.0);
    EXPECT_EQ(topBids[1].priceLevel, 100.0);

    // Get top 2 asks
    auto topAsks = ob.getTopLevels(2, false);
    ASSERT_EQ(topAsks.size(), 2);
    EXPECT_EQ(topAsks[0].priceLevel, 104.0);
    EXPECT_EQ(topAsks[1].priceLevel, 105.0);
}

TEST(OrderBookTest, UpdateQuantity) {
    OrderBook ob;
    auto entry = createEntry("bid", 100.0, 1.0);
    ob.update(entry);
    auto snapshot = ob.getSnapshot();
    EXPECT_EQ(snapshot->bestBid, 100.0);


    // Update the same price level with new quantity
    entry.quantity = 2.0;
    ob.update(entry);
    EXPECT_EQ(snapshot->bestBid, 100.0);

    // Check that the quantity has been updated
    auto topBids = ob.getTopLevels(1, true);
    ASSERT_EQ(topBids.size(), 1);
    EXPECT_EQ(topBids[0].quantity, 2.0);
}

TEST(OrderBookTest, RemoveEntry) {
    OrderBook ob;
    auto entry = createEntry("ask", 105.0, 1.0);
    ob.update(entry);
    auto snapshot = ob.getSnapshot();
    EXPECT_EQ(snapshot->bestAsk, 105.0);

    // Remove the entry by setting quantity to zero
    entry.quantity = 0.0;
    ob.update(entry);
    snapshot = ob.getSnapshot();
    EXPECT_EQ(snapshot->bestAsk, std::numeric_limits<double>::max());
}

TEST(OrderBookTest, RemoveOldEntries) {
    OrderBook ob;
    ob.setWindowMillis(1000); // Set window to 1 second for testing

    auto recentEntry = createEntry("bid", 100.0, 1.0);
    auto oldEntry = createEntry("bid", 101.0, 1.0, 500); // 500 ms ago

    ob.update(recentEntry);
    ob.update(oldEntry);

    auto snapshot = ob.getSnapshot();
    // Initially, both entries are present
    EXPECT_EQ(snapshot->bestBid, 101.0);

    // Wait for entries to expire
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // will expires the oldEntry

    auto newEntry = createEntry("bid", 99.0, 1.0); 
    ob.update(newEntry); // will force remove the oldEntry
    snapshot = ob.getSnapshot();
    EXPECT_EQ(snapshot->bestBid, 100.0); // remaining entry is the recentEntry
}

TEST(OrderBookTest, EdgeCaseZeroQuantity) {
    OrderBook ob;
    auto entry = createEntry("bid", 100.0, 1.0);
    ob.update(entry);
    auto snapshot = ob.getSnapshot();
    EXPECT_EQ(snapshot->bestBid, 100.0);

    // Attempt to add an entry with zero quantity
    auto zeroQuantityEntry = createEntry("bid", 99.0, 0.0);
    ob.update(zeroQuantityEntry);

    // Ensure that zero quantity entries are not added
    auto topBids = ob.getTopLevels(2, true);
    ASSERT_EQ(topBids.size(), 1);
    EXPECT_EQ(topBids[0].priceLevel, 100.0);
}

// Updated EdgeCaseNegativeQuantity test
TEST(OrderBookTest, EdgeCaseNegativeQuantity) {
    OrderBook ob;
    // Attempt to add an entry with negative quantity
    auto negativeQuantityEntry = createEntry("ask", 105.0, -1.0);
    ob.update(negativeQuantityEntry);

    auto snapshot = ob.getSnapshot();

    // Ensure that negative quantity entries are rejected
    EXPECT_EQ(snapshot->bestAsk, std::numeric_limits<double>::max());
}

TEST(OrderBookTest, EdgeCaseSamePriceLevelDifferentTimes) {
    OrderBook ob;
    auto entry1 = createEntry("bid", 100.0, 1.0, 1); // 1 second ago
    auto entry2 = createEntry("bid", 100.0, 2.0, 0); // Now

    ob.update(entry1);
    ob.update(entry2);

    // Ensure the latest entry replaces the old one
    auto topBids = ob.getTopLevels(1, true);
    ASSERT_EQ(topBids.size(), 1);
    EXPECT_EQ(topBids[0].quantity, 2.0);
}

TEST(OrderBookTest, EdgeCaseHighPrecisionPrices) {
    OrderBook ob;
    auto entry = createEntry("bid", 100.123456789, 1.0);
    ob.update(entry);

    auto snapshot = ob.getSnapshot();

    EXPECT_NEAR(snapshot->bestBid, 100.123456789, 1e-9);
}

TEST(OrderBookTest, EdgeCaseLargeNumbers) {
    OrderBook ob;
    auto entry = createEntry("ask", 1e9, 1e6);
    ob.update(entry);
    auto snapshot = ob.getSnapshot();
    EXPECT_EQ(snapshot->bestAsk, 1e9);
}


// File: OrderBookTest.cpp