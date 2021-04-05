#include "dmc_serverconnection.h"
#include "dmc_constants.h"
#include "dmhnetworkmanager.h"
#include "dmhnetworkobserver.h"
#include "dmhpayload.h"
#include "dmhlogon.h"
#include "dmconstants.h"
#include "dmversion.h"
#include "remoteaudioplayer.h"
#include "remoterenderer.h"
#include "audiofactory.h"
#include "audiotrack.h"
#include <QFileDialog>
#include <QFile>
#include <QDebug>

DMC_ServerConnection::DMC_ServerConnection(const QString& cacheDirectory, QObject *parent) :
    QObject(parent),
    _networkManager(nullptr),
    _networkObserver(nullptr),
    _audioPlayer(new RemoteAudioPlayer(cacheDirectory, this)),
    _renderer(new RemoteRenderer(cacheDirectory, this)),
    _pmp(),
    _lastPayload(),
    _cacheDirectory(cacheDirectory)
{
    connectRemotePlayers();
}

DMC_ServerConnection::DMC_ServerConnection(const DMHLogon& logon, const QString& cacheDirectory, QObject *parent) :
    QObject(parent),
    _networkManager(nullptr),
    _networkObserver(nullptr),
    _audioPlayer(new RemoteAudioPlayer(cacheDirectory, this)),
    _renderer(new RemoteRenderer(cacheDirectory, this)),
    _pmp(),
    _lastPayload(),
    _cacheDirectory(cacheDirectory)
{
    connectRemotePlayers();
    startServer(logon);
}

DMC_ServerConnection::DMC_ServerConnection(const QString& urlString, const QString& username, const QString& password, const QString& session, const QString& cacheDirectory, QObject *parent) :
    QObject(parent),
    _networkManager(nullptr),
    _networkObserver(nullptr),
    _audioPlayer(new RemoteAudioPlayer(cacheDirectory, this)),
    _renderer(new RemoteRenderer(cacheDirectory, this)),
    _pmp(),
    _lastPayload(),
    _cacheDirectory(cacheDirectory)
{
    connectRemotePlayers();
    startServer(DMHLogon(urlString, username, password, session));
}

DMC_ServerConnection::~DMC_ServerConnection()
{
    stopServer();
}

void DMC_ServerConnection::downloadComplete(int requestID, const QString& fileMD5, const QByteArray& data)
{
    qDebug() << "[DMC_ServerConnection] Download complete " << requestID << ": " << fileMD5 << ", " << data.size() << " bytes";
    emit fileRequestCompleted(requestID, fileMD5, data);
}

void DMC_ServerConnection::payloadReceived(const DMHPayload& payload, const QString& timestamp)
{
    if((!_audioPlayer) || (!_renderer))
        return;

    if(timestamp == _lastPayload)
        return;

    qDebug() << "[DMC_ServerConnection] Payload received with new data. Image: " << payload.getImageFile() << ", Audio: " << payload.getAudioFile()<< ", Data: " << payload.getData() << ", Timestamp: " << timestamp;
    _renderer->parseImageData(payload.getImageFile(), payload.getData());
    _audioPlayer->parseAudioString(payload.getAudioFile());

    _lastPayload = timestamp;
}

void DMC_ServerConnection::startServer(const DMHLogon& logon)
{
    qDebug() << "[DMC_ServerConnection] Starting server with logon: " << logon;

    stopServer();

    _networkManager = new DMHNetworkManager(logon, this);
    connect(_networkManager, SIGNAL(downloadComplete(int, const QString&, const QByteArray&)), this, SLOT(downloadComplete(int, const QString&, const QByteArray&)));

    _networkObserver = new DMHNetworkObserver(logon, this);
    connect(_networkObserver, SIGNAL(payloadReceived(const DMHPayload&, const QString&)), this, SLOT(payloadReceived(const DMHPayload&, const QString&)));
    _networkObserver->start();
}

void DMC_ServerConnection::stopServer()
{
    qDebug() << "[DMC_ServerConnection] Stop server.";

    if(_networkManager)
    {
        disconnect(_networkManager, nullptr, this, nullptr);
        _networkManager->deleteLater();
        _networkManager = nullptr;
    }

    if(_networkObserver)
    {
        disconnect(_networkObserver, nullptr, this, nullptr);
        _networkObserver->deleteLater();
        _networkObserver = nullptr;
    }
}

void DMC_ServerConnection::fileRequested(const QString& md5String)
{
    QString cachedFile = _cacheDirectory + QString("/") + md5String;
    if(QFile::exists(cachedFile))
    {
        emit fileRequestStarted(-1, QString());
        emit fileRequestCompleted(-1, md5String, QByteArray());
    }
    else
    {
        emit fileRequestStarted(_networkManager->downloadFile(md5String), md5String);
    }
}

void DMC_ServerConnection::fileAborted(int requestID)
{
    if(_networkManager)
        _networkManager->abortRequest(requestID);
}

void DMC_ServerConnection::setCacheDirectory(const QString& cacheDirectory)
{
    _cacheDirectory = cacheDirectory;
}

void DMC_ServerConnection::targetResized(const QSize& newSize)
{
    if(_renderer)
        _renderer->targetResized(newSize);
}

void DMC_ServerConnection::connectRemotePlayers()
{
    if((!_audioPlayer) || (!_renderer))
        return;

    connect(_audioPlayer, &RemoteAudioPlayer::requestFile, this, &DMC_ServerConnection::fileRequested);
    connect(this, &DMC_ServerConnection::fileRequestStarted, _audioPlayer, &RemoteAudioPlayer::fileRequestStarted);
    connect(this, &DMC_ServerConnection::fileRequestCompleted, _audioPlayer, &RemoteAudioPlayer::fileRequestCompleted);

    connect(_renderer, &RemoteRenderer::requestFile, this, &DMC_ServerConnection::fileRequested);
    connect(_renderer, &RemoteRenderer::abortRequest, this, &DMC_ServerConnection::fileAborted);
    connect(_renderer, &RemoteRenderer::publishPixmap, this, &DMC_ServerConnection::pixmapActive);
    connect(_renderer, &RemoteRenderer::publishImage, this, &DMC_ServerConnection::imageActive);
    connect(this, &DMC_ServerConnection::fileRequestStarted, _renderer, &RemoteRenderer::fileRequestStarted);
    connect(this, &DMC_ServerConnection::fileRequestCompleted, _renderer, &RemoteRenderer::fileRequestCompleted);
}
