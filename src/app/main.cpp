#include "ui/mainwindow.h"
#include <QApplication>
#include <QString>
#include <string>
#include <iostream>
#include <thread>
#include <client.cpp>

using namespace std;

int main(int argc, char *argv[])
{
    auto host = "ws-feed.exchange.coinbase.com";
    auto port = "443";
    auto text = R"({"type": "subscribe","channels": [{"name": "ticker", "product_ids": ["BTC-USD"]}]})";

    // The io_context is required for all I/O
    net::io_context ioc;

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv13_client};

    // This holds the root certificate used for verification
    ctx.set_default_verify_paths();
    

    // Launch the asynchronous operation
    std::make_shared<session>(ioc, ctx)->run(host, port, text);

    // Run the I/O service. The call will return when
    // the socket is closed.
    // Let's run it in a separate thread.
    std::thread io_thread = std::thread([&ioc](){ ioc.run(); });

    QApplication a(argc, argv);

    QString hello("Hello World!");
    MainWindow w;
    w.show();
    
    int ret = a.exec();
    ioc.stop();
    if (io_thread.joinable()) io_thread.join();

    return ret;
}