#include "dmhlogger.h"
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QStandardPaths>
#include <QDebug>

#ifndef QT_DEBUG
#define DMH_LOG_TO_FILE
#endif

DMHLogger* DMHLogger::_instance = nullptr;

const int MAX_NUMBER_OF_LOGFILES = 5;

void logOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if(DMHLogger::instance())
        DMHLogger::instance()->writeOutput(type, context, msg);
}

DMHLogger::DMHLogger() :
    _out(nullptr),
    _log(nullptr)
{
#ifdef DMH_LOG_TO_FILE
    if(!_instance)
    {
        initialize();
        _instance = this;
        qDebug() << "[DMHLogger] Logger initialized";
    }
#endif
}

DMHLogger::~DMHLogger()
{
#ifdef DMH_LOG_TO_FILE
    if(_instance)
    {
        qDebug() << "[DMHLogger] Logger shutting down";
        _instance = nullptr;
        shutdown();
    }
#endif
}

DMHLogger* DMHLogger::instance()
{
    return _instance;
}

void DMHLogger::initialize()
{
    if(_instance)
        return;

    QString logDirBasePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString logDirPath = logDirBasePath + QString("/log");
    QDir logDir(logDirPath);
    if(!logDir.exists())
    {
        QDir().mkpath(logDirPath);
        if(!logDir.exists())
        {
            qDebug() << "Log directory not found '" << logDirPath << "'. All debug output directed to console.";
            return;
        }
    }

    QStringList logFileList = logDir.entryList(QStringList(QString("*.log")),
                                               QDir::NoFilter,
                                               QDir::Time);

    if(logFileList.count() >= MAX_NUMBER_OF_LOGFILES)
    {
        QString removeLogFile =logFileList.last();
        if(logDir.exists(removeLogFile))
            logDir.remove(removeLogFile);
    }

    QString logFileName = QString("DMHelper-") + QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss") + QString(".log");
    QString fullLogFile = logDir.filePath(logFileName);

    _log = new QFile(fullLogFile);
    if(_log->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        _out = new QTextStream(_log);
        qInstallMessageHandler(logOutput);
    }
    else
    {
        qCritical() << "Error opening log file '" << fullLogFile << "'.  All debug output directed to console.";
        qCritical() << "Error: " << _log->error() << ", " << _log->errorString();
        delete _log;
        _log = nullptr;
    }
}

void DMHLogger::shutdown()
{
    qInstallMessageHandler(0);

    if(_out)
    {
        QTextStream* tempOut = _out;
        _out = nullptr;
        delete tempOut;
    }

    if(_log)
    {
        QFile* tempLog = _log;
        _log = nullptr;
        delete tempLog;
    }
}

void DMHLogger::writeOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if(!_out)
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
    (*_out) << debugdate << " " << msgContext << msg << Qt::endl;

    if (QtFatalMsg == type)
    {
        abort();
    }
}
