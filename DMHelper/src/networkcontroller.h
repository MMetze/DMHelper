#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#include "dmconstants.h"

#ifdef INCLUDE_NETWORK_SUPPORT

#include "dmhpayload.h"
#include "dmhlogon.h"
#include "uploadobject.h"
#include <QObject>
#include <QImage>
#include <QPixmap>
#include <QMap>

class AudioTrack;
class DMHNetworkManager;
class QNetworkReply;
class CampaignObjectBase;
class Map;
class EncounterText;
class EncounterBattle;

class NetworkController : public QObject
{
    Q_OBJECT
public:
    explicit NetworkController(QObject *parent = nullptr);
    ~NetworkController();

signals:
    void requestSettings(DMHelper::OptionsTab startTab);
    void networkEnabledChanged(bool enabled);

    void networkError(int error);
    void networkSuccess();

    void uploadStarted(int requestID, QNetworkReply* reply, const QString& fileName);
    void uploadComplete(int requestID, const QString& fileMD5, const QString& fileUuid);
    void downloadStarted(int requestID, const QString& fileMD5, const QString& fileUuid, QNetworkReply* reply);
    void downloadComplete(int requestID, const QString& fileMD5, const QString& fileUuid, const QByteArray& data);

public slots:
    void addTrack(AudioTrack* track);
    void removeTrack(AudioTrack* track);
    void setBackgroundColor(QColor color);
    void uploadImage(QImage background);
    void uploadImage(QImage background, QColor color);
    void uploadObject(CampaignObjectBase* baseObject);
    void cancelUpload();
    void setPayloadData(const QString& data);
    void clearTracks();
    void clearPayloadData();
    void enableNetworkController(bool enabled);
    void setNetworkLogin(const QString& urlString, const QString& username, const QString& userId, const QString& password, const QString& sessionID, const QString& inviteID);

private slots:
    void userInfoCompleted(int requestID, const QString& username, const QString& userId, const QString& email, const QString& surname, const QString& forename, bool disabled);
    void uploadCompleted(int requestID, const QString& fileMD5, const QString& fileUuid);
    void existsCompleted(int requestID, const QString& fileMD5, const QString& fileUuid, const QString& filename, bool exists);
    bool handleExistsResult(UploadObject* uploadObject, int requestID, const QString& fileMD5, const QString& fileUuid, const QString& filename, bool exists);
    void uploadError(int requestID);
    void uploadPayload();

    void removeTrackIndex(int index);
    void removeTrackUUID(const QUuid& id);
    void attemptCampaignObjectUpload();
    void startObjectUpload(UploadObject* uploadObject);
    bool containsTrack(AudioTrack* track);
    void updateAudioPayload();
    void clearUploadErrors();

    UploadObject* uploadImage(QImage image, const QString& imageName);
    UploadObject* uploadPixmap(QPixmap pixmap, const QString& pixmapName);
    void updateImagePayload();

    bool uploadMap(Map* map);
    bool uploadEncounterText(EncounterText* encounterText);
    bool uploadBattle(EncounterBattle* encounterBattle);

    bool validateLogon(const DMHLogon& logon);

    bool isAlreadyUploaded(const QString& md5);
    void registerUpload(const QString& md5, const QString& uuid);
    QByteArray getFileMD5(const QString& filename);
    QByteArray getDataMD5(const QByteArray& data);

    void cancelObjectUpload();
    void cancelBackgroundUpload();
    void cancelFoWUpload();
    void cancelDependencyUpload();

private:
    DMHNetworkManager* _networkManager;
    DMHPayload _payload;
    CampaignObjectBase* _currentObject;
    QList<UploadObject*> _tracks;
    UploadObject* _backgroundUpload;
    qint64 _backgroundCacheKey; // TODO: check if this has any value!
    UploadObject* _fowUpload;
    QString _backgroundColor;
    QList<UploadObject*> _dependencies;
    QMap<QString, QString> _uploadedFiles;
    bool _enabled;
};

#endif // INCLUDE_NETWORK_SUPPORT

#endif // NETWORKCONTROLLER_H
