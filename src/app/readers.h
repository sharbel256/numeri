// File: ReaderThread.cpp

#include "orderbook.h"
#include <thread>
#include <chrono>
#include <iostream>

namespace readers {
    void readerThreadFunction(trading::OrderBook& orderBook) {
        while (true) {
            // Get the snapshot
            auto snapshot = orderBook.getSnapshot();

            // Measure latency
            auto now = std::chrono::system_clock::now();
            auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(now - snapshot->timestamp).count();

            // Optionally, print the metrics and the time difference
            std::cout << "Best Bid: " << snapshot->bestBid
                    << ", Best Ask: " << snapshot->bestAsk
                    << ", Spread: " << snapshot->spread
                    << ", latency: " << latency << " ms" << std::endl;

            // Sleep for a specified interval
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
}
