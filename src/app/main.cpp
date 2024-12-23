#include "trading.h"
#include "readers.h"
#include <thread>
#include <boost/asio/signal_set.hpp>
#include <atomic>

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

    // Start the reader thread
    std::thread readerThread([&]()
    {
        auto& orderBooks = trading.getOrderbooks();
        if (orderBooks.find("BTC-USD") != orderBooks.end()) {
            readers::readerThreadFunction(orderBooks["BTC-USD"], shutdownFlag);
        }
    });

    // Start trading operations
    trading.getAccounts();
    trading.startWebsocket();

    // Run the io_context in the main thread
    ioc.run();
    
    // Wait for the reader thread to finish
    if (readerThread.joinable())
    {
        readerThread.join();
    }

    return 0;
}
