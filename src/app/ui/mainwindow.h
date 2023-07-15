// TODO: Separate UI and background into different queues

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "websocketclient.h"
#include <iostream>
#include <string>
#include <thread>
#include <QLabel>
#include <QMainWindow>
#include <QJsonObject>
#include <QPushButton>
#include <QJsonDocument>
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
    void liveFunction();
    void sandboxFunction();
    void processData(const QString& data);
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


    // The io_context is required for all I/O
    net::io_context ioc;
    std::unique_ptr<std::thread> io_thread;

    std::shared_ptr<WebSocketClient> websocket_client;
};
#endif // MAINWINDOW_H