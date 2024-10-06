// File: OrderBook.cpp

#include "orderbook.h"
#include <algorithm>
#include <iostream>

namespace trading {

void OrderBook::update(const OrderBookEntry& entry) {
    if (entry.quantity < 0) {
        return;
    }
    // std::cout << "Updating order book with entry: " << entry.priceLevel << std::endl;
    std::lock_guard<std::mutex> lock(bookMutex);

    auto& book = (entry.side == "bid") ? bids : asks;

    // Remove existing entry at this price level
    auto range = book.equal_range(entry);
    book.erase(range.first, range.second);

    // Add new entry if quantity is not zero
    if (!fuzzyIsNull(entry.quantity)) {
        book.insert(entry);
    }

    removeOldEntries();
}

void OrderBook::removeOldEntries() {
    auto currentTime = std::chrono::system_clock::now();
    oldestAllowedTime = currentTime - std::chrono::milliseconds(windowMillis);

    auto removeOld = [this](const OrderBookEntry& e) {
        return e.eventTime < oldestAllowedTime;
    };

    // For bids
    for (auto it = bids.begin(); it != bids.end(); ) {
        if (removeOld(*it)) {
            it = bids.erase(it);
        } else {
            ++it;
        }
    }

    // For asks
    for (auto it = asks.begin(); it != asks.end(); ) {
        if (removeOld(*it)) {
            it = asks.erase(it);
        } else {
            ++it;
        }
    }
}

std::vector<OrderBookEntry> OrderBook::getTopLevels(int n, bool isBid) const {
    const auto& book = isBid ? bids : asks;
    std::vector<OrderBookEntry> result;
    result.reserve(std::min<int>(n, static_cast<int>(book.size())));

    if (isBid) {
        // For bids, we want the highest prices first (reverse order)
        auto it = book.rbegin();
        auto end = book.rend();
        for (int i = 0; i < n && it != end; ++i, ++it) {
            result.push_back(*it);
        }
    } else {
        // For asks, we want the lowest prices first (normal order)
        auto it = book.begin();
        auto end = book.end();
        for (int i = 0; i < n && it != end; ++i, ++it) {
            result.push_back(*it);
        }
    }

    return result;
}

void OrderBook::createSnapshot() {
    std::lock_guard<std::mutex> lock(bookMutex);

    // Create a deep copy of the current bids and asks
    auto snapshot = std::make_shared<OrderBookSnapshot>(bids, asks);

    // Atomically store the new snapshot
    std::atomic_store(&snapshotPtr, snapshot);
}

std::shared_ptr<OrderBookSnapshot> OrderBook::getSnapshot() {
    auto snapshot = std::atomic_load(&snapshotPtr);
    if (snapshot) {
        snapshot->updateMetrics();
        return snapshot;
    } else {
        // Handle the case when snapshotPtr is null
        std::cerr << "Error: snapshotPtr is null in getSnapshot()" << std::endl;
        // You can choose to return a default snapshot or handle it appropriately
        return nullptr;
    }
}

void OrderBook::stopSnapshotThread()
{
    snapshotThreadRunning = false;
    if (snapshotThread.joinable()) {
        snapshotThread.join();
    }
}
} // namespace trading