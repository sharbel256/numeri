#include "mainwindow.h"
#include "./ui_mainwindow.h"

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

void MainWindow::runFunction()
{
    // TODO: Remove hardcoded values
    // TODO: Add error handling
    // TODO: Add a stop button or handle multiple invocation of runFunction()
    // TODO: Let the user decide host, port, and subscription
    std::cout << "MainWindow::runFunction()" << std::endl;
    auto host = "ws-feed.exchange.coinbase.com";
    auto port = "443";
    auto text = R"({"type": "subscribe","channels": [{"name": "ticker", "product_ids": ["BTC-USD"]}]})";

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv13_client};
    ctx.set_default_verify_paths();

    websocket_client = std::make_shared<WebSocketClient>(ioc, ctx);

    connect(websocket_client.get(), &WebSocketClient::newDataReceived, this, &MainWindow::processData);

    try {
        websocket_client->run(host, port, text);
        io_thread = std::make_unique<std::thread>([this](){ ioc.run(); });
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }   
}

void MainWindow::processData(const QString& data)
{
    // TODO: create async data processing
    std::cout << "MainWindow::processData()" << std::endl;

    // Convert the QString to QByteArray
    QByteArray jsonBytes = data.toUtf8();

    // Create a QJsonDocument from the QByteArray
    QJsonDocument doc(QJsonDocument::fromJson(jsonBytes));

    // Get a QJsonObject from the QJsonDocument
    QJsonObject json = doc.object();

    // Now you can access values from the JSON object, for example:
    QString price = json["price"].toString();

    updatePrice(price);
}

void MainWindow::updatePrice(const QString& price)
{
    ui->btc_price->setText(price);
}