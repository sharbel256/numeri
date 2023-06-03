#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include "websocketclient.h"

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
    void updateWithNewData(const QString& data);
    void runFunction();

private:
    Ui::MainWindow *ui;
    std::shared_ptr<WebSocketClient> websocket_client;

    QLabel *label_2;
    QPushButton *eth_price;
    QPushButton *btc_price;

    QPushButton *runButton;

    // The io_context is required for all I/O
    net::io_context ioc;
    std::unique_ptr<std::thread> io_thread;

};
#endif // MAINWINDOW_H