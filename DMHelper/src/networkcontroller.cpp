#include "networkcontroller.h"

#ifdef INCLUDE_NETWORK_SUPPORT

#include "audiotrack.h"
#include "dmhnetworkmanager.h"
#include "dmhpayload.h"
#include "dmhlogon.h"
#include "map.h"
#include "encountertext.h"
#include "encounterbattle.h"
#include "battledialogmodel.h"
#include "battledialogmodelmonsterclass.h"
#include "monsterclass.h"
#include "undobase.h"
#include <QFile>
#include <QFileInfo>
#include <QBuffer>
#include <QMessageBox>
#include <QDomDocument>
#include <QImageReader>
#include <QPushButton>
#include <QNetworkReply>
#include <QUndoStack>
#include <QDir>
#include <QDebug>

// Uncomment this define to output payload data to a local file "_dmhpayload.txt"
// #define DMH_NETWORK_CONTROLLER_LOCAL_PAYLOAD

NetworkController::NetworkController(QObject *parent) :
    QObject(parent),
    _networkManager(new DMHNetworkManager(DMHLogon(), this)),
    _payload(),
    _currentObject(nullptr),
    _tracks(),
    _backgroundUpload(nullptr),
    _backgroundCacheKey(0),
    _fowUpload(nullptr),
    _backgroundColor(),
    _dependencies(),
    _uploadedFiles(),
    _enabled(false)
{
    connect(_networkManager, SIGNAL(uploadComplete(int, const QString&, const QString&)), this, SLOT(uploadCompleted(int, const QString&, const QString&)));
    connect(_networkManager, SIGNAL(existsComplete(int, const QString&, const QString&, const QString&, bool)), this, SLOT(existsCompleted(int, const QString&, const QString&, const QString&, bool)));
    connect(_networkManager, SIGNAL(requestError(int)), this, SLOT(uploadError(int)));
    connect(_networkManager, SIGNAL(requestError(int)), this, SIGNAL(networkError(int)));
    connect(_networkManager, SIGNAL(otherRequestComplete()), this, SIGNAL(networkSuccess()));
    connect(_networkManager, &DMHNetworkManager::uploadComplete, this, &NetworkController::uploadComplete);
    connect(_networkManager, &DMHNetworkManager::downloadStarted, this, &NetworkController::downloadStarted);
    connect(_networkManager, &DMHNetworkManager::downloadComplete, this, &NetworkController::downloadComplete);
    connect(_networkManager, &DMHNetworkManager::userInfoComplete, this, &NetworkController::userInfoCompleted);
}

NetworkController::~NetworkController()
{
    disconnect(_networkManager, nullptr, this, nullptr);

    delete _backgroundUpload;
    delete _fowUpload;
    qDeleteAll(_tracks);
    qDeleteAll(_dependencies);
}

void NetworkController::addTrack(AudioTrack* track)
{
    if((!track) || (containsTrack(track)))
        return;

    qDebug() << "[NetworkController] Adding audio track: " << track << ", ID: " << track->getID();
    connect(track, &AudioTrack::campaignObjectDestroyed, this, &NetworkController::removeTrackUUID);
    connect(track, &AudioTrack::muteChanged, this, &NetworkController::updateAudioPayload);
    connect(track, &AudioTrack::repeatChanged, this, &NetworkController::updateAudioPayload);

    UploadObject* trackUpload = new UploadObject(track, track->getFileName(), UploadObject::Status_Complete);
    if(track->getAudioType() == DMHelper::AudioType_File)
    {
        trackUpload->setStatus(UploadObject::Status_NotStarted);
        _tracks.append(trackUpload);
        startObjectUpload(trackUpload);
    }
    else
    {
        _tracks.append(trackUpload);
        updateAudioPayload();
    }
}

void NetworkController::removeTrack(AudioTrack* track)
{
    if(!track)
        return;

    CampaignObjectBase* trackObject = dynamic_cast<CampaignObjectBase*>(track);
    for(int i = 0; i < _tracks.count(); ++i)
    {
        if((_tracks.at(i)) && (_tracks.at(i)->getObject() == trackObject))
        {
            removeTrackIndex(i);
            return;
        }
    }

    updateAudioPayload();
}

void NetworkController::setBackgroundColor(QColor color)
{
    if(!_enabled)
        return;

    if(color.name() != _backgroundColor)
    {
        _backgroundColor = color.name();
        //updateImagePayload();
    }
}

void NetworkController::uploadImage(QImage background)
{
    uploadImage(background, QColor(_backgroundColor));
}

