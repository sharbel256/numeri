// File: ReaderThread.cpp

#include "orderbook.h"
#include <thread>
#include <chrono>
#include <iostream>

namespace readers {
    void readerThreadFunction(std::shared_ptr<trading::OrderBook> orderBook, std::atomic<bool>& shutdownFlag)
    {
        double worstBid = 0;
        double bestBid = 0;
        double bestAsk = std::numeric_limits<double>::max();
        double worstAsk = std::numeric_limits<double>::max();
        double spread = bestAsk - bestBid;
        while (!shutdownFlag.load()) {
            // Get the latest snapshot
            auto snapshot = orderBook->getSnapshot();
            if (snapshot) {
                // latency 
                auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now() - snapshot->timestamp).count();

                worstBid = snapshot->bids.empty() ? 0 : snapshot->bids.begin()->priceLevel;
                bestBid = snapshot->bids.empty() ? 0 : snapshot->bids.rbegin()->priceLevel;
                bestAsk = snapshot->asks.empty() ? std::numeric_limits<double>::max() : snapshot->asks.begin()->priceLevel;
                worstAsk = snapshot->asks.empty() ? std::numeric_limits<double>::max() : snapshot->asks.rbegin()->priceLevel;
                spread = bestAsk - bestBid;

                // Access snapshot data
                std::cout << "worst bid: " << worstBid
                        << ", Best Bid: " << bestBid
                        << ", Best Ask: " << bestAsk
                        << ", worst ask: " << worstAsk
                        << ", Spread: " << spread 
                        << ", latency: " << latency << " ms"
                        << ", book size: " << snapshot->bids.size()+ snapshot->asks.size() << std::endl;

            } else {
                std::cerr << "Warning: snapshot is null in readerThreadFunction" << std::endl;
            }
            // Sleep for a while before the next read
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}
