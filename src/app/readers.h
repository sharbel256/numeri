// File: ReaderThread.cpp

#include "orderbook.h"
#include <thread>
#include <chrono>
#include <iostream>

namespace readers {
    void readerThreadFunction(std::shared_ptr<trading::OrderBook> orderBook, std::atomic<bool>& shutdownFlag)
    {
        while (!shutdownFlag.load()) {
            // Get the latest snapshot
            auto snapshot = orderBook->getSnapshot();
            if (snapshot) {
                // latency 
                auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now() - snapshot->timestamp).count();

                // Access snapshot data
                std::cout << "Best Bid: " << snapshot->bestBid
                        << ", Best Ask: " << snapshot->bestAsk
                        << ", Spread: " << snapshot->spread 
                        << ", latency: " << latency << " ms" << std::endl;

            } else {
                std::cerr << "Warning: snapshot is null in readerThreadFunction" << std::endl;
            }

            // Sleep for a while before the next read
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
}