void NetworkController::uploadImage(QImage background, QColor color)
{
    if(!_enabled)
        return;

    bool changed = false;

    qDebug() << "[NetworkController] Uploading image: " << background << ", color: " << color;

    if((!background.isNull()) && (background.cacheKey() != _backgroundCacheKey))
    {
        changed = true;
        if(_backgroundUpload)
        {
            if(_backgroundUpload->getStatus() > 0)
                _networkManager->abortRequest(_backgroundUpload->getStatus());

            delete _backgroundUpload;
            _backgroundUpload = nullptr;
        }

        _backgroundCacheKey = background.cacheKey();
        _backgroundUpload = uploadImage(background, QString("Published Image"));
        startObjectUpload(_backgroundUpload);
    }

    if(_fowUpload)
    {
        if(_fowUpload->getStatus() > 0)
            _networkManager->abortRequest(_fowUpload->getStatus());

        delete _fowUpload;
        _fowUpload = nullptr;
        changed = true;
    }

    if(color.name() != _backgroundColor)
    {
        changed = true;
        _backgroundColor = color.name();
    }

    if(changed)
    {
        _payload.setData(QString());
        updateImagePayload();
    }
}

void NetworkController::uploadObject(CampaignObjectBase* baseObject)
{
    if(!baseObject)
        return;

    if(baseObject != _currentObject)
        cancelUpload();

    _currentObject = baseObject;
    connect(baseObject, &QObject::destroyed, this, &NetworkController::cancelUpload);
    attemptCampaignObjectUpload();
}

void NetworkController::cancelUpload()
{
    if(!_enabled)
        return;

    if(_currentObject)
    {
        disconnect(_currentObject, nullptr, this, nullptr);
        _currentObject = nullptr;
    }

    if(_backgroundUpload)
    {
        if(_backgroundUpload->getStatus() > 0)
            _networkManager->abortRequest(_backgroundUpload->getStatus());

        delete _backgroundUpload;
        _backgroundUpload = nullptr;
    }

    if(_fowUpload)
    {
        if(_fowUpload->getStatus() > 0)
            _networkManager->abortRequest(_fowUpload->getStatus());

        delete _fowUpload;
        _fowUpload = nullptr;
    }

    while(_dependencies.count() > 0)
    {
        if((_dependencies.at(0)) && (_dependencies.at(0)->getStatus() > 0))
            _networkManager->abortRequest(_dependencies.at(0)->getStatus());

        delete _dependencies.takeAt(0);
    }

    //_payload.setImageFile(QString());
    //uploadPayload();
}

void NetworkController::setPayloadData(const QString& data)
{
    if(!_enabled)
        return;

    _payload.setData(data);
    uploadPayload();

    //_currentPayloadRequest = _networkManager->uploadData(_payload.toUtf8());
}

void NetworkController::clearTracks()
{
    if(!_enabled)
        return;

    while(_tracks.count() > 0)
        removeTrackIndex(0);
    //_tracks.clear();

    updateAudioPayload();

    // uploadPayload();
}

void NetworkController::clearPayloadData()
{
    if(!_enabled)
        return;

    _payload.setData(QString());
    uploadPayload();
}

void NetworkController::enableNetworkController(bool enabled)
{
    if(enabled == _enabled)
        return;

    _enabled = enabled;
    if(_enabled)
    {
        DMHLogon logon = _networkManager->getLogon();
        if(!validateLogon(logon))
            return;

        if(logon.getUserId().isEmpty())
            _networkManager->getUserInfo();
        else
            userInfoCompleted(-1, logon.getUserName(), logon.getUserId(), QString(), QString(), QString(), false);
    }

    emit networkEnabledChanged(_enabled);
}

void NetworkController::setNetworkLogin(const QString& urlString, const QString& username, const QString& userId, const QString& password, const QString& sessionID, const QString& inviteID)
{
    Q_UNUSED(inviteID);

    qDebug() << "[NetworkController] Network login updated. URL: " << urlString << ", Username: " << username << ", User ID: " << userId << ", Session: " << sessionID << ", Invite: " << inviteID;

    DMHLogon logon(urlString, username, userId, password, sessionID);
    if(!validateLogon(logon))
        return;

    _networkManager->setLogon(logon);

    if(logon.getUserId().isEmpty())
        _networkManager->getUserInfo();
    else
        userInfoCompleted(-1, logon.getUserName(), logon.getUserId(), QString(), QString(), QString(), false);

    /*
    clearUploadErrors();
    //updateAudioPayload();
    */
}

void NetworkController::userInfoCompleted(int requestID, const QString& username, const QString& userId, const QString& email, const QString& surname, const QString& forename, bool disabled)
{
    DMHLogon logon = _networkManager->getLogon();
    if(username != logon.getUserName())
        return;

    qDebug() << "[NetworkController] User Info query answered with request " << requestID << ". User: " << username << ", User ID: " << userId << ", email: " << email << ", surname: " << surname << ", forename: " << forename << ", disabled: " << disabled;

    logon.setUserId(userId);
    _networkManager->setLogon(logon);

    clearUploadErrors();
    updateAudioPayload();
}

