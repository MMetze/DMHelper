#ifndef DMC_SERVERCONNECTION_H
#define DMC_SERVERCONNECTION_H

#include <QObject>
#include <QPixmap>
#include "dmhlogon.h"
#include "dmhmessage.h"

class DMHNetworkManager;
class DMHPayload;
class DMHNetworkObserver;
class AudioTrack;
class RemoteAudioPlayer;
class RemoteRenderer;
class DMC_OptionsContainer;

class DMC_ServerConnection : public QObject
{
    Q_OBJECT
public:
    DMC_ServerConnection(DMC_OptionsContainer& options, QObject *parent = 0);
    virtual ~DMC_ServerConnection();

    bool isConnected() const;

signals:
    void connectionChanged(bool connected);
    void networkMessage(const QString& message);

    void pixmapActive(QPixmap pixmap);
    void imageActive(QImage pixmap);
    void trackActive(AudioTrack* track);

    void fileRequestStarted(int requestId, const QString& fileMD5, const QString& fileUuid);
    void fileRequestCompleted(int requestId, const QString& fileMD5, const QString& fileUuid, const QByteArray& data);

public slots:
    void downloadComplete(int requestID, const QString& fileMD5, const QString& fileUuid, const QByteArray& data);
    void payloadReceived(const DMHPayload& payload, const QString& timestamp);

    void connectServer(bool connect);

    void checkLogon();
    void startServer();
    void stopServer();

    void fileRequested(const QString& md5, const QString& uuid);
    void fileAborted(int requestID);
    void targetResized(const QSize& newSize);

    void publishWindowMouseDown(const QPointF& position);
    void publishWindowMouseMove(const QPointF& position);
    void publishWindowMouseRelease(const QPointF& position);

private slots:
    void userInfoCompleted(int requestID, const QString& username, const QString& userId, const QString& email, const QString& surname, const QString& forename, bool disabled);
    void joinSessionComplete(int requestID, const QString& session);
    void handleMessageReceived(const QList<DMHMessage>& messages);

    void messageError(int requestID, const QString& errorString);
    void requestError(int requestID);

    void rendererMessage(const QString& message);

private:
    void startManager();
    void stopManager();
    void startObserver();
    void startObserverPayload();
    void stopObserver();

    void connectRemotePlayers();
    void disconnectRemotePlayers();
    void joinSession();

    bool _connected;
    DMC_OptionsContainer& _options;
    QString _session;
    DMHNetworkManager* _networkManager;
    DMHNetworkObserver* _networkObserver;
    RemoteAudioPlayer* _audioPlayer;
    RemoteRenderer* _renderer;
    QString _lastPayload;

};

#endif // DMC_SERVERCONNECTION_H
