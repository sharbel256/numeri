#include "trading.h"
#include "readers.h"
// #include "orderbuilder.cpp"
#include <thread>
#include <atomic>
#include <boost/asio/signal_set.hpp>

int main(int argc, char *argv[])
{
    net::io_context ioc;

    // Create a signal set registered for SIGINT and SIGTERM
    boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);

    // Atomic flag to signal shutdown
    std::atomic<bool> shutdownFlag(false);

    // Instantiate Trading with shared io_context and shutdown flag
    Trading trading(ioc, shutdownFlag);

    // Ensure the OrderBook for "BTC-USD" is created
    {
        auto& orderBooks = trading.getOrderbooks();
        std::lock_guard<std::mutex> lock(trading.orderBooksMutex);
        if (orderBooks.find("BTC-USD") == orderBooks.end()) {
            // Create a new OrderBook with desired snapshot interval
            orderBooks["BTC-USD"] = std::make_shared<trading::OrderBook>();
        }
    }

    signals.async_wait([&](const boost::system::error_code&, int)
    {
        std::cout << "Signal received, shutting down..." << std::endl;
        shutdownFlag = true;
        trading.shutdown();
        ioc.stop();
    });

    // Start trading operations
    trading.getAccounts();
    trading.startWebsocket();

    // Create and run the market maker
    auto& orderBooks = trading.getOrderbooks();
    if (orderBooks.find("BTC-USD") != orderBooks.end()) {
        // Create a shared pointer to Trading for the market maker
        std::shared_ptr<Trading> tradingPtr = std::make_shared<Trading>(ioc, shutdownFlag);
        
        // Create and run the market maker
        readers::MarketMaker marketMaker(
            orderBooks["BTC-USD"],
            tradingPtr,
            shutdownFlag,
            "BTC-USD",  // product ID
            20.0,       // minimum spread ($30)
            10.0,       // minimum profit ($20)
            0.001       // order size (0.001 BTC)
        );

        // Run the market maker in a separate thread
        std::thread marketMakerThread([&marketMaker]() {
            marketMaker.run();
        });

        // Run the io_context in the main thread
        ioc.run();

        // Wait for the market maker thread to finish
        if (marketMakerThread.joinable()) {
            marketMakerThread.join();
        }
    } else {
        std::cerr << "Error: BTC-USD order book not found" << std::endl;
        return 1;
    }

    return 0;
}
