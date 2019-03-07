#include "mainwindow.h"
#include <QApplication>
#include <QDateTime>
#include <QTextStream>
#include <QtGlobal>
#include <QDebug>
#include <QSplashScreen>

#ifndef QT_NO_DEBUG_OUTPUT
    QTextStream *out = nullptr;

    void logOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
        if(!out)
            return;

        QByteArray localMsg = msg.toLocal8Bit();
        QString debugdate = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss");
        QString msgContext;
        switch (type) {
            case QtDebugMsg:
                msgContext = QString("Debug: [%1:%2, %3] ").arg(context.file).arg(context.line).arg(context.function);
                break;
            case QtInfoMsg:
                msgContext = QString("Info: [%1:%2, %3] ").arg(context.file).arg(context.line).arg(context.function);
                break;
            case QtWarningMsg:
                msgContext = QString("Warning: [%1:%2, %3] ").arg(context.file).arg(context.line).arg(context.function);
                break;
            case QtCriticalMsg:
                msgContext = QString("Critical: [%1:%2, %3] ").arg(context.file).arg(context.line).arg(context.function);
                break;
            case QtFatalMsg:
                msgContext = QString("Fatal: [%1:%2, %3] ").arg(context.file).arg(context.line).arg(context.function);
                break;
        }
        (*out) << debugdate << " " << msgContext << msg << endl;

        if (QtFatalMsg == type) {
            abort();
        }
    }
#endif


int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QPixmap pixmap(":/img/data/dmhelper_large.png");
    QSplashScreen splash(pixmap);
    splash.show();

#ifndef QT_NO_DEBUG_OUTPUT
    QString fileName = QCoreApplication::applicationFilePath().replace(".exe", ".log");
    QFile *log = nullptr;

    if(QFile::exists(fileName)) {
        log = new QFile(fileName);
        if(log->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            out = new QTextStream(log);
            qInstallMessageHandler(logOutput);
        } else {
            delete log;
            log = nullptr;
            out = nullptr;
            qCritical() << "Error opening log file '" << fileName << "'. All debug output redirected to console.";
        }
    } else {
        qDebug() << "Logfile not found '" << fileName << "'. All debug output redirected to console.";
    }

#endif

    int result = 0;

    try {
        qInfo() << "DM Helper started.";

        MainWindow* w = new MainWindow;
        w->show();

        result = a.exec();

        qInfo() << "DM Helper exiting.";

        delete w;
        w = nullptr;

        if(log){
            qInstallMessageHandler(nullptr);
            delete out;
            out = nullptr;
            delete log;
            log = nullptr;
        }
    } catch (const std::exception& e) {
        qDebug() << "DM Helper threw an exception: " << e.what();
    } catch (...) {
        qDebug() << "DM Helper threw an unknown exception";
    }

    return result;
}