void NetworkController::uploadCompleted(int requestID, const QString& fileMD5, const QString& fileUuid)
{
    qDebug() << "[NetworkController] Upload complete " << requestID << ": " << fileMD5;

    registerUpload(fileMD5, fileUuid);

    if((_backgroundUpload) && (_backgroundUpload->getStatus() == requestID))
    {
        if(fileMD5.isEmpty())
        {
            qDebug() << "[NetworkController] Upload complete for background image with invalid MD5 value, no payload uploaded.";
            _backgroundUpload->setStatus(UploadObject::Status_Error);
        }
        else
        {
            qDebug() << "[NetworkController] Upload complete for background image: " << fileMD5 << ", UUID: " << fileUuid;
            _backgroundUpload->setMD5(fileMD5);
            _backgroundUpload->setUuid(fileUuid);
            _backgroundUpload->setStatus(UploadObject::Status_Complete);
            attemptCampaignObjectUpload();
            //uploadPayload();
        }

        return;
    }

    if((_fowUpload) && (_fowUpload->getStatus() == requestID))
    {
        if(fileMD5.isEmpty())
        {
            qDebug() << "[NetworkController] Upload complete for fow with invalid MD5 value, no payload uploaded.";
            _fowUpload->setStatus(UploadObject::Status_Error);
        }
        else
        {
            qDebug() << "[NetworkController] Upload complete for fow: " << fileMD5 << ", UUID: " << fileUuid;
            _fowUpload->setMD5(fileMD5);
            _fowUpload->setUuid(fileUuid);
            _fowUpload->setStatus(UploadObject::Status_Complete);
            attemptCampaignObjectUpload();
            //uploadPayload();
        }

        return;
    }

    for(int i = 0; i < _tracks.count(); ++i)
    {
        if((_tracks.at(i)) && (_tracks.at(i)->getStatus() == requestID))
        {
            if(fileMD5.isEmpty())
            {
                _tracks.at(i)->setStatus(UploadObject::Status_Error);
                qDebug() << "[NetworkController] ERROR: Upload complete for Audio with invalid MD5 value: " << requestID;
            }
            else
            {
                _tracks.at(i)->setStatus(UploadObject::Status_Complete);
                _tracks.at(i)->setMD5(fileMD5);
                _tracks.at(i)->setUuid(fileUuid);

                qDebug() << "[NetworkController] Upload complete for track: " << _tracks.at(i)->getObject() << ", MD5: " << fileMD5 << ", Uuid: " << fileUuid;
                updateAudioPayload();
                //uploadPayload();
            }
            return;
        }
    }

    for(int i = 0; i < _dependencies.count(); ++i)
    {
        if((_dependencies.at(i)) && (_dependencies.at(i)->getStatus() == requestID))
        {
            if(fileMD5.isEmpty())
            {
                _dependencies.at(i)->setStatus(UploadObject::Status_Error);
                qDebug() << "[NetworkController] ERROR: Upload complete for dependency with invalid MD5 value: " << requestID;
            }
            else
            {
                _dependencies.at(i)->setStatus(UploadObject::Status_Complete);
                _dependencies.at(i)->setMD5(fileMD5);
                _dependencies.at(i)->setUuid(fileUuid);

                qDebug() << "[NetworkController] Upload complete for dependency: " << _dependencies.at(i)->getObject() << ", MD5: " << fileMD5 << ", Uuid: " << fileUuid;
                attemptCampaignObjectUpload();
            }
            return;
        }
    }

    qDebug() << "[NetworkController] ERROR: Unexpected request ID received: " << requestID;
}

/*
UploadObject uploadObject = _tracks.at(i);
if(exists)
{
    qDebug() << "[NetworkController] Audio track already exists, updating payload directly.";
    uploadObject.setStatus(UploadObject::Status_Complete);
    if(uploadObject.getObject())
    {
        uploadObject.getObject()->setMD5(fileMD5);
    }
    _tracks[i] = uploadObject;
    registerUpload(fileMD5, fileUuid);
    updateAudioPayload();
}
else
{
    qDebug() << "[NetworkController] Audio track does not exist, beginning upload.";
    uploadObject.setStatus(UploadObject::Status_NotStarted);
    if(uploadObject.getObject())
    {
        uploadObject.getObject()->setMD5(QString());
        startObjectUpload(&uploadObject);
    }
    _tracks[i] = uploadObject;
}
return;
*/

void NetworkController::existsCompleted(int requestID, const QString& fileMD5, const QString& fileUuid, const QString& filename, bool exists)
{
    qDebug() << "[NetworkController] Exists complete " << requestID << ": " << fileMD5 << ", UUID: " << fileUuid << ", " << filename;

    if(handleExistsResult(_backgroundUpload, requestID, fileMD5, fileUuid, filename, exists))
    {
        if(exists)
            attemptCampaignObjectUpload();
        return;
    }

    if(handleExistsResult(_fowUpload, requestID, fileMD5, fileUuid, filename, exists))
    {
        if(exists)
            attemptCampaignObjectUpload();
        return;
    }

    for(int i = 0; i < _tracks.count(); ++i)
    {
        if(handleExistsResult(_tracks.at(i), requestID, fileMD5, fileUuid, filename, exists))
        {
            if(exists)
                updateAudioPayload();
            return;
        }
    }

    for(int i = 0; i < _dependencies.count(); ++i)
    {
        if(handleExistsResult(_dependencies.at(i), requestID, fileMD5, fileUuid, filename, exists))
        {
            if(exists)
                attemptCampaignObjectUpload();
            return;
        }
    }

    qDebug() << "[NetworkController] ERROR: Unexpected file exists request ID received!";
}

