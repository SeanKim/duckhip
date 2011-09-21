#include <QtGui/QApplication>
#include "mainwindow.h"
#include "define.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/images/linux.png"));
    MainWindow w;
    w.show();

    return a.exec();
}
