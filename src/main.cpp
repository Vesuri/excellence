#include "mainwindow.h"
#include <QApplication>
#include <QtPlugin>

Q_IMPORT_PLUGIN(ILBMPlugin)
Q_IMPORT_PLUGIN(RawPlugin)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowFlag(Qt::Tool);
    w.show();

    return a.exec();
}
