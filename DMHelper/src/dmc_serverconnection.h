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

class DMC_ServerConnection : public QObject
{
    Q_OBJECT
public:
    DMC_ServerConnection(QObject *parent = 0);
    DMC_ServerConnection(const DMHLogon& logon, QObject *parent = 0);
    DMC_ServerConnection(const QString& urlString, const QString& username, const QString& password, const QString& session, QObject *parent = 0);
    virtual ~DMC_ServerConnection();

signals:
    void pixmapActive(QPixmap pixmap);
    void trackActive(AudioTrack* track);

    void fileRequestStarted(int requestId);
    void fileRequestCompleted(int requestId, const QString& fileMD5, const QByteArray& data);

public slots:
    void downloadComplete(int requestID, const QString& fileMD5, const QByteArray& data);
    void payloadReceived(const DMHPayload& payload, const QString& timestamp);

    void startServer(const DMHLogon& logon);
    void stopServer();

    void fileRequested(const QString& md5String);

private:
    //void parseAudioData(const QString& audioData);
    void connectAudioPlayer();
    void loadBattle();
    //void stopAudio();

    DMHNetworkManager* _networkManager;
    DMHNetworkObserver* _networkObserver;
    RemoteAudioPlayer* _audioPlayer;
    QString _imageMD5client;
    //QString _audioMD5client;
    int _currentImageRequest;
    //int _currentAudioRequest;
    //AudioTrack* _track;
    QPixmap _pmp;
    QString _lastPayload;

};

#endif // DMC_SERVERCONNECTION_H
