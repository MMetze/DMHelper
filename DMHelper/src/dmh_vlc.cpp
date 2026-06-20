#include "dmh_vlc.h"
#include "videoplayergl.h"
#include "videoplayerglvideo.h"
#include "dmhwaitingdialog.h"
#include <QApplication>
#include <QEventLoop>
#include <QThread>
#include <QTimerEvent>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QDebug>
#include <cstdarg>

//#define VIDEO_DEBUG_MESSAGES
//#define VIDEO_CREATE_CACHE

DMH_VLC* DMH_VLC::_instance = nullptr;

namespace
{
#ifndef Q_OS_MAC
// Name of the libVLC plugin description cache. Its presence lets libVLC skip
// loading every plugin binary just to read its description, which makes
// startup dramatically faster. libVLC only ever writes this file when started
// with --reset-plugins-cache; a normal cached startup just reads it.
const QString VlcPluginCacheFileName = QStringLiteral("plugins.dat");

// Resolves the on-disk path where libVLC keeps (or expects) its plugin cache.
// Mirrors the plugin directory layout that DMHelper deploys per platform.
QString vlcPluginCacheFilePath()
{
#if defined(Q_OS_WIN)
    return QCoreApplication::applicationDirPath() + QStringLiteral("/plugins/") + VlcPluginCacheFileName;
#else
    const QByteArray envPluginPath = qgetenv("VLC_PLUGIN_PATH");
    if(!envPluginPath.isEmpty())
        return QString::fromLocal8Bit(envPluginPath) + QDir::separator() + VlcPluginCacheFileName;
    return QCoreApplication::applicationDirPath() + QStringLiteral("/plugins/") + VlcPluginCacheFileName;
#endif
}
#endif

class VlcInitializationThread final : public QThread
{
public:
    libvlc_instance_t* result;

    explicit VlcInitializationThread(bool rebuildPluginCache) :
        QThread(),
        result(nullptr),
        _rebuildPluginCache(rebuildPluginCache)
    {
    }

protected:
    virtual void run() override
    {
#ifndef Q_OS_MAC
        // If the plugin cache is missing, rebuild it up front so the file is
        // created and subsequent launches take the fast cached path.
        if(_rebuildPluginCache)
        {
            qDebug() << "[DMH_VLC] Plugin cache missing; starting libVLC with a plugin cache rebuild to create it";
            result = createVlcInstance(true);
            if(result)
                qDebug() << "[DMH_VLC] libVLC startup with plugin cache rebuild succeeded";
            else
                qDebug() << "[DMH_VLC] libVLC startup with plugin cache rebuild failed";
            return;
        }

        result = createVlcInstance(false);
        if(result)
        {
            qDebug() << "[DMH_VLC] libVLC startup with existing plugin cache succeeded";
            return;
        }

        // The cache exists but startup failed (e.g. a stale cache after a
        // libVLC version change); recover by rebuilding it.
        qDebug() << "[DMH_VLC] libVLC startup with existing plugin cache failed; retrying with a plugin cache rebuild";
        result = createVlcInstance(true);

        if(result)
            qDebug() << "[DMH_VLC] Recovery libVLC startup with plugin cache rebuild succeeded";
        else
            qDebug() << "[DMH_VLC] Recovery libVLC startup with plugin cache rebuild failed";
#else
        result = libvlc_new(0, nullptr);
#endif
    }

private:
#ifndef Q_OS_MAC
    static libvlc_instance_t* createVlcInstance(bool rebuildPluginCache)
    {
        if(rebuildPluginCache)
        {
            const char *args[] = {
                "--reset-plugins-cache",
                "--plugins-cache",
                "--plugins-scan",
                "--verbose=0"
            };
            return libvlc_new(sizeof(args) / sizeof(*args), args);
        }

        const char *args[] = {
            "--no-reset-plugins-cache",
            "--plugins-cache",
            "--verbose=0"
        };
        return libvlc_new(sizeof(args) / sizeof(*args), args);
    }
#endif

