// TODO: Separate UI and background into different queues

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "websocketclient.h"
#include "httpclient.h"
#include <iostream>
#include <string>
#include <thread>
#include <QLabel>
#include <QMainWindow>
#include <QJsonObject>
#include <QPushButton>
#include <QListWidget>
#include <QJsonDocument>
#include <QJsonArray>
#include <nlohmann/json.hpp>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(WebSocketClient* client = nullptr, QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onStartup();
    void liveFunction();
    void sandboxFunction();
    void login();
    void processData(const QString& data);
    void processLoginData(const QString& response);
    void updateBtc(const QString& price, const QString& volume, const QString& time);
    void updateEth(const QString& price, const QString& volume, const QString& time);
    std::string calculateSignature(const std::string& message, const std::string& secretKey);
    std::string getTimestamp();

private:
    Ui::MainWindow *ui;
    QLabel         *btc_time;
    QLabel         *eth_time;
    QPushButton    *btc_price;
    QPushButton    *eth_price;
    QPushButton    *btc_volume;
    QPushButton    *eth_volume;
    QPushButton    *liveButton;
    QPushButton    *sandboxButton;

    QPushButton    *cash_button;
    QPushButton    *btc_wallet_button;
    QPushButton    *eth_wallet_button;

    QListWidget   *list_widget;

    // The io_context is required for all I/O
    net::io_context ws_ioc;
    net::io_context http_ioc;

    std::unique_ptr<std::thread> ws_io_thread;
    std::unique_ptr<std::thread> http_io_thread;

    std::shared_ptr<WebSocketClient> websocket_client;
    std::shared_ptr<HTTPClient> http_client;
};
#endif // MAINWINDOW_H