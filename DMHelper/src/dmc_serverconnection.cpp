#include "dmc_serverconnection.h"
#include "dmc_constants.h"
#include "dmc_optionscontainer.h"
#include "dmc_connectionsettingsdialog.h"
#include "dmhnetworkmanager.h"
#include "dmhnetworkobserver.h"
#include "dmhpayload.h"
#include "dmhlogon.h"
#include "dmhmessage.h"
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

const int DMC_OBSERVER_INTERVAL = 100;

DMC_ServerConnection::DMC_ServerConnection(DMC_OptionsContainer& options, QObject *parent) :
    QObject(parent),
    _connected(false),
    _options(options),
    _session(),
    _networkManager(nullptr),
    _networkObserver(nullptr),
    _audioPlayer(new RemoteAudioPlayer(options.getCacheDirectory(), this)),
    _renderer(new RemoteRenderer(options.getCacheDirectory(), this)),
    _lastPayload()
{
}

DMC_ServerConnection::~DMC_ServerConnection()
{
    stopServer();
}

bool DMC_ServerConnection::isConnected() const
{
    return _connected;
}

void DMC_ServerConnection::downloadComplete(int requestID, const QString& fileMD5, const QString& fileUuid, const QByteArray& data)
{
    qDebug() << "[DMC_ServerConnection] Download complete " << requestID << ": " << fileMD5  << ", " << fileUuid << ", " << data.size() << " bytes";
    emit fileRequestCompleted(requestID, fileMD5, fileUuid, data);
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
        return;

    /*
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
    emit networkMessage(QString("Joining the session with invite: ") + _options.getCurrentInvite());
    */

    startManager();

    if(_options.getUserId().isEmpty())
        _networkManager->getUserInfo();
    else
        userInfoCompleted(-1, _options.getUserName(), _options.getUserId(), QString(), QString(), QString(), false);
}

void DMC_ServerConnection::startServer()
{
    qDebug() << "[DMC_ServerConnection] Starting server with logon: " << _options.getLogon();

    if(_connected)
        stopServer();

    startManager();
    connectServer(true);
}

void DMC_ServerConnection::stopServer()
{
    qDebug() << "[DMC_ServerConnection] Stop server.";

    stopManager();
    stopObserver();

    emit connectionChanged(false);
    _connected = false;
}

