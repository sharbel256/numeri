#include "trading.h"
#include "readers.h"
#include "orderbuilder.cpp"
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

    // Start the reader thread
    // std::thread readerThread([&]()
    // {
    //     auto& orderBooks = trading.getOrderbooks();
    //     if (orderBooks.find("BTC-USD") != orderBooks.end()) {
    //         readers::readerThreadFunction(orderBooks["BTC-USD"], shutdownFlag);
    //     }
    // });

    // Start trading operations
    trading.getAccounts();
    trading.startWebsocket();

    std::string requestBody = R"({
        "product_id": "BTC-USD",
        "side": "BUY",
        "order_configuration": {
            "limit_limit_gtc": {
                "base_size": "0.001",
                "limit_price": "45000.00",
                "post_only": false
            }
        }
    })";

    std::string limitGtcOrder = OrderBuilder()
            .setClientOrderId("selling-btc-001")
            .setProductId("BTC-USD")
            .setSide("SELL")
            .limitLimitGtc("0.001", "100000.00", false) // Sell 0.001 BTC at 100,000
            .build();
    
    std::cout << "hello" << std::endl;
    std::string response = trading.createOrder(limitGtcOrder);
    std::cout << response << std::endl;
    
    // Run the io_context in the main thread
    ioc.run();
    
    // Wait for the reader thread to finish
    // if (readerThread.joinable())
    // {
    //     readerThread.join();
    // }

    return 0;
}