bool NetworkController::handleExistsResult(UploadObject* uploadObject, int requestID, const QString& fileMD5, const QString& fileUuid, const QString& filename, bool exists)
{
    if((!uploadObject) || (uploadObject->getStatus() != requestID))
        return false;

    uploadObject->setStatus(exists ? UploadObject::Status_Complete : UploadObject::Status_NotStarted);
    if(exists)
    {
        qDebug() << "[NetworkController] Upload object already exists, no upload needed. MD5: " << fileMD5 << ", UUID: " << fileUuid << ", Name: " << filename;
        uploadObject->setMD5(fileMD5);
        uploadObject->setUuid(fileUuid);
        registerUpload(fileMD5, fileUuid);
    }
    else
    {
        qDebug() << "[NetworkController] File/data does not exist, beginning upload. MD5: " << fileMD5 << ", UUID: " << fileUuid << ", Name: " << filename;
        uploadObject->setMD5(QString());
        uploadObject->setUuid(QString());
        startObjectUpload(uploadObject);
    }

    return true;
}

void NetworkController::uploadError(int requestID)
{
    if((_backgroundUpload) && (_backgroundUpload->getStatus() == requestID))
    {
        qDebug() << "[NetworkController] Upload error discovered for background request " << requestID;
        _backgroundUpload->setStatus(UploadObject::Status_Error);
        return;
    }

    if((_fowUpload) && (_fowUpload->getStatus() == requestID))
    {
        qDebug() << "[NetworkController] Upload error discovered for FoW request " << requestID;
        _fowUpload->setStatus(UploadObject::Status_Error);
        return;
    }

    for(int i = 0; i < _tracks.count(); ++i)
    {
        if((_tracks.at(i)) && (_tracks.at(i)->getStatus() == requestID))
        {
            qDebug() << "[NetworkController] Upload audio error discovered for request " << requestID;
            _tracks.at(i)->setStatus(UploadObject::Status_Error);
            return;
        }
    }

    for(int i = 0; i < _dependencies.count(); ++i)
    {
        if((_dependencies.at(i)) && (_dependencies.at(i)->getStatus() == requestID))
        {
            qDebug() << "[NetworkController] Upload dependency error discovered for request " << requestID;
            _dependencies.at(i)->setStatus(UploadObject::Status_Error);
            return;
        }
    }

    qDebug() << "[NetworkController] ERROR: Unexpected request ID received in upload error: " << requestID;
}

void NetworkController::uploadPayload()
{
    if(!_enabled)
        return;

#ifdef DMH_NETWORK_CONTROLLER_LOCAL_PAYLOAD
    QFile localPayload("_dmhpayload.txt");
    localPayload.open(QIODevice::WriteOnly);
    localPayload.write(_payload.toString().toUtf8());
    localPayload.close();

    QFileInfo fi(localPayload);
    qDebug() <<"[NetworkController] DEBUG payload output written to location: " << fi.absoluteFilePath();
#endif

#ifdef QT_DEBUG
    qDebug() << "[NetworkController] Uploading payload: " << _payload.toString().toUtf8();
#else
    qDebug() << "[NetworkController] Uploading payload: " << _payload.toString().left(100).toUtf8();
#endif
    _networkManager->uploadPayload(_payload);
}

void NetworkController::removeTrackIndex(int index)
{
    if((index < 0) || (index >= _tracks.count()))
        return;

    if(_tracks.at(index))
    {
        AudioTrack* track = dynamic_cast<AudioTrack*>(_tracks.at(index)->getObject());
        if(track)
        {
            disconnect(track, &AudioTrack::campaignObjectDestroyed, this, &NetworkController::removeTrackUUID);
            disconnect(track, &AudioTrack::muteChanged, this, &NetworkController::updateAudioPayload);
            disconnect(track, &AudioTrack::repeatChanged, this, &NetworkController::updateAudioPayload);
        }

        if(_tracks.at(index)->getStatus() > 0)
            _networkManager->abortRequest(_tracks.at(index)->getStatus());

        delete _tracks.takeAt(index);
    }
    else
    {
        _tracks.removeAt(index);
    }
}

void NetworkController::removeTrackUUID(const QUuid& id)
{
    for(int i = 0; i < _tracks.count(); ++i)
    {
        if((_tracks.at(i)) && (_tracks.at(i)->getObject()) && (_tracks.at(i)->getObject()->getID() == id))
        {
            AudioTrack* track = dynamic_cast<AudioTrack*>(_tracks.at(i)->getObject());
            removeTrack(track);
            return;
        }
    }
}

