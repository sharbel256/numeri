// File: readers.h

#include "orderbook.h"
#include "orderbuilder.cpp"
#include <thread>
#include <chrono>
#include <cmath>
#include <nlohmann/json.hpp>
#include <iomanip>
#include <sstream>

namespace readers {
    class MarketMaker {
    private:
        std::shared_ptr<trading::OrderBook> orderBook;
        std::shared_ptr<Trading> trading;
        std::atomic<bool>& shutdownFlag;
        double minSpread;  // in dollars
        double minProfit;  // in dollars
        double orderSize;
        std::string productId;
        std::string currentBidOrderId;
        std::string currentAskOrderId;
        double currentBidPrice;
        double currentAskPrice;

        // Depth analysis parameters
        const int depthLevels = 5;  // Number of price levels to analyze
        const double pressureThreshold = 2.0;  // Ratio threshold for significant pressure

        struct DepthAnalysis {
            double bidVolume;
            double askVolume;
            double pressureRatio;  // bidVolume / askVolume
            double weightedMidPrice;  // Volume-weighted mid price
        };

        DepthAnalysis analyzeDepth(const std::shared_ptr<trading::OrderBookSnapshot>& snapshot) {
            DepthAnalysis analysis{0.0, 0.0, 1.0, 0.0};
            double totalVolume = 0.0;
            double weightedPriceSum = 0.0;

            // Analyze bid side
            int bidLevels = 0;
            for (auto it = snapshot->bids.rbegin(); it != snapshot->bids.rend() && bidLevels < depthLevels; ++it) {
                analysis.bidVolume += it->quantity;
                weightedPriceSum += it->priceLevel * it->quantity;
                totalVolume += it->quantity;
                bidLevels++;
            }

            // Analyze ask side
            int askLevels = 0;
            for (auto it = snapshot->asks.begin(); it != snapshot->asks.end() && askLevels < depthLevels; ++it) {
                analysis.askVolume += it->quantity;
                weightedPriceSum += it->priceLevel * it->quantity;
                totalVolume += it->quantity;
                askLevels++;
            }

            // Calculate pressure ratio and weighted mid price
            if (analysis.askVolume > 0) {
                analysis.pressureRatio = analysis.bidVolume / analysis.askVolume;
            }
            if (totalVolume > 0) {
                analysis.weightedMidPrice = weightedPriceSum / totalVolume;
            }

            return analysis;
        }

        double calculateAdjustedPrice(double basePrice, double pressureRatio, bool isBid) {
            // Adjust price based on pressure
            // If pressureRatio > 1: more buy pressure
            // If pressureRatio < 1: more sell pressure
            double adjustment = 0.0;

            if (pressureRatio > pressureThreshold) {
                // Strong buy pressure
                adjustment = isBid ? minProfit * 0.5 : minProfit * 1.5;
            } else if (pressureRatio < 1.0 / pressureThreshold) {
                // Strong sell pressure
                adjustment = isBid ? minProfit * 1.5 : minProfit * 0.5;
            } else {
                // Balanced pressure
                adjustment = minProfit;
            }

            return isBid ? basePrice - adjustment: (basePrice + adjustment);
        }

        // Helper function to format price with exactly 2 decimal places
        std::string formatPrice(double price) {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << price;
            return ss.str();
        }

        // Helper function to format size with exactly 6 decimal places
        std::string formatSize(double size) {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(6) << size;
            return ss.str();
        }

    public:
        MarketMaker(std::shared_ptr<trading::OrderBook> ob, 
                   std::shared_ptr<Trading> t, 
                   std::atomic<bool>& sf,
                   const std::string& pid = "BTC-USD",
                   double ms = 30.0,  // minimum spread in dollars
                   double mp = 20.0,  // minimum profit in dollars
                   double os = 0.001) // order size
            : orderBook(ob), trading(t), shutdownFlag(sf), 
              minSpread(ms), minProfit(mp), orderSize(os), productId(pid),
              currentBidOrderId(""), currentAskOrderId(""),
              currentBidPrice(0), currentAskPrice(0) {}

