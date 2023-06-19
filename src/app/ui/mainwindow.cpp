#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(session* client, QWidget *parent)
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
    // @TODO: Remove hardcoded values
    // @TODO: Add error handling
    // @TODO: Add a stop button or handle multiple invocations of runFunction()
    // @TODO: Let the user decide host, port, and subscription
    std::cout << "MainWindow::runFunction()" << std::endl;
    auto host = "ws-feed.exchange.coinbase.com";
    auto port = "443";
    auto text = R"({"type": "subscribe","channels": ["ticker_batch"], "product_ids": ["BTC-USD", "ETH-USD"]})";

    try {
        // The SSL context is required, and holds certificates
        ssl::context ctx{ssl::context::tlsv13_client};
        ctx.set_default_verify_paths();

        websocket_client = std::make_shared<session>(ioc, ctx);

        websocket_client->setOnReadCallback([this](const std::string& data) {
            processData(QString::fromStdString(data));
        });

        websocket_client->run(host, port, text);
        io_thread = std::make_unique<std::thread>([this](){ ioc.run(); });

    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void MainWindow::processData(const QString& data)
{
    // @TODO: create async data processing

    // Convert the QString to QByteArray
    QByteArray jsonBytes = data.toUtf8();

    // Create a QJsonDocument from the QByteArray
    QJsonDocument doc(QJsonDocument::fromJson(jsonBytes));

    // Get a QJsonObject from the QJsonDocument
    QJsonObject json = doc.object();

    // Now you can access values from the JSON object, for example:
    QString price = json["price"].toString();
    QString volume = json["volume_24h"].toString();
    QString time = json["time"].toString();

    double price_d, volume_d;

    price_d = price.toDouble();
    volume_d = volume.toDouble();

    price = QString::number(price_d, 'f', 1);
    volume = QString::number(volume_d, 'f', 1);

    if (json["product_id"].toString() == "BTC-USD") {
        updateBtc(price, volume, time);
    } else if (json["product_id"].toString() == "ETH-USD") {
        updateEth(price, volume, time);
    }
}

void MainWindow::updateBtc(const QString& price, const QString& volume, const QString& time)
{
    ui->btc_price->setText(price);
    ui->btc_volume->setText(volume);
    ui->btc_time->setText(time);
}

void MainWindow::updateEth(const QString& price, const QString& volume, const QString& time)
{
    ui->eth_price->setText(price);
    ui->eth_volume->setText(volume);
    ui->eth_time->setText(time);
}