#include "mainwindow.h"
#include "dmhlogger.h"
#include "optionsaccessor.h"
#include <QApplication>
#include <QSurfaceFormat>
#include <QDebug>

int main(int argc, char *argv[]) {
    QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    QApplication a(argc, argv);

    // Explicitly set the application surface format for OpenGL surfaces
    QSurfaceFormat fmt;
#ifdef Q_OS_MAC
    fmt.setVersion(4, 1);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
#endif
    fmt.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(fmt);

    // this important so we can call makeCurrent from our rendering thread
    QCoreApplication::setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity);

    int result = 0;
    try {
        DMHLogger logger;

        qInfo() << "[Main] #########################################################################################";
        qInfo() << "[Main]                                  DMHELPER STARTED";
        qInfo() << "[Main] #########################################################################################";

        QStringList arguments = QCoreApplication::arguments();
        if(arguments.contains("-clear-settings"))
        {
            qInfo() << "[Main] WARNING: CLEARING ALL REGISTRY SETTINGS";
            OptionsAccessor settings;
            settings.remove("");
        }

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