void NetworkController::attemptCampaignObjectUpload()
{
    bool changed = false;

    if(!_currentObject)
    {
        changed = true;
    }
    else
    {
        if(!_backgroundUpload)
        {
            _backgroundUpload = new UploadObject(_currentObject);
            _backgroundCacheKey = 0;
            changed = true;
        }

        if(_currentObject->getObjectType() == DMHelper::CampaignType_Map)
        {
            // TODO : fix this to iterate
            Map* map = dynamic_cast<Map*>(_currentObject);
            if(map)
            {
                _backgroundUpload->setFileType(map->isInitialized() ? DMHelper::FileType_Image : DMHelper::FileType_Video);
                changed = uploadMap(map) ? true : changed;
            }
        }
        else if(_currentObject->getObjectType() == DMHelper::CampaignType_Text)
        {
            EncounterText* encounter = dynamic_cast<EncounterText*>(_currentObject);
            if(encounter)
            {
                if(_backgroundUpload->getFileType() == DMHelper::FileType_Other)
                {
                    QImageReader reader(encounter->getFileName());
                    _backgroundUpload->setFileType(reader.canRead() ? DMHelper::FileType_Image : DMHelper::FileType_Video);
                }
                changed = uploadEncounterText(encounter) ? true : changed;
            }
        }
        else if(_currentObject->getObjectType() == DMHelper::CampaignType_Battle)
        {
            // TODO : fix this to iterate
            EncounterBattle* encounter = dynamic_cast<EncounterBattle*>(_currentObject);
            if(encounter)
            {
                if(!encounter->getFileName().isEmpty())
                {
                    QImageReader reader(encounter->getFileName());
                    _backgroundUpload->setFileType(reader.canRead() ? DMHelper::FileType_Image : DMHelper::FileType_Video);

                    if(encounter->getBattleDialogModel()->getMap())
                        changed = uploadMap(encounter->getBattleDialogModel()->getMap()) ? true : changed;
                }
                changed = uploadBattle(encounter) ? true : changed;
            }
        }
        else
        {
            qDebug() << "[NetworkController] ERROR: Unexpected object type for upload!";
        }
    }

    if(changed)
        updateImagePayload();
}

void NetworkController::startObjectUpload(UploadObject* uploadObject)
{
    if((!_enabled) || (!uploadObject))
    {
        qDebug() << "[NetworkController] Uploading object failed! enabled: " << _enabled << ", uploadObject: " << uploadObject;
        return;
    }

    // If the object to be uploaded has a specific file, make sure that file exists
    if(uploadObject->getObject())
    {
        uploadObject->setFilename(uploadObject->getObject()->getFileName());
        if((!uploadObject->getFilename().isEmpty()) && (!QFile::exists(uploadObject->getFilename())))
        {
            qDebug() << "[NetworkController] Uploading local file failed! File not found: " << uploadObject->getFilename();
            uploadObject->setStatus(UploadObject::Status_Error);
            return;
        }
    }

    // Prepare the description for the upload panel
    QString uploadName = uploadObject->getDescription();
    if((!uploadObject->getDescription().isEmpty()) && (uploadObject->getObject() != nullptr))
        uploadName += QString(": ");
    if(uploadObject->getObject())
        uploadName += uploadObject->getObject()->getName();


    /*
    md5 empty --> upload
    if id empty --> getexists(md5)
                if not exists --> upload
    if both there, done --
    */


    // Check if this file has been uploaded previously
    if(uploadObject->hasUuid())
    {
        if(isAlreadyUploaded(uploadObject->getMD5()))
        {
            qDebug() << "[NetworkController] Object known and already uploaded: " << uploadName << ", MD5: " << uploadObject->getMD5() << ", Request: " << uploadObject->getStatus();
            uploadObject->setStatus(UploadObject::Status_Exists);
            existsCompleted(UploadObject::Status_Exists, uploadObject->getMD5(), uploadObject->getUuid(), QString(), true);
        }
        else
        {
            uploadObject->setStatus(_networkManager->fileExists(uploadObject->getMD5(), uploadObject->getUuid()));
            qDebug() << "[NetworkController] Checking if object exists: " << uploadName << ", MD5: " << uploadObject->getMD5() << ", UUID: " << uploadObject->getUuid() << ", Request: " << uploadObject->getStatus();
        }
    }
    else
    {
        if(uploadObject->hasMD5())
        {
            uploadObject->setStatus(_networkManager->fileExists(uploadObject->getMD5(), uploadObject->getUuid()));
            qDebug() << "[NetworkController] Checking if object exists: " << uploadName << ", MD5: " << uploadObject->getMD5() << ", UUID: " << uploadObject->getUuid() << ", Request: " << uploadObject->getStatus();
        }
        else
        {
            if(uploadObject->getFilename().isEmpty())
                uploadObject->setStatus(_networkManager->uploadData(uploadObject->getData()));
            else
                uploadObject->setStatus(_networkManager->uploadFile(uploadObject->getFilename()));

            if(uploadObject->getStatus() > 0)
            {
                qDebug() << "[NetworkController] Uploading object: " << uploadName << ", MD5: " << uploadObject->getMD5() << ", Request: " << uploadObject->getStatus();
                emit uploadStarted(uploadObject->getStatus(), _networkManager->getNetworkReply(uploadObject->getStatus()), uploadName);
            }
        }
    }
}

