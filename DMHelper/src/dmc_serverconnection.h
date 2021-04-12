#ifndef DMC_SERVERCONNECTION_H
#define DMC_SERVERCONNECTION_H

#include <QObject>
#include <QPixmap>
#include "dmhlogon.h"

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
//    DMC_ServerConnection(const QString& cacheDirectory, QObject *parent = 0);
    DMC_ServerConnection(DMC_OptionsContainer& options, QObject *parent = 0);
//    DMC_ServerConnection(const QString& urlString, const QString& username, const QString& password, const QString& session, const QString& cacheDirectory, QObject *parent = 0);
    virtual ~DMC_ServerConnection();

signals:
    void pixmapActive(QPixmap pixmap);
    void imageActive(QImage pixmap);
    void trackActive(AudioTrack* track);

    void fileRequestStarted(int requestId, const QString& fileMD5);
    void fileRequestCompleted(int requestId, const QString& fileMD5, const QByteArray& data);

public slots:
    void downloadComplete(int requestID, const QString& fileMD5, const QByteArray& data);
    void payloadReceived(const DMHPayload& payload, const QString& timestamp);

    void checkLogon();
    void startServer();
    void stopServer();

    void fileRequested(const QString& md5String);
    void fileAborted(int requestID);
    void targetResized(const QSize& newSize);

private:
    void connectRemotePlayers();

    DMC_OptionsContainer& _options;
    DMHNetworkManager* _networkManager;
    DMHNetworkObserver* _networkObserver;
    RemoteAudioPlayer* _audioPlayer;
    RemoteRenderer* _renderer;
    QPixmap _pmp;
    QString _lastPayload;

};

#endif // DMC_SERVERCONNECTION_H
