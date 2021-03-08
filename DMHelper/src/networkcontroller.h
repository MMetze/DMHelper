#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#include "dmconstants.h"

#ifdef INCLUDE_NETWORK_SUPPORT

#include "dmhpayload.h"
#include "dmhlogon.h"
#include <QObject>
#include <QImage>

class AudioTrack;
class DMHNetworkManager;

typedef QPair<int, AudioTrack*> AudioTrackUpload;

class NetworkController : public QObject
{
    Q_OBJECT
public:
    explicit NetworkController(QObject *parent = nullptr);
    ~NetworkController();

signals:
    void requestSettings(DMHelper::OptionsTab startTab);
    void networkEnabledChanged(bool enabled);

public slots:
    void addTrack(AudioTrack* track);
    void removeTrack(AudioTrack* track);
    void uploadImage(QImage img);
    void uploadImage(QImage img, QColor color);
    void setPayloadData(const QString& data);
    void clearTracks();
    void clearImage();
    void clearPayloadData();
    void enableNetworkController(bool enabled);
    void setNetworkLogin(const QString& urlString, const QString& username, const QString& password, const QString& sessionID, const QString& inviteID);

private slots:
    void uploadComplete(int requestID, const QString& fileMD5);
    void existsComplete(int requestID, const QString& fileMD5, const QString& filename, bool exists);
    void uploadError(int requestID);
    void uploadPayload();

    void removeTrackUUID(const QUuid& id);
    void uploadTrack(AudioTrackUpload* trackPair);
    bool containsTrack(AudioTrack* track);
    void updateAudioPayload();
    void clearUploadErrors();

    bool validateLogon(const DMHLogon& logon);

private:
    DMHNetworkManager* _networkManager;
    DMHPayload _payload;
    int _currentImageRequest;
    QList<AudioTrackUpload> _tracks;
    bool _enabled;
};

#endif // INCLUDE_NETWORK_SUPPORT

#endif // NETWORKCONTROLLER_H
