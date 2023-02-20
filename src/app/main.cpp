#include "ui/mainwindow.h"
#include <QApplication>
#include <QString>
#include <string>
#include <iostream>
#include "service.h"

using namespace std;

int main(int argc, char *argv[])
{   
    Service service;
    service.sendMessage(R"({"type": "subscribe", "product_ids": ["ETH-USD"], "channels": ["level2"]})");
    QApplication a(argc, argv);

    QString message("Hello World!");
    MainWindow w;
    w.show();
    
    return a.exec();
}