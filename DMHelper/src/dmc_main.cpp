#include "dmc_mainwindow.h"
#include "dmhlogger.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    int result = 0;

    try {
        DMHLogger logger;

        qInfo() << "[Main] #########################################################################################";
        qInfo() << "[Main]                               DM HELPER CLIENT STARTED";
        qInfo() << "[Main] #########################################################################################";

        DMC_MainWindow* w = new DMC_MainWindow;
        w->show();

        result = a.exec();

        qInfo() << "DM Helper Client exiting.";

        delete w;
        w = nullptr;

        qInfo() << "[Main] #########################################################################################";
        qInfo() << "[Main]                               DM HELPER CLIENT SHUT DOWN";
        qInfo() << "[Main] #########################################################################################";

    } catch (const std::exception& e) {
        qDebug() << "DM Helper Client threw an exception: " << e.what();
    } catch (...) {
        qDebug() << "DM Helper Client threw an unknown exception";
    }

    return result;
}
