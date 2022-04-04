#include "mainwindow.h"
#include "dmhlogger.h"
#include <QApplication>
#include <QSurfaceFormat>
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // Explicitly set the application surface format for OpenGL surfaces    
#ifdef Q_OS_MAC
    // Explicitly set the application surface format for OpenGL surfaces
    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(fmt);
#endif

    // this important so we can call makeCurrent from our rendering thread
    QCoreApplication::setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity);

    int result = 0;

    try {
        DMHLogger logger;

        qInfo() << "[Main] #########################################################################################";
        qInfo() << "[Main]                                  DMHELPER STARTED";
        qInfo() << "[Main] #########################################################################################";

        MainWindow* w = new MainWindow;
        w->show();

        result = a.exec();

        qInfo() << "[Main] DMHelper exiting...";

        delete w;
        w = nullptr;

        qInfo() << "[Main] #########################################################################################";
        qInfo() << "[Main]                                 DMHELPER SHUT DOWN";
        qInfo() << "[Main] #########################################################################################";

    } catch (const std::exception& e) {
        qDebug() << "DMHelper threw an exception: " << e.what();
    } catch (...) {
        qDebug() << "DMHelper threw an unknown exception";
    }

    return result;
}
