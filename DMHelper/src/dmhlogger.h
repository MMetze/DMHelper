#ifndef DMHLOGGER_H
#define DMHLOGGER_H

#include <QtGlobal>

class QTextStream;
class QFile;

class DMHLogger
{
public:
    DMHLogger();
    ~DMHLogger();

    static DMHLogger* instance();
    static QString getLogDirPath();

    void initialize();
    void shutdown();

    void writeOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    QTextStream* _out;
    QFile* _log;

    static DMHLogger* _instance;
};

#endif // DMHLOGGER_H
