#include "mainwindow.h"
#include <openssl/hmac.h>
#include <chrono>
#include "./ui_mainwindow.h"

MainWindow::MainWindow(WebSocketClient* client, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , ws_io_thread(nullptr)
    , http_io_thread(nullptr)
{
    ui->setupUi(this);
    
    connect(ui->liveButton, &QPushButton::clicked, this, &MainWindow::liveFunction);
    connect(ui->sandboxButton, &QPushButton::clicked, this, &MainWindow::sandboxFunction);

    onStartup();
}

MainWindow::~MainWindow()
{
    std::cout << "MainWindow::~MainWindow()" << std::endl;

    if (websocket_client) {
        websocket_client->close();
    }

    if (http_client) {
        http_client->shutdown();
    }

    while (!websocket_client->stopped) {} 
    while (!http_client->stopped) {} 

    while (ws_ioc.poll()) {}
    ws_ioc.stop();
    if (ws_io_thread && ws_io_thread->joinable()) {
        ws_io_thread->join();
    }

    while (http_ioc.poll()) {}
    http_ioc.stop();
    if (http_io_thread && http_io_thread->joinable()) {
        http_io_thread->join();
    }

    delete ui;
}

void MainWindow::onStartup()
{
    std::cout << "MainWindow::onStartup()" << std::endl;
    login();

    // 2. get accounts
    // 3. get & display balances
    // 4. get & display orders
    // 5. update local database
}

void MainWindow::login()
{
    auto host = "api.coinbase.com";
    auto port = "https";

    try {
        // The SSL context is required, and holds certificates
        ssl::context ctx{ssl::context::tlsv13_client};
        ctx.set_default_verify_paths();

        http_client = std::make_shared<HTTPClient>(net::make_strand(http_ioc), ctx);

        http_client->setReadCallback([this](const std::string& data) {
            processData(QString::fromStdString(data));
        });

        http_client->run(host, port);
        http_io_thread = std::make_unique<std::thread>([this](){ http_ioc.run(); });

    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void MainWindow::liveFunction()
{
    // @TODO: Remove hardcoded values
    // @TODO: Add error handling
    // @TODO: Add a stop button or handle multiple invocations of runFunction()
    // @TODO: Let the user decide host, port, and subscription
    std::cout << "MainWindow::liveFunction()" << std::endl;
    auto host = "advanced-trade-ws.coinbase.com";
    auto port = "443";

    std::string apiKey = std::getenv("COINBASE_API_KEY");
    std::string secretKey = std::getenv("COINBASE_SECRET_KEY");

    std::string type = "subscribe";
    std::vector<std::string> product_ids = {"BTC-USD"};
    std::string channel = "level2";
    std::string timestamp = getTimestamp();

    std::string message = timestamp + channel + product_ids[0];
    std::string signature = calculateSignature(message, secretKey);

    nlohmann::json j;
    j["type"] = type;
    j["product_ids"] = product_ids;
    j["channel"] = channel;
    j["signature"] = signature;
    j["api_key"] = apiKey;
    j["timestamp"] = timestamp;

    auto text = j.dump();
    std::cout << text << std::endl;

    try {
        // The SSL context is required, and holds certificates
        ssl::context ctx{ssl::context::tlsv13_client};
        ctx.set_default_verify_paths();

        websocket_client = std::make_shared<WebSocketClient>(ws_ioc, ctx);

        websocket_client->setReadCallback([this](const std::string& data) {
            processData(QString::fromStdString(data));
        });

        websocket_client->run(host, port, text);
        ws_io_thread = std::make_unique<std::thread>([this](){ ws_ioc.run(); });

    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void MainWindow::sandboxFunction()
{
    // @TODO: Remove hardcoded values
    // @TODO: Add error handling
    // @TODO: Add a stop button or handle multiple invocations of runFunction()
    // @TODO: Let the user decide host, port, and subscription
    std::cout << "MainWindow::sandboxFunction()" << std::endl;
    auto host = "ws-feed-public.sandbox.exchange.coinbase.com";
    auto port = "443";
    auto text = R"({"type": "subscribe","channels": ["ticker_batch"], "product_ids": ["BTC-USD", "ETH-USD"]})";

    try {
        // The SSL context is required, and holds certificates
        ssl::context ctx{ssl::context::tlsv13_client};
        ctx.set_default_verify_paths();

        websocket_client = std::make_shared<WebSocketClient>(ws_ioc, ctx);

        websocket_client->setReadCallback([this](const std::string& data) {
            processData(QString::fromStdString(data));
        });

        websocket_client->run(host, port, text);
        ws_io_thread = std::make_unique<std::thread>([this](){ ws_ioc.run(); });

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

std::string MainWindow::calculateSignature(const std::string& message, const std::string& secretKey)
{

    unsigned char hmacResult[EVP_MAX_MD_SIZE];
    unsigned int hmacLength;

    HMAC(EVP_sha256(), secretKey.c_str(), secretKey.length(),
         reinterpret_cast<const unsigned char*>(message.c_str()), message.length(), 
         hmacResult, &hmacLength);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for(unsigned int i = 0; i < hmacLength; ++i)
        ss << std::setw(2) << static_cast<unsigned>(hmacResult[i]);

    return ss.str();
}

std::string MainWindow::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration);

    double seconds = micros.count() / 1e6; // Converting microseconds to seconds.

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(0) << seconds; // Set precision to 6 decimal places.

    return ss.str();
}