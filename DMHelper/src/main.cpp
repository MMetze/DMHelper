#include "mainwindow.h"
#include "dmhlogger.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    int result = 0;

    try {
        DMHLogger logger;

        qInfo() << "[Main] #########################################################################################";
        qInfo() << "[Main]                                  DM HELPER STARTED";
        qInfo() << "[Main] #########################################################################################";

        MainWindow* w = new MainWindow;
        w->show();

        result = a.exec();

        qInfo() << "[Main] DM Helper exiting...";

        delete w;
        w = nullptr;

        qInfo() << "[Main] #########################################################################################";
        qInfo() << "[Main]                                 DM HELPER SHUT DOWN";
        qInfo() << "[Main] #########################################################################################";

    } catch (const std::exception& e) {
        qDebug() << "DM Helper threw an exception: " << e.what();
    } catch (...) {
        qDebug() << "DM Helper threw an unknown exception";
    }

    return result;
}
