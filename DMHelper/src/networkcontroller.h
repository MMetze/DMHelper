#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#ifdef INCLUDE_NETWORK_SUPPORT

#include <QObject>
#include <QImage>

class AudioTrack;
class DMHNetworkManager;

class NetworkController : public QObject
{
    Q_OBJECT
public:
    explicit NetworkController(QObject *parent = nullptr);
    ~NetworkController();

signals:

public slots:
    void uploadTrack(AudioTrack* track);
    void uploadImage(QImage img, const QColor& color);
    void setPayload(const QString& command, const QString& payload);
    void clearTrack();
    void clearImage();
    void clearPayload();
    void enableNetworkController(bool enabled);
    void setNetworkLogin(const QString& urlString, const QString& username, const QString& password, const QString& sessionID, const QString& inviteID);

private slots:
    void uploadComplete(int requestID, const QString& fileMD5);
    void existsComplete(int requestID, const QString& fileMD5, const QString& filename, bool exists);
    void uploadError(int requestID);
    void uploadPayload();

private:
    DMHNetworkManager* _networkManager;
    QString _imageMD5;
    QString _audioMD5;
    QString _command;
    QString _payload;
    int _currentImageRequest;
    int _currentAudioRequest;
    int _currentPayloadRequest;
    AudioTrack* _currentTrack; // TODO: replace somehow, this isn't safe
    bool _enabled;

};

#endif // INCLUDE_NETWORK_SUPPORT

#endif // NETWORKCONTROLLER_H