bool NetworkController::containsTrack(AudioTrack* track)
{
    if(!track)
        return false;

    CampaignObjectBase* baseObject = dynamic_cast<CampaignObjectBase*>(track);
    for(int i = 0; i < _tracks.count(); ++i)
    {
        if((_tracks.at(i)) && (_tracks.at(i)->getObject() == baseObject))
            return true;
    }

    return false;
}

void NetworkController::updateAudioPayload()
{
    if(!_enabled)
        return;

    QDomDocument doc;

    for(int i = 0; i < _tracks.count(); ++i)
    {
        if((_tracks.at(i)) && (_tracks.at(i)->getObject()))
        {
            if(_tracks.at(i)->getStatus() == UploadObject::Status_Complete)
            {
                QDomElement trackElement = _tracks.at(i)->getObject()->outputNetworkXML(doc);
                doc.appendChild(trackElement);
            }
            else if(_tracks.at(i)->getStatus() == UploadObject::Status_NotStarted)
            {
                startObjectUpload(_tracks.at(i));
            }
        }
    }

    QString audioPayload = doc.toString();
    audioPayload.remove(QString("\n"));
    qDebug() << "[NetworkController] Audio payload set to: " << audioPayload;
    _payload.setAudioFile(audioPayload);

    uploadPayload();
}

void NetworkController::clearUploadErrors()
{
    if(!_enabled)
        return;

    if((_backgroundUpload) && (_backgroundUpload->getStatus() == UploadObject::Status_Error))
        _backgroundUpload->setStatus(UploadObject::Status_NotStarted);

    if((_fowUpload) && (_fowUpload->getStatus() == UploadObject::Status_Error))
        _fowUpload->setStatus(UploadObject::Status_NotStarted);

    for(int i = 0; i < _tracks.count(); ++i)
    {
        if((_tracks.at(i)) && (_tracks.at(i)->getStatus() == UploadObject::Status_Error))
            _tracks.at(i)->setStatus(UploadObject::Status_NotStarted);
    }

    for(int i = 0; i < _dependencies.count(); ++i)
    {
        if((_dependencies.at(i)) && (_dependencies.at(i)->getStatus() == UploadObject::Status_Error))
            _dependencies.at(i)->setStatus(UploadObject::Status_NotStarted);
    }
}

UploadObject* NetworkController::uploadImage(QImage image, const QString& imageName)
{
    if((!_enabled) || (image.isNull()))
        return nullptr;

    QByteArray data;
    QBuffer buffer(&data);
    if(!buffer.open(QIODevice::WriteOnly))
        return nullptr;

    UploadObject* result = nullptr;
    if(image.save(&buffer, "PNG"))
    {
        result = new UploadObject(nullptr, UploadObject::Status_Error);
        result->setMD5(getDataMD5(data));
        result->setDescription(imageName);
        result->setFileType(DMHelper::FileType_Image);
        result->setStatus(UploadObject::Status_NotStarted);
        result->setData(data);
        qDebug() << "[NetworkController] Uploading image " << image << " with name: " << imageName << "data with MD5 hash HEX: " << result->getMD5();
    }

    buffer.close();
    return result;
}

UploadObject* NetworkController::uploadPixmap(QPixmap pixmap, const QString& pixmapName)
{
    if((!_enabled) || (pixmap.isNull()))
        return nullptr;

    QByteArray data;
    QBuffer buffer(&data);
    if(!buffer.open(QIODevice::WriteOnly))
        return nullptr;

    UploadObject* result = nullptr;
    if(pixmap.save(&buffer, "PNG"))
    {
        result = new UploadObject(nullptr, UploadObject::Status_Error);
        result->setMD5(getDataMD5(data));
        result->setDescription(pixmapName);
        result->setFileType(DMHelper::FileType_Image);
        result->setStatus(UploadObject::Status_NotStarted);
        result->setData(data);
        qDebug() << "[NetworkController] Uploading pixmap " << pixmap << " with name: " << pixmapName << "data with MD5 hash HEX: " << result->getMD5();
    }

    buffer.close();
    return result;
}