    bool _rebuildPluginCache;
};

void libVlcLogCallback(void* data, int level, const libvlc_log_t* context, const char* fmt, va_list args)
{
    Q_UNUSED(data)
    Q_UNUSED(context)

    if(!fmt)
        return;

    va_list argsCopy;
    va_copy(argsCopy, args);
    int required = vsnprintf(nullptr, 0, fmt, argsCopy);
    va_end(argsCopy);

    QString message;
    if(required > 0)
    {
        QByteArray buffer(required + 1, '\0');
        va_copy(argsCopy, args);
        vsnprintf(buffer.data(), static_cast<size_t>(buffer.size()), fmt, argsCopy);
        va_end(argsCopy);
        message = QString::fromUtf8(buffer.constData()).trimmed();
    }

    if(message.isEmpty())
        return;

    switch(level)
    {
        case LIBVLC_DEBUG:
            qDebug().noquote() << "[libvlc]" << message;
            break;
        case LIBVLC_NOTICE:
            qInfo().noquote() << "[libvlc]" << message;
            break;
        case LIBVLC_WARNING:
            qWarning().noquote() << "[libvlc]" << message;
            break;
        case LIBVLC_ERROR:
            qCritical().noquote() << "[libvlc]" << message;
            break;
        default:
            qInfo().noquote() << "[libvlc]" << message;
            break;
    }
}
}

DMH_VLC::DMH_VLC(QObject *parent) :
    QObject(parent),
    _vlcInstance(nullptr),
    _currentVideo(nullptr)
{
#ifndef Q_OS_MAC
    const QString cacheFilePath = vlcPluginCacheFilePath();
    const bool rebuildPluginCache = !QFileInfo::exists(cacheFilePath);
    if(rebuildPluginCache)
        qDebug() << "[DMH_VLC] Plugin cache not found at" << cacheFilePath << "; it will be created on startup";
#else
    const bool rebuildPluginCache = false;
#endif

    VlcInitializationThread initThread(rebuildPluginCache);
    DMHWaitingDialog waitingDlg(QString("Initializing DMHelper's video player.."));
    waitingDlg.setModal(true);
    waitingDlg.show();
    qApp->processEvents();

    QEventLoop waitLoop;
    connect(&initThread, &QThread::finished, &waitLoop, &QEventLoop::quit);
    initThread.start();
    waitLoop.exec();
    initThread.wait();

    _vlcInstance = initThread.result;

    waitingDlg.close();

    if(_vlcInstance)
        libvlc_log_set(_vlcInstance, libVlcLogCallback, nullptr);
}

DMH_VLC::~DMH_VLC()
{
    if(_vlcInstance)
    {
        libvlc_release(_vlcInstance);
        _vlcInstance = nullptr;
    }
}

DMH_VLC* DMH_VLC::DMH_VLCInstance()
{
    if(_instance)
        return _instance;

    Initialize();
    return _instance;
}

libvlc_instance_t* DMH_VLC::vlcInstance()
{
    if(!_instance)
    {
        Initialize();
        if(!_instance)
        {
            qDebug() << "[DMH_VLC] ERROR: unable to find or initialize a VLC instance for playback!";
            return nullptr;
        }
    }

    return _instance->_vlcInstance;
}

void DMH_VLC::Initialize()
{
    if(_instance)
        return;

    _instance = new DMH_VLC();
}

void DMH_VLC::Shutdown()
{
    if(_instance)
    {
        _instance->releaseVideo(_instance->_currentVideo);

        DMH_VLC* deleteInstance = _instance;
        _instance = nullptr;
        delete deleteInstance;
    }
}

VideoPlayerGLVideo* DMH_VLC::requestVideo(VideoPlayerGL* player)
{
    if((_currentVideo) || (!player))
        return nullptr;

    _currentVideo = new VideoPlayerGLVideo(player);
    qDebug() << "[DMH_VLC] New video created (" << reinterpret_cast<void *>(_currentVideo) << ") for player: " << reinterpret_cast<void *>(player);

    return _currentVideo;
}

bool DMH_VLC::releaseVideo(VideoPlayerGLVideo* video)
{
    if((!_currentVideo) || (video != _currentVideo))
        return false;

    delete _currentVideo;
    qDebug() << "[DMH_VLC] Video released: " << reinterpret_cast<void *>(_currentVideo);
    startTimer(1000);

    return true;
}

void DMH_VLC::timerEvent(QTimerEvent *event)
{
    if(!event)
        return;

    killTimer(event->timerId());
    _currentVideo = nullptr;
    qDebug() << "[DMH_VLC] Video now available";
    emit playerAvailable();
}