        void checkOrderStatus() {
            // Check bid order status
            if (!currentBidOrderId.empty()) {
                try {
                    std::string response = trading->getOrder(currentBidOrderId);
                    auto jsonResponse = nlohmann::json::parse(response);
                    if (jsonResponse["status"] == "FILLED" || jsonResponse["status"] == "CANCELED") {
                        std::cout << "Bid order " << currentBidOrderId << " was " << jsonResponse["status"].get<std::string>() << std::endl;
                        currentBidOrderId = "";
                        currentBidPrice = 0;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error checking bid order status: " << e.what() << std::endl;
                }
            }

            // Check ask order status
            if (!currentAskOrderId.empty()) {
                try {
                    std::string response = trading->getOrder(currentAskOrderId);
                    auto jsonResponse = nlohmann::json::parse(response);
                    if (jsonResponse["status"] == "FILLED" || jsonResponse["status"] == "CANCELED") {
                        std::cout << "Ask order " << currentAskOrderId << " was " << jsonResponse["status"].get<std::string>() << std::endl;
                        currentAskOrderId = "";
                        currentAskPrice = 0;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error checking ask order status: " << e.what() << std::endl;
                }
            }
        }

        void run() {
            while (!shutdownFlag.load()) {
                // Check status of existing orders
                checkOrderStatus();

                auto snapshot = orderBook->getSnapshot();
                if (snapshot) {
                    // Get best bid and ask, handling empty order book case
                    double bestBid = 0;
                    double bestAsk = 0;
                    bool hasValidPrices = false;

                    if (!snapshot->bids.empty()) {
                        bestBid = snapshot->bids.rbegin()->priceLevel;
                        hasValidPrices = true;
                    }
                    if (!snapshot->asks.empty()) {
                        bestAsk = snapshot->asks.begin()->priceLevel;
                        hasValidPrices = true;
                    }

                    // Only proceed if we have valid prices
                    if (hasValidPrices && bestBid > 0 && bestAsk > 0) {
                        double spread = bestAsk - bestBid;

                        // Analyze order book depth
                        DepthAnalysis depth = analyzeDepth(snapshot);
                        double midPrice = depth.weightedMidPrice > 0 ? depth.weightedMidPrice : (bestBid + bestAsk) / 2.0;

                        // // Print depth analysis
                        // std::cout << "Depth Analysis - "
                        //          << "Bid Volume: " << depth.bidVolume
                        //          << ", Ask Volume: " << depth.askVolume
                        //          << ", Pressure Ratio: " << depth.pressureRatio
                        //          << ", Weighted Mid: " << midPrice << std::endl;

                        // Check if we should place new orders
                        if (spread >= minSpread) {
                            // Calculate target prices based on pressure
                            double targetBidPrice = calculateAdjustedPrice(midPrice, depth.pressureRatio, true);
                            double targetAskPrice = calculateAdjustedPrice(midPrice, depth.pressureRatio, false);

                            // Ensure prices are within reasonable bounds
                            if (targetBidPrice > 0 && targetAskPrice > 0) {
                                // Place or update orders if needed
                                if (currentBidOrderId.empty() || targetBidPrice != currentBidPrice) {
                                    placeBidOrder(targetBidPrice);
                                }
                                if (currentAskOrderId.empty() || targetAskPrice != currentAskPrice) {
                                    placeAskOrder(targetAskPrice);
                                }
                            }
                        }

                        // // Print market data
                        // std::cout << "Market Data - "
                        //          << "Best Bid: " << bestBid
                        //          << ", Best Ask: " << bestAsk
                        //          << ", Spread: " << spread
                        //          << ", Mid Price: " << midPrice
                        //          << ", Our Bid: " << currentBidPrice
                        //          << ", Our Ask: " << currentAskPrice << std::endl;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            // Clean up any remaining orders on shutdown
            if (!currentBidOrderId.empty()) {
                cancelOrder(currentBidOrderId);
            }
            if (!currentAskOrderId.empty()) {
                cancelOrder(currentAskOrderId);
            }
        }

    private:
        void placeBidOrder(double price) {
            if (!currentBidOrderId.empty()) {
                // Cancel existing bid order
                cancelOrder(currentBidOrderId);
            }

            // Create new bid order with formatted price and size
            std::string order = OrderBuilder()
                .setProductId(productId)
                .setSide("BUY")
                .limitLimitGtc(formatSize(orderSize), formatPrice(price), false)
                .build();

            try {
                std::string response = trading->createOrder(order);
                auto jsonResponse = nlohmann::json::parse(response);
                
                if (jsonResponse["success"] == true && jsonResponse.contains("order_id") && !jsonResponse["order_id"].is_null()) {
                    currentBidOrderId = jsonResponse["order_id"].get<std::string>();
                    currentBidPrice = price;
                    std::cout << "Successfully placed bid order at " << price << " with ID: " << currentBidOrderId << std::endl;
                } else {
                    std::cerr << "Error: Failed to place bid order. Full response: " << response << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error placing bid order: " << e.what() << std::endl;
            }
        }

        void placeAskOrder(double price) {
            if (!currentAskOrderId.empty()) {
                // Cancel existing ask order
                cancelOrder(currentAskOrderId);
            }

            // Create new ask order with formatted price and size
            std::string order = OrderBuilder()
                .setProductId(productId)
                .setSide("SELL")
                .limitLimitGtc(formatSize(orderSize), formatPrice(price), false)
                .build();

            try {
                std::string response = trading->createOrder(order);
                auto jsonResponse = nlohmann::json::parse(response);
                
                if (jsonResponse["success"] == true && jsonResponse.contains("order_id") && !jsonResponse["order_id"].is_null()) {
                    currentAskOrderId = jsonResponse["order_id"].get<std::string>();
                    currentAskPrice = price;
                    std::cout << "Successfully placed ask order at " << price << " with ID: " << currentAskOrderId << std::endl;
                } else {
                    std::cerr << "Error: Failed to place ask order. Full response: " << response << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error placing ask order: " << e.what() << std::endl;
            }
        }

        void cancelOrder(const std::string& orderId) {
            try {
                std::string requestBody = R"({"order_ids": [")" + orderId + R"("]})";
                trading->batchCancelOrder(requestBody);
                std::cout << "Successfully canceled order: " << orderId << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error canceling order: " << e.what() << std::endl;
            }
        }
    };

    void readerThreadFunction(std::shared_ptr<trading::OrderBook> orderBook, std::atomic<bool>& shutdownFlag)
    {
        // Create a Trading instance
        net::io_context ioc;
        std::shared_ptr<Trading> trading = std::make_shared<Trading>(ioc, shutdownFlag);

        // Create and run the market maker
        MarketMaker marketMaker(orderBook, trading, shutdownFlag);
        marketMaker.run();
    }
}