void DMC_ServerConnection::fileRequested(const QString& md5, const QString& uuid)
{
    QString cachedFile = _options.getCacheDirectory() + QString("/") + md5;
    if(QFile::exists(cachedFile))
    {
        emit fileRequestStarted(-1, QString(), QString());
        emit fileRequestCompleted(-1, md5, uuid, QByteArray());
    }
    else
    {
        emit fileRequestStarted(_networkManager->downloadFile(md5, uuid), md5, uuid);
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

void DMC_ServerConnection::publishWindowMouseDown(const QPointF& position)
{
    if(_renderer)
        _renderer->publishWindowMouseDown(position);
}

void DMC_ServerConnection::publishWindowMouseMove(const QPointF& position)
{
    if(_renderer)
        _renderer->publishWindowMouseMove(position);
}

void DMC_ServerConnection::publishWindowMouseRelease(const QPointF& position)
{
    if(_renderer)
        _renderer->publishWindowMouseRelease(position);
}

void DMC_ServerConnection::userInfoCompleted(int requestID, const QString& username, const QString& userId, const QString& email, const QString& surname, const QString& forename, bool disabled)
{
    Q_UNUSED(requestID);
    Q_UNUSED(email);
    Q_UNUSED(surname);
    Q_UNUSED(forename);
    Q_UNUSED(disabled);

    if(!_networkManager)
        return;

    if((username != _options.getUserName()) || (userId.isEmpty()) || (disabled))
    {
        if(username == _options.getUserName() && (disabled))
            emit networkMessage(QString("User ") + username + QString(" is disabled!"));
        else
            emit networkMessage(QString("Unexpected error connecting to server with user name ") + username);

        qDebug() << "[DMC_ServerConnection] Invalid user info received. Username: " << username << ", User ID: " << userId << ", disabled: " << disabled;
        stopServer();
    }

    emit networkMessage(QString("Connected to server with user name ") + username);
    _options.setUserId(userId);
    _networkManager->setLogon(_options.getLogon());
    joinSession();
}

void DMC_ServerConnection::joinSessionComplete(int requestID, const QString& session)
{
    Q_UNUSED(requestID);

    if(session.isEmpty())
        return;

    emit networkMessage(QString("Successfully joined the session with ID ") + session);
    qDebug() << "[DMC_ServerConnection] Successfully joined the session. Session ID: " << session;
    _session = session;

    // Just restart the server components
    startManager();
    startObserver();
    //startServer();

    _networkManager->sendMessage(DMHMessage(QString("join"), QString("")));

    // TODO: local debug only
    startObserverPayload();

}

void DMC_ServerConnection::handleMessageReceived(const QList<DMHMessage>& messages)
{
    qDebug() << "[DMC_ServerConnection] Message received: " << messages.count();
    for(int i = 0; i < messages.count(); ++i)
    {
        DMHMessage message(messages.at(i));
        qDebug() << "[DMC_ServerConnection]       Message " << i << ": " << message;
        //if(message.getCommand() == QString("accepted"))
        if(message.getBody() == QString("accept"))
        {
            qDebug() << "[DMC_ServerConnection] DM Helper server accepted participation";
            startObserverPayload();
        }
    }
}

void DMC_ServerConnection::messageError(int requestID, const QString& errorString)
{
    emit networkMessage(QString("An error was received from the server: ") + errorString);
    qDebug() << "[DMC_ServerConnection] Error in message, stopping the server. Request: " << requestID << ", error: " << errorString;
}

void DMC_ServerConnection::requestError(int requestID)
{
    emit networkMessage(QString("There was an unexpected error in communication with the server!"));
    qDebug() << "[DMC_ServerConnection] Error in connection, stopping the server. Request: " << requestID;
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
        connect(_networkManager, &DMHNetworkManager::userInfoComplete, this, &DMC_ServerConnection::userInfoCompleted);
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
        _networkObserver = new DMHNetworkObserver(logon, DMHNetworkObserver::ObserverType_Message, this);
        _networkObserver->setInterval(DMC_OBSERVER_INTERVAL);
        connectRemotePlayers();
        _networkObserver->start();
    }
}

void DMC_ServerConnection::startObserverPayload()
{
    if(_networkObserver)
        _networkObserver->setObserverType(DMHNetworkObserver::ObserverType_All);

    if(!_connected)
    {
        emit connectionChanged(true);
        _connected = true;
    }
}

void DMC_ServerConnection::stopObserver()
{
    if(_networkObserver)
    {
        disconnectRemotePlayers();

        _networkObserver->deleteLater();
        _networkObserver = nullptr;

        _lastPayload.clear();
        _audioPlayer->reset();
        _renderer->reset();
    }
}

void DMC_ServerConnection::connectRemotePlayers()
{
    connect(_networkObserver, SIGNAL(payloadReceived(const DMHPayload&, const QString&)), this, SLOT(payloadReceived(const DMHPayload&, const QString&)));
    connect(_networkObserver, &DMHNetworkObserver::messageReceived, this, &DMC_ServerConnection::handleMessageReceived);

    if(_audioPlayer)
    {
        connect(_audioPlayer, &RemoteAudioPlayer::requestFile, this, &DMC_ServerConnection::fileRequested);
        connect(this, &DMC_ServerConnection::fileRequestStarted, _audioPlayer, &RemoteAudioPlayer::fileRequestStarted);
        connect(this, &DMC_ServerConnection::fileRequestCompleted, _audioPlayer, &RemoteAudioPlayer::fileRequestCompleted);
    }

    if(_renderer)
    {
        connect(_renderer, &RemoteRenderer::requestFile, this, &DMC_ServerConnection::fileRequested);
        connect(_renderer, &RemoteRenderer::abortRequest, this, &DMC_ServerConnection::fileAborted);
        connect(_renderer, &RemoteRenderer::publishPixmap, this, &DMC_ServerConnection::pixmapActive);
        connect(_renderer, &RemoteRenderer::publishImage, this, &DMC_ServerConnection::imageActive);
        connect(this, &DMC_ServerConnection::fileRequestStarted, _renderer, &RemoteRenderer::fileRequestStarted);
        connect(this, &DMC_ServerConnection::fileRequestCompleted, _renderer, &RemoteRenderer::fileRequestCompleted);
    }
}

void DMC_ServerConnection::disconnectRemotePlayers()
{
    disconnect(_networkObserver, SIGNAL(payloadReceived(const DMHPayload&, const QString&)), this, SLOT(payloadReceived(const DMHPayload&, const QString&)));
    disconnect(_networkObserver, &DMHNetworkObserver::messageReceived, this, &DMC_ServerConnection::handleMessageReceived);

    if(_audioPlayer)
    {
        disconnect(_audioPlayer, &RemoteAudioPlayer::requestFile, this, &DMC_ServerConnection::fileRequested);
        disconnect(this, &DMC_ServerConnection::fileRequestStarted, _audioPlayer, &RemoteAudioPlayer::fileRequestStarted);
        disconnect(this, &DMC_ServerConnection::fileRequestCompleted, _audioPlayer, &RemoteAudioPlayer::fileRequestCompleted);
    }

    if(_renderer)
    {
        disconnect(_renderer, &RemoteRenderer::requestFile, this, &DMC_ServerConnection::fileRequested);
        disconnect(_renderer, &RemoteRenderer::abortRequest, this, &DMC_ServerConnection::fileAborted);
        disconnect(_renderer, &RemoteRenderer::publishPixmap, this, &DMC_ServerConnection::pixmapActive);
        disconnect(_renderer, &RemoteRenderer::publishImage, this, &DMC_ServerConnection::imageActive);
        disconnect(this, &DMC_ServerConnection::fileRequestStarted, _renderer, &RemoteRenderer::fileRequestStarted);
        disconnect(this, &DMC_ServerConnection::fileRequestCompleted, _renderer, &RemoteRenderer::fileRequestCompleted);
    }
}

void DMC_ServerConnection::joinSession()
{
    if((!_networkManager) || (!_options.getLogon().isValid()) || (_options.getCurrentInvite().isEmpty()))
        return;

    emit networkMessage(QString("Joining the session with invite ") + _options.getCurrentInvite());
    qDebug() << "[DMC_ServerConnection] Sending the join session request with invite: " << _options.getCurrentInvite();
    _networkManager->joinSession(_options.getCurrentInvite());
    //joinSessionComplete(0, QString("7B3AA550-649A-4D51-920E-CAB465616995"));
}
