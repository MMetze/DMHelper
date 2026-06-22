#include "smoketestrunner.h"
#include "optionscontainer.h"
#include "dmh_vlc.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QJsonDocument>
#include <QJsonObject>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QTextStream>
#include <QDebug>

SmokeTestRunner::SmokeTestRunner(bool skipOpenGL) :
    _skipOpenGL(skipOpenGL),
    _checks(),
    _passed(0),
    _failed(0),
    _skipped(0)
{
}

int SmokeTestRunner::run()
{
    qInfo() << "[SmokeTest] Starting smoke test...";

    checkEmbeddedResources();
    checkFontRegistration();
    checkAppResources();
    checkAppDirectories();
    checkVlcFiles();
    checkVlcInit();
    checkOpenGL();

    int total = _passed + _failed + _skipped;
    bool allPassed = (_failed == 0);

    QJsonObject summary;
    summary[QString("total")] = total;
    summary[QString("passed")] = _passed;
    summary[QString("failed")] = _failed;
    summary[QString("skipped")] = _skipped;

    QJsonObject root;
    root[QString("passed")] = allPassed;
    root[QString("checks")] = _checks;
    root[QString("summary")] = summary;

    QJsonDocument doc(root);
    QTextStream out(stdout);
    out << doc.toJson(QJsonDocument::Compact) << Qt::endl;

    qInfo() << "[SmokeTest] Complete:" << _passed << "passed," << _failed << "failed," << _skipped << "skipped";
    return allPassed ? 0 : 1;
}

void SmokeTestRunner::addResult(const QString& name, bool passed, const QString& detail)
{
    QJsonObject check;
    check[QString("name")] = name;
    check[QString("passed")] = passed;
    if(!detail.isEmpty())
        check[QString("detail")] = detail;
    _checks.append(check);

    if(passed)
        _passed++;
    else
        _failed++;
}

void SmokeTestRunner::addSkipped(const QString& name, const QString& detail)
{
    QJsonObject check;
    check[QString("name")] = name;
    check[QString("skipped")] = true;
    if(!detail.isEmpty())
        check[QString("detail")] = detail;
    _checks.append(check);
    _skipped++;
}

void SmokeTestRunner::checkEmbeddedResources()
{
    const QStringList resources = {
        QString(":/img/data/dmhelper_opaque.png"),
        QString(":/img/data/parchment.jpg"),
        QString(":/img/data/fonts/Rellanic-Agx7.ttf"),
        QString(":/img/data/fonts/Davek-vGXA.ttf"),
        QString(":/img/data/fonts/Iokharic-dqvK.ttf"),
        QString(":/img/data/plus.png")
    };

    for(const QString& res : resources)
    {
        bool exists = QFile::exists(res);
        QString shortName = res.mid(res.lastIndexOf('/') + 1);
        addResult(QString("embedded_") + shortName, exists, exists ? QString() : QString("not found: ") + res);
    }
}

void SmokeTestRunner::checkFontRegistration()
{
    const QStringList fonts = {
        QString(":/img/data/fonts/Rellanic-Agx7.ttf"),
        QString(":/img/data/fonts/Davek-vGXA.ttf"),
        QString(":/img/data/fonts/Iokharic-dqvK.ttf")
    };

    for(const QString& font : fonts)
    {
        int id = QFontDatabase::addApplicationFont(font);
        QString shortName = font.mid(font.lastIndexOf('/') + 1);
        addResult(QString("font_") + shortName, id >= 0, id >= 0 ? QString() : QString("registration failed"));
    }
}

void SmokeTestRunner::checkAppResources()
{
    QStringList expected = OptionsContainer::getExpectedAppResources();
    for(const QString& filePath : expected)
    {
        bool exists = QFile::exists(filePath);
        QString shortName = QFileInfo(filePath).fileName();
        addResult(QString("app_resource_") + shortName, exists, exists ? QString() : QString("not found: ") + filePath);
    }
}

void SmokeTestRunner::checkAppDirectories()
{
    QStringList expected = OptionsContainer::getExpectedAppDirectories();
    for(const QString& dirPath : expected)
    {
        bool exists = QDir(dirPath).exists();
        QString shortName = QDir(dirPath).dirName();
        addResult(QString("app_dir_") + shortName, exists, exists ? QString() : QString("not found: ") + dirPath);
    }
}

void SmokeTestRunner::checkVlcFiles()
{
    QString appDir = QCoreApplication::applicationDirPath();

#ifdef Q_OS_MAC
    QDir frameworksDir(appDir);
    frameworksDir.cdUp();
    QString libPath = frameworksDir.filePath(QString("Frameworks/libvlc.dylib"));
    QString pluginsPath = frameworksDir.filePath(QString("Frameworks/plugins"));

    addResult(QString("vlc_library"), QFile::exists(libPath), QFile::exists(libPath) ? QString() : QString("not found: ") + libPath);
    addResult(QString("vlc_plugins_dir"), QDir(pluginsPath).exists(), QDir(pluginsPath).exists() ? QString() : QString("not found: ") + pluginsPath);
#else
    QString libPath = appDir + QString("/libvlc.dll");
    QString pluginsPath = appDir + QString("/plugins/plugins.dat");

    addResult(QString("vlc_library"), QFile::exists(libPath), QFile::exists(libPath) ? QString() : QString("not found: ") + libPath);
    addResult(QString("vlc_plugins_dat"), QFile::exists(pluginsPath), QFile::exists(pluginsPath) ? QString() : QString("not found: ") + pluginsPath);
#endif
}

void SmokeTestRunner::checkVlcInit()
{
    // Create a standalone VLC instance with --reset-plugins-cache to avoid
    // stale cache errors that produce massive stderr noise on CI.
    const char *args[] = {
        "--reset-plugins-cache",
        "--no-audio",
        "--no-video",
        "--verbose=0",
        ""
    };

    libvlc_instance_t* instance = libvlc_new(sizeof(args) / sizeof(*args), args);
    bool ok = (instance != nullptr);
    addResult(QString("vlc_init"), ok, ok ? QString() : QString("libvlc_new returned null"));
    if(instance)
        libvlc_release(instance);
}

void SmokeTestRunner::checkOpenGL()
{
    if(_skipOpenGL)
    {
        addSkipped(QString("opengl_context"), QString("skipped via --skip-opengl"));
        return;
    }

    QOffscreenSurface surface;
    surface.create();
    if(!surface.isValid())
    {
        addResult(QString("opengl_context"), false, QString("QOffscreenSurface creation failed"));
        return;
    }

    QOpenGLContext context;
    bool created = context.create();
    if(!created)
    {
        addResult(QString("opengl_context"), false, QString("QOpenGLContext creation failed"));
        surface.destroy();
        return;
    }

    context.makeCurrent(&surface);
    QSurfaceFormat fmt = context.format();
    QString version = QString::number(fmt.majorVersion()) + QString(".") + QString::number(fmt.minorVersion());
    context.doneCurrent();
    surface.destroy();

    addResult(QString("opengl_context"), true, version);
}
