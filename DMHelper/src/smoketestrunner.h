#ifndef SMOKETESTRUNNER_H
#define SMOKETESTRUNNER_H

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

class SmokeTestRunner
{
public:
    explicit SmokeTestRunner(bool skipOpenGL = false);

    int run();

private:
    void addResult(const QString& name, bool passed, const QString& detail = QString());
    void addSkipped(const QString& name, const QString& detail = QString());

    void checkEmbeddedResources();
    void checkFontRegistration();
    void checkAppResources();
    void checkAppDirectories();
    void checkVlcFiles();
    void checkVlcInit();
    void checkOpenGL();

    bool _skipOpenGL;
    QJsonArray _checks;
    int _passed;
    int _failed;
    int _skipped;
};

#endif // SMOKETESTRUNNER_H
