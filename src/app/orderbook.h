// File: OrderBook.h

#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <map>
#include <set>
#include <vector>
#include <string>
#include <chrono>
#include <cmath>
#include <atomic>
#include <iostream>
#include <thread>

namespace trading {

inline bool fuzzyCompare(double a, double b, double epsilon = 0.000001) {
    return std::abs(a - b) < epsilon;
}

inline bool fuzzyIsNull(double a, double epsilon = 0.000001) {
    return std::abs(a) < epsilon;
}

class OrderBookEntry {
public:
    std::string side;
    std::chrono::system_clock::time_point eventTime;
    double priceLevel;
    double quantity;

    bool operator==(const OrderBookEntry& other) const {
        return side == other.side && 
               fuzzyCompare(priceLevel, other.priceLevel) && 
               fuzzyCompare(quantity, other.quantity);
    }

    bool operator<(const OrderBookEntry& other) const {
        return priceLevel < other.priceLevel;
    }
};

class OrderBookSnapshot {
public:
    std::multiset<OrderBookEntry> bids;
    std::multiset<OrderBookEntry> asks;
    std::chrono::system_clock::time_point timestamp;

    double bestBid;
    double bestAsk;
    double spread;

    OrderBookSnapshot():bestBid(0), 
                        bestAsk(std::numeric_limits<double>::max()), 
                        spread(std::numeric_limits<double>::max()){};

    // Copy constructor for deep copy
    OrderBookSnapshot(const std::multiset<OrderBookEntry>& bids,
                      const std::multiset<OrderBookEntry>& asks)
        : bids(bids), asks(asks), timestamp(std::chrono::system_clock::now()) {}

    void updateMetrics() {
        // std::cout << "updating metrics ";
        bestBid = bids.empty() ? 0 : bids.rbegin()->priceLevel;
        // std::cout << "bestBid: " << bestBid;
        bestAsk = asks.empty() ? std::numeric_limits<double>::max() : asks.begin()->priceLevel;
        // std::cout << "bestAsk: " << bestAsk;
        spread = bestAsk - bestBid;
        // std::cout <<"spread: " << spread << std::endl;
    }

    void printBids() {
        for (auto bid : bids) {
            std::cout << bid.priceLevel << std::endl;
        }
    }
};

class OrderBook {
private:
    std::multiset<OrderBookEntry> bids;
    std::multiset<OrderBookEntry> asks;
    std::chrono::system_clock::time_point oldestAllowedTime;
    int windowMillis;

    std::shared_ptr<OrderBookSnapshot> snapshotPtr;

    // New members for snapshot thread
    std::thread snapshotThread;
    std::atomic<bool> snapshotThreadRunning;
    int snapshotIntervalMillis; // Snapshot interval in milliseconds
    std::mutex bookMutex; // Mutex for thread safety

public:
    OrderBook() : windowMillis(5000), snapshotIntervalMillis(100), snapshotThreadRunning(true)
    {
        // Initialize snapshotPtr
        std::atomic_store(&snapshotPtr, std::make_shared<OrderBookSnapshot>());

        // Start the snapshot thread
        snapshotThread = std::thread([this]() {
            while (snapshotThreadRunning.load()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(snapshotIntervalMillis));
                createSnapshot();
            }
        });
    }
    ~OrderBook() {
        stopSnapshotThread();
    }

    // Delete copy constructor and copy assignment operator
    OrderBook(const OrderBook&) = delete;
    OrderBook& operator=(const OrderBook&) = delete;

    // // If necessary, define move constructor and move assignment operator
    // OrderBook(OrderBook&&) = default;
    // OrderBook& operator=(OrderBook&&) = default;


    void update(const OrderBookEntry& entry);
    void removeOldEntries();
    std::vector<OrderBookEntry> getTopLevels(int n, bool isBid) const;
    void setWindowMillis(int millis) { windowMillis = millis; }

    // Snapshot mechanism
    std::shared_ptr<OrderBookSnapshot> getSnapshot();
    void createSnapshot();
    void stopSnapshotThread();

    // getter

};

} // namespace trading

#endif // ORDERBOOK_H