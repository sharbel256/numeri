#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <thread>
#include <QString>
#include <string>
#include <iostream>
#include <QJsonDocument>
#include <QJsonObject>


MainWindow::MainWindow(WebSocketClient* client, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , io_thread(nullptr)
{
    std::cout << "MainWindow::MainWindow()" << std::endl;

    ui->setupUi(this);

    connect(ui->runButton, &QPushButton::clicked, this, &MainWindow::runFunction);
}

MainWindow::~MainWindow()
{   
    std::cout << "MainWindow::~MainWindow()" << std::endl;

    if (websocket_client) {
        websocket_client->close();
    }

    while (!websocket_client->stopped) {}

    while (ioc.poll()) {}
    ioc.stop();
    if (io_thread && io_thread->joinable()) {
        io_thread->join();
        
    }

    delete ui;
}


void MainWindow::updateWithNewData(const QString& data)
{
    // Update your UI with the new data here
    

    // Convert the QString to QByteArray
    QByteArray jsonBytes = data.toUtf8();

    // Create a QJsonDocument from the QByteArray
    QJsonDocument doc(QJsonDocument::fromJson(jsonBytes));

    // Get a QJsonObject from the QJsonDocument
    QJsonObject json = doc.object();

    // Now you can access values from the JSON object, for example:
    QString price = json["price"].toString();

    ui->btc_price->setText(price);
}

void MainWindow::runFunction()
{
    std::cout << "MainWindow::runFunction()" << std::endl;
    auto host = "ws-feed.exchange.coinbase.com";
    auto port = "443";
    auto text = R"({"type": "subscribe","channels": [{"name": "ticker", "product_ids": ["BTC-USD"]}]})";

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv13_client};

    // This holds the root certificate used for verification
    ctx.set_default_verify_paths();

    websocket_client = std::make_shared<WebSocketClient>(ioc, ctx);

    connect(websocket_client.get(), &WebSocketClient::newDataReceived, this, &MainWindow::updateWithNewData);

    try {
        websocket_client->run(host, port, text);
        io_thread = std::make_unique<std::thread>([this](){ ioc.run(); });
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }   
}