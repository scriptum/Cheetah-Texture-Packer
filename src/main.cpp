#include <QtGui/QApplication>
#include "mainwindow.h"
#include <QTranslator>
#include <QLocale>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator myTranslator;
    myTranslator.load("tile_" + QLocale::system().name(), "qm");
    a.installTranslator(&myTranslator);
    MainWindow w;
    w.show();

    
    return a.exec();
}
