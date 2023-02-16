#include "ui/mainwindow.h"
#include <QApplication>
#include <QString>
#include <iostream>
#include "service.h"

using namespace std;

int main(int argc, char *argv[])
{  
    Service service;
    cout << service.test << endl;
    QApplication a(argc, argv);

    QString message("Hello World!");
    MainWindow w;
    w.show();
    return a.exec();
}