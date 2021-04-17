#include "dmc_serverconnection.h"
#include "dmc_constants.h"
#include "dmc_optionscontainer.h"
#include "dmc_connectionsettingsdialog.h"
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
#include <QGuiApplication>
#include <QScreen>
#include <QMessageBox>
#include <QDebug>

DMC_ServerConnection::DMC_ServerConnection(DMC_OptionsContainer& options, QObject *parent) :
    QObject(parent),
    _connected(false),
    _options(options),
    _session(),
    _networkManager(nullptr),
    _networkObserver(nullptr),
    _audioPlayer(new RemoteAudioPlayer(options.getCacheDirectory(), this)),
    _renderer(new RemoteRenderer(options.getCacheDirectory(), this)),
    _pmp(),
    _lastPayload()
{
    connectRemotePlayers();
}

DMC_ServerConnection::~DMC_ServerConnection()
{
    stopServer();
}

bool DMC_ServerConnection::isConnected() const
{
    return _connected;
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

void DMC_ServerConnection::connectServer(bool connect)
{
    if(_connected == connect)
        return;

    qDebug() << "[DMC_ServerConnection] Connecting server: " << connect;

    if(connect)
        checkLogon();
    else
        stopServer();
}

void DMC_ServerConnection::checkLogon()
{
    if((!_options.getLogon().isValid()) || (_options.getCurrentInvite().isEmpty()))
    {
        qDebug() << "[DMC_ServerConnection] Login data not valid, opening connection settings.";
        DMC_ConnectionSettingsDialog dlg(_options);
        QScreen* primary = QGuiApplication::primaryScreen();
        if(primary)
            dlg.resize(primary->availableSize().width() / 3, primary->availableSize().height() / 3);

        dlg.exec();
        if((!_options.getLogon().isValid()) || (_options.getCurrentInvite().isEmpty()))
        {
            qDebug() << "[DMC_ServerConnection] Login data not valid, disconnecting.";
            emit connectionChanged(false);
            return;
        }
    }

    qDebug() << "[DMC_ServerConnection] Login data valid, attempting to join the session. Invite: " << _options.getCurrentInvite();

    startManager();
    joinSession();
}

void DMC_ServerConnection::startServer()
{
    qDebug() << "[DMC_ServerConnection] Starting server with logon: " << _options.getLogon();

    if(_connected)
        stopServer();

    startManager();
    startObserver();

    if(!_connected)
    {
        emit connectionChanged(true);
        _connected = true;
    }

    if(_networkManager)
        _networkManager->sendMessage(QString("join"));
}

void DMC_ServerConnection::stopServer()
{
    qDebug() << "[DMC_ServerConnection] Stop server.";

    stopManager();
    stopObserver();

    if(_connected)
    {
        emit connectionChanged(false);
        _connected = false;
    }
}

void DMC_ServerConnection::fileRequested(const QString& md5String)
{
    QString cachedFile = _options.getCacheDirectory() + QString("/") + md5String;
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

void DMC_ServerConnection::targetResized(const QSize& newSize)
{
    if(_renderer)
        _renderer->targetResized(newSize);
}

void DMC_ServerConnection::joinSessionComplete(int requestID, const QString& session)
{
    Q_UNUSED(requestID);

    if(session.isEmpty())
        return;

    qDebug() << "[DMC_ServerConnection] Successfully joined the session. Session ID: " << session;
    _session = session;
    startServer();
}

void DMC_ServerConnection::messageError(int requestID, const QString& errorString)
{
    QMessageBox::critical(nullptr, QString("Server Error"), QString("An error was received from the DMH server:\n\n") + errorString);
    qDebug() << "[DMC_ServerConnection] Error in message, stopping the server. Request: " << requestID << ", error: " << errorString;
    stopServer();
}

void DMC_ServerConnection::requestError(int requestID)
{
    qDebug() << "[DMC_ServerConnection] Error in connection, stopping the server. Request: " << requestID;
    stopServer();
}

void DMC_ServerConnection::startManager()
{
    DMHLogon logon = _options.getLogon();
    logon.setSession(_session);

    if(_networkManager)
    {
        _networkManager->setLogon(logon);
    }
    else
    {
        _networkManager = new DMHNetworkManager(logon, this);
        connect(_networkManager, &DMHNetworkManager::downloadComplete, this, &DMC_ServerConnection::downloadComplete);
        connect(_networkManager, &DMHNetworkManager::joinSessionComplete, this, &DMC_ServerConnection::joinSessionComplete);
        connect(_networkManager, &DMHNetworkManager::messageError, this, &DMC_ServerConnection::messageError);
        connect(_networkManager, &DMHNetworkManager::requestError, this, &DMC_ServerConnection::requestError);
    }
}

void DMC_ServerConnection::stopManager()
{
    if(_networkManager)
    {
        disconnect(_networkManager, nullptr, this, nullptr);
        _networkManager->deleteLater();
        _networkManager = nullptr;
    }
}

void DMC_ServerConnection::startObserver()
{
    if(!_networkObserver)
    {
        DMHLogon logon = _options.getLogon();
        logon.setSession(_session);
        _networkObserver = new DMHNetworkObserver(logon, this);
        connect(_networkObserver, SIGNAL(payloadReceived(const DMHPayload&, const QString&)), this, SLOT(payloadReceived(const DMHPayload&, const QString&)));
        _networkObserver->start();
    }
}

void DMC_ServerConnection::stopObserver()
{
    if(_networkObserver)
    {
        disconnect(_networkObserver, nullptr, this, nullptr);
        _networkObserver->deleteLater();
        _networkObserver = nullptr;
    }
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

void DMC_ServerConnection::joinSession()
{
    if((!_networkManager) || (!_options.getLogon().isValid()) || (_options.getCurrentInvite().isEmpty()))
        return;

    qDebug() << "[DMC_ServerConnection] Sending the join session request.";
    _networkManager->joinSession(_options.getCurrentInvite());
}