void NetworkController::updateImagePayload()
{
    if(!_enabled)
        return;

    // TODO: dependency upload before sending the payload
    QString imagePayload;
    bool payloadReady = true;

    if(_backgroundUpload)
    {
        if(_backgroundUpload->getStatus() < UploadObject::Status_Complete)
        {
            startObjectUpload(_backgroundUpload);
            payloadReady = false;
        }
        else if(_backgroundUpload->hasMD5())
        {
            if(_backgroundUpload->getFileType() == DMHelper::FileType_Other)
                imagePayload += QString("<background>");
            else
                imagePayload += QString("<background ") + QString("type=""") + QString::number(_backgroundUpload->getFileType()) + QString(""">");
            imagePayload += _backgroundUpload->getDescriptor() + QString("</background>");

            if((_fowUpload) && (_fowUpload->isValid()))
            {
                if(_fowUpload->getStatus() < UploadObject::Status_Complete)
                {
                    startObjectUpload(_fowUpload);
                    payloadReady = false;
                }
                else
                {
                    imagePayload += QString("<fow>") + _fowUpload->getDescriptor() + QString("</fow>");
                }
            }
        }
    }

    for(int i = 0; i < _dependencies.count(); ++i)
    {
        if((_dependencies.at(i)) && (_dependencies.at(i)->getStatus() < UploadObject::Status_Complete))
        {
            startObjectUpload(_dependencies.at(i));
            payloadReady = false;
        }
    }

    if(payloadReady)
    {
        qDebug() << "[NetworkController] Image payload set to: " << imagePayload;
        _payload.setImageFile(imagePayload);
        uploadPayload();
    }
}

bool NetworkController::uploadMap(Map* map)
{
    if(!map)
        return false;

    QUndoStack* undoStack = map->getUndoStack();
    if(!undoStack)
        return false;

    QDomDocument doc;
    QDomElement fowElement = doc.createElement("fow");
    doc.appendChild(fowElement);
    QDir emptyDir;
    for(int i = 0; i < undoStack->index(); ++i)
    {
        const UndoBase* action = dynamic_cast<const UndoBase*>(undoStack->command(i));
        if(action)
        {
            QDomElement actionElement = doc.createElement("action");
            actionElement.setAttribute("type", action->getType());
            action->outputXML(doc, actionElement, emptyDir, true);
            fowElement.appendChild(actionElement);
        }
    }

    if(_fowUpload)
    {
        if(_fowUpload->getStatus() > 0)
            _networkManager->abortRequest(_fowUpload->getStatus());
        delete _fowUpload;
        _fowUpload = nullptr;
    }

    _fowUpload = new UploadObject();
    _fowUpload->setData(doc.toString().toUtf8());
    _fowUpload->setFileType(DMHelper::FileType_Text);
    _fowUpload->setDescription(QString("Fog of War: ") + map->getName());
    return true;
}

bool NetworkController::uploadEncounterText(EncounterText* encounterText)
{
    if(!encounterText)
        return false;

    QByteArray textHash;
    QString textMD5;
    QByteArray translatedHash;
    QString translatedMD5;
    UploadObject* textUpload = nullptr;
    UploadObject* translateUpload = nullptr;

    if(!encounterText->getText().isEmpty())
    {
        textHash = getDataMD5(encounterText->getText().toUtf8());
        textMD5 = QString::fromUtf8(textHash);
        if(!_uploadedFiles.contains(textMD5))
        {
            textUpload = new UploadObject();
            textUpload->setData(encounterText->getText().toUtf8());
            textUpload->setDescription(QString("Text: ") + encounterText->getName());
            textUpload->setFileType(DMHelper::FileType_Text);
            textUpload->setMD5(textMD5);
            _dependencies.append(textUpload);
        }
    }

    if((encounterText->getTranslated()) && (!encounterText->getTranslatedText().isEmpty()))
    {
        translatedHash = getDataMD5(encounterText->getTranslatedText().toUtf8());
        translatedMD5 = QString::fromUtf8(translatedHash);
        if(!_uploadedFiles.contains(translatedMD5))
        {
            translateUpload = new UploadObject();
            translateUpload->setData(encounterText->getTranslatedText().toUtf8());
            translateUpload->setDescription(QString("Translated Text: ") + encounterText->getName());
            translateUpload->setFileType(DMHelper::FileType_Text);
            translateUpload->setMD5(translatedMD5);
            _dependencies.append(translateUpload);
        }
    }

    if((textUpload) || (translateUpload))
    {
        qDebug() << "[NetworkController] Prepared encounter text, dependencies needed. Text: " << (textUpload ? textUpload->getMD5() : QString()) << ", Translated: " << (translateUpload ? translateUpload->getMD5() : QString());
        return true;
    }

    QDomDocument doc;
    QDomElement textElement = encounterText->outputNetworkXML(doc);
    if(textElement.isNull())
    {
        qDebug() << "[NetworkController] ERROR: Unable to prepare encounter text for publishing!";
        return false;
    }

    textElement.setAttribute(QString("text"), UploadObject::getDescriptor(textMD5, _uploadedFiles.value(textMD5)));
    textElement.setAttribute(QString("translated-text"), UploadObject::getDescriptor(translatedMD5, _uploadedFiles.value(translatedMD5)));

    doc.appendChild(textElement);

    QString encounterPayload = doc.toString();
    encounterPayload.remove(QString("\n"));
    _payload.setData(encounterPayload);
    qDebug() << "[NetworkController] Prepared encounter text with text hash: " << (textUpload ? textUpload->getMD5() : QString()) << " and translated hash: " << (translateUpload ? translateUpload->getMD5() : QString());

    return true;
}

bool NetworkController::uploadBattle(EncounterBattle* encounterBattle)
{
    if((!encounterBattle) || (!encounterBattle->getBattleDialogModel()))
        return false;

    BattleDialogModel* model = encounterBattle->getBattleDialogModel();

    QDomDocument doc;

    QDomElement battleElement = model->outputNetworkXML(doc);
    if(battleElement.isNull())
        return false;

    QStringList monsters;
    QDomElement combatantsElement = doc.createElement("combatants");
    for(int i = 0; i < model->getCombatantCount(); ++i)
    {
        BattleDialogModelCombatant* combatant = model->getCombatant(i);
        if((combatant) &&
           (combatant->getKnown()) &&
           ((combatant->getHitPoints() > 0) || (model->getShowDead())) &&
           ((combatant->getHitPoints() <= 0) || (model->getShowAlive())))
        {
            QDomElement combatantElement = combatant->outputNetworkXML(doc);

            if(!isAlreadyUploaded(combatant->getMD5()))
            {
                UploadObject* tokenUpload = uploadPixmap(combatant->getIconPixmap(DMHelper::PixmapSize_Full), QString("Token for ") + combatant->getName());
                if(tokenUpload)
                {
                    combatant->setMD5(tokenUpload->getMD5());
                    _dependencies.append(tokenUpload);
                    startObjectUpload(tokenUpload);
                }
            }
            combatantElement.setAttribute("token", combatant->getMD5());

            combatantsElement.appendChild(combatantElement);
        }
    }
    battleElement.appendChild(combatantsElement);

    if(model->getShowEffects())
    {
        QDomElement effectsElement = doc.createElement("effects");
        for(int i = 0; i < model->getEffectCount(); ++i)
        {
            BattleDialogModelEffect* effect = model->getEffect(i);
            if((effect) && (effect->getEffectVisible()))
            {
                QDomElement effectElement = effect->outputNetworkXML(doc);

                if(!effect->getImageFile().isEmpty())
                {
                    if(!isAlreadyUploaded(effect->getMD5()))
                    {
                        if(effect->getMD5().isEmpty())
                            effect->setMD5(getFileMD5(effect->getImageFile()));

                        UploadObject* effectToken = new UploadObject(nullptr, UploadObject::Status_NotStarted);
                        effectToken->setFilename(effect->getImageFile());
                        effectToken->setFileType(DMHelper::FileType_Image);
                        effectToken->setDescription(QString("Effect Token ") + effect->getImageFile());
                        _dependencies.append(effectToken);
                        startObjectUpload(effectToken);
                    }
                    effectElement.setAttribute("token", effect->getMD5());
                }
                effectsElement.appendChild(effectElement);
            }
        }
        battleElement.appendChild(effectsElement);
    }

    QDomElement battleObject = encounterBattle->outputNetworkXML(doc);
    battleObject.appendChild(battleElement);
    doc.appendChild(battleObject);

    QString battlePayload = doc.toString();
    battlePayload.remove(QString("\n"));
    qDebug() << "[NetworkController] Prepared battle for upload: " << battlePayload;
    _payload.setData(battlePayload);

    return true;
}

bool NetworkController::validateLogon(const DMHLogon& logon)
{
    if(logon.isValid())
    {


        return true;
    }

    /*
    QMessageBox msgBox(QMessageBox::Warning,
                       QString("DMHelper Network Connection"),
                       QString("Warning: The network client publishing is enabled, but the network URL, username, password and/or session ID are not properly set. Network publishing will not work unless these values are correct.\n\nWould you like to correct these settings or disable network publishing?"));
    QAbstractButton* openButton = msgBox.addButton(QString("Open Network Settings"), QMessageBox::AcceptRole);
    msgBox.addButton(QString("Disable Network Publish"), QMessageBox::RejectRole);
    */
    QMessageBox msgBox(QMessageBox::Warning,
                       QString("DMHelper Network Connection"),
                       QString("Warning: The network client publishing is enabled, but the network URL, username, password and/or session ID are not properly set. Network publishing will not work unless these values are correct.\n\nWould you like to correct these settings?"));
    QAbstractButton* openButton = msgBox.addButton(QString("Yes"), QMessageBox::AcceptRole);
    msgBox.addButton(QString("No"), QMessageBox::RejectRole);
    msgBox.exec();
    if(msgBox.clickedButton() == openButton)
        emit requestSettings(DMHelper::OptionsTab_Network);

    return false;
}

bool NetworkController::isAlreadyUploaded(const QString& md5)
{
    if(md5.isEmpty())
        return false;

    return _uploadedFiles.contains(md5);
}

void NetworkController::registerUpload(const QString& md5, const QString& uuid)
{
    if((md5.isEmpty()) || (_uploadedFiles.contains(md5)))
        return;

    _uploadedFiles.insert(md5, uuid);
}

QByteArray NetworkController::getFileMD5(const QString& filename)
{
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
        return QByteArray();

    QByteArray readData = file.readAll();
    if(readData.size() <= 0)
        return readData;

    file.close();
    return getDataMD5(readData);
}

QByteArray NetworkController::getDataMD5(const QByteArray& data)
{
    return QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex(0);
}

#endif // INCLUDE_NETWORK_SUPPORT
