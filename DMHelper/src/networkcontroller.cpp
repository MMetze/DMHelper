#include "networkcontroller.h"

#ifdef INCLUDE_NETWORK_SUPPORT

#include "audiotrack.h"
#include "dmhnetworkmanager.h"
#include "dmhpayload.h"
#include "dmhlogon.h"
#include "map.h"
#include "undobase.h"
#include <QFile>
#include <QFileInfo>
#include <QBuffer>
#include <QMessageBox>
#include <QDomDocument>
#include <QPushButton>
#include <QNetworkReply>
#include <QUndoStack>
#include <QDir>
#include <QDebug>

// Uncomment this define to output payload data to a local file "_dmhpayload.txt"
#define DMH_NETWORK_CONTROLLER_LOCAL_PAYLOAD

NetworkController::NetworkController(QObject *parent) :
    QObject(parent),
    _networkManager(new DMHNetworkManager(DMHLogon(), this)),
    _payload(),
    _currentImageRequest(UploadObject::Status_Complete),
    _tracks(),
    _backgroundUpload(),
    _backgroundCacheKey(0),
    _fowUpload(),
    _backgroundColor(),
    _enabled(false)
{
    connect(_networkManager, SIGNAL(uploadComplete(int, const QString&)), this, SLOT(uploadCompleted(int, const QString&)));
    connect(_networkManager, SIGNAL(existsComplete(int, const QString&, const QString&, bool)), this, SLOT(existsCompleted(int, const QString&, const QString&, bool)));
    connect(_networkManager, SIGNAL(requestError(int)), this, SLOT(uploadError(int)));
    connect(_networkManager, SIGNAL(requestError(int)), this, SIGNAL(networkError(int)));
    connect(_networkManager, SIGNAL(otherRequestComplete()), this, SIGNAL(networkSuccess()));
    connect(_networkManager, &DMHNetworkManager::uploadComplete, this, &NetworkController::uploadComplete);
    connect(_networkManager, &DMHNetworkManager::downloadStarted, this, &NetworkController::downloadStarted);
    connect(_networkManager, &DMHNetworkManager::downloadComplete, this, &NetworkController::downloadComplete);
}

NetworkController::~NetworkController()
{
    disconnect(_networkManager, nullptr, this, nullptr);
}

void NetworkController::addTrack(AudioTrack* track)
{
    if((!track) || (containsTrack(track)))
        return;

    qDebug() << "[NetworkController] Adding audio track: " << track << ", ID: " << track->getID();
    connect(track, &AudioTrack::campaignObjectDestroyed, this, &NetworkController::removeTrackUUID);
    connect(track, &AudioTrack::muteChanged, this, &NetworkController::updateAudioPayload);
    connect(track, &AudioTrack::repeatChanged, this, &NetworkController::updateAudioPayload);

    UploadObject trackUpload(track, track->getFileName(), UploadObject::Status_Complete);
    if(track->getAudioType() == DMHelper::AudioType_File)
    {
        trackUpload.setStatus(UploadObject::Status_NotStarted);
        startObjectUpload(&trackUpload);
        _tracks.append(trackUpload);
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
        if(_tracks.at(i).getObject() == trackObject)
        {
            disconnect(track, &AudioTrack::campaignObjectDestroyed, this, &NetworkController::removeTrackUUID);
            disconnect(track, &AudioTrack::muteChanged, this, &NetworkController::updateAudioPayload);
            disconnect(track, &AudioTrack::repeatChanged, this, &NetworkController::updateAudioPayload);
            _tracks.removeAt(i);
            updateAudioPayload();
            return;
        }
    }
}

void NetworkController::setBackgroundColor(QColor color)
{
    if(!_enabled)
        return;

    if(color.name() != _backgroundColor)
    {
        _backgroundColor = color.name();
        updateImagePayload();
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
        if(_backgroundUpload.getStatus() > 0)
            _networkManager->abortRequest(_backgroundUpload.getStatus());

        _backgroundCacheKey = background.cacheKey();
        _backgroundUpload = UploadObject(nullptr, uploadImage(background, QString("Published Image")));
    }

    if(_fowUpload.isValid())
    {
        changed = true;
        if(_fowUpload.getStatus() > 0)
            _networkManager->abortRequest(_fowUpload.getStatus());

        _fowUpload = UploadObject();
    }

    if(color.name() != _backgroundColor)
    {
        changed = true;
        _backgroundColor = color.name();
    }

    if(changed)
        updateImagePayload();
}

void NetworkController::uploadObject(CampaignObjectBase* baseObject)
{
    if(!baseObject)
        return;

    bool changed = false;

    if(baseObject->getObjectType() == DMHelper::CampaignType_Map)
    {
        if(baseObject != _backgroundUpload.getObject())
        {
            if(_backgroundUpload.getStatus() > 0)
                _networkManager->abortRequest(_backgroundUpload.getStatus());

            _backgroundUpload = UploadObject(baseObject);
            _backgroundCacheKey = 0;
            changed = true;
        }

        Map* map = dynamic_cast<Map*>(baseObject);
        QUndoStack* undoStack = map->getUndoStack();
        if(undoStack)
        {
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

            if(_fowUpload.getStatus() > 0)
                _networkManager->abortRequest(_fowUpload.getStatus());

            _fowUpload = UploadObject();
            _fowUpload.setData(doc.toString());
            _fowUpload.setDescription(QString("Fog of War: ") + baseObject->getName());
            changed = true;
        }
    }

    if(changed)
        updateImagePayload();
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

    _tracks.clear();
    updateAudioPayload();

    // uploadPayload();
}

void NetworkController::clearImage()
{
    if(!_enabled)
        return;

    if(_currentImageRequest > 0)
        _networkManager->abortRequest(_currentImageRequest);

    _payload.setImageFile(QString());
    uploadPayload();
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
        if(!validateLogon(_networkManager->getLogon()))
            return;

        clearUploadErrors();
        updateAudioPayload();
    }

    emit networkEnabledChanged(_enabled);
}

void NetworkController::setNetworkLogin(const QString& urlString, const QString& username, const QString& password, const QString& sessionID, const QString& inviteID)
{
    Q_UNUSED(inviteID);

    if(!_enabled)
        return;

    qDebug() << "[NetworkController] Network login updated. URL: " << urlString << ", Username: " << username << ", Session: " << sessionID << ", Invite: " << inviteID;

    DMHLogon logon(urlString, username, password, sessionID);
    if(!validateLogon(logon))
        return;

    _networkManager->setLogon(logon);

    clearUploadErrors();
    updateAudioPayload();
}

void NetworkController::uploadCompleted(int requestID, const QString& fileMD5)
{
    qDebug() << "[NetworkController] Upload complete " << requestID << ": " << fileMD5;

    if(_backgroundUpload.getStatus() == requestID)
    {
        if(fileMD5.isEmpty())
        {
            qDebug() << "[NetworkController] Upload complete for background image with invalid MD5 value, no payload uploaded.";
            _backgroundUpload.setStatus(UploadObject::Status_Error);
        }
        else
        {
            qDebug() << "[NetworkController] Upload complete for background image: " << fileMD5;
            _backgroundUpload.setMD5(fileMD5);
            _backgroundUpload.setStatus(UploadObject::Status_Complete);
            updateImagePayload();
            uploadPayload();
        }

        return;
    }

    if(_fowUpload.getStatus() == requestID)
    {
        if(fileMD5.isEmpty())
        {
            qDebug() << "[NetworkController] Upload complete for fow with invalid MD5 value, no payload uploaded.";
            _fowUpload.setStatus(UploadObject::Status_Error);
        }
        else
        {
            qDebug() << "[NetworkController] Upload complete for fow: " << fileMD5;
            _fowUpload.setMD5(fileMD5);
            _fowUpload.setStatus(UploadObject::Status_Complete);
            updateImagePayload();
            uploadPayload();
        }

        return;
    }

    for(int i = 0; i < _tracks.count(); ++i)
    {
        if(_tracks.at(i).getStatus() == requestID)
        {
            UploadObject uploadObject = _tracks.at(i);
            if(fileMD5.isEmpty())
            {
                uploadObject.setStatus(UploadObject::Status_Error);
                qDebug() << "[NetworkController] ERROR: Upload complete for Audio with invalid MD5 value: " << requestID;
                _tracks[i] = uploadObject;
            }
            else
            {
                uploadObject.setStatus(UploadObject::Status_Complete);
                if(uploadObject.getObject())
                    uploadObject.getObject()->setMD5(fileMD5);

                qDebug() << "[NetworkController] Upload complete for track: " << uploadObject.getObject() << ", MD5: " << fileMD5;
                _tracks[i] = uploadObject;
                updateAudioPayload();
                uploadPayload();
            }
            return;
        }
    }

    qDebug() << "[NetworkController] ERROR: Unexpected request ID received: " << requestID;
}

void NetworkController::existsCompleted(int requestID, const QString& fileMD5, const QString& filename, bool exists)
{
    Q_UNUSED(filename);

    qDebug() << "[NetworkController] Exists complete " << requestID << ": " << fileMD5;

    for(int i = 0; i < _tracks.count(); ++i)
    {
        if(_tracks.at(i).getStatus() == requestID)
        {
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
        }
    }

    qDebug() << "[NetworkController] ERROR: Unexpected file exists request ID received!";
}

void NetworkController::uploadError(int requestID)
{
    if(requestID == _currentImageRequest)
    {
        qDebug() << "[NetworkController] Upload image error discovered for request " << requestID;
        _currentImageRequest = 0;
        return;
    }

    for(int i = 0; i < _tracks.count(); ++i)
    {
        if(_tracks.at(i).getStatus() == requestID)
        {
            qDebug() << "[NetworkController] Upload audio error discovered for request " << requestID;
            _tracks[i].setStatus(UploadObject::Status_Error);
            return;
        }
    }

    qDebug() << "[NetworkController] ERROR: Unexpected request ID received: " << requestID;
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

    qDebug() << "[NetworkController] Uploading payload";
    _networkManager->uploadPayload(_payload);
}

void NetworkController::removeTrackUUID(const QUuid& id)
{
    for(int i = 0; i < _tracks.count(); ++i)
    {
        if((_tracks.at(i).getObject()) && (_tracks.at(i).getObject()->getID() == id))
        {
            AudioTrack* track = dynamic_cast<AudioTrack*>(_tracks.at(i).getObject());
            removeTrack(track);
            return;
        }
    }
}

void NetworkController::startObjectUpload(UploadObject* uploadObject)
{
    if((!_enabled) || (!uploadObject))
    {
        qDebug() << "[NetworkController] Uploading object failed! enabled: " << _enabled << ", uploadObject: " << uploadObject;
        return;
    }

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

    QString uploadName = uploadObject->getDescription();
    if((!uploadObject->getDescription().isEmpty()) && (uploadObject->getObject() != nullptr))
        uploadName += QString(": ");
    if(uploadObject->getObject())
        uploadName += uploadObject->getObject()->getName();

    if(uploadObject->getMD5().isEmpty())
    {
        if(uploadObject->getFilename().isEmpty())
            uploadObject->setStatus(_networkManager->uploadData(uploadObject->getData().toUtf8()));
        else
            uploadObject->setStatus(_networkManager->uploadFile(uploadObject->getFilename()));

        if(uploadObject->getStatus() > 0)
        {
            qDebug() << "[NetworkController] Uploading object: " << uploadName << ", MD5: " << uploadObject->getMD5() << ", Request: " << uploadObject->getStatus();
            emit uploadStarted(uploadObject->getStatus(), _networkManager->getNetworkReply(uploadObject->getStatus()), uploadName);
        }
    }
    else
    {
        uploadObject->setStatus(_networkManager->fileExists(uploadObject->getMD5()));
        qDebug() << "[NetworkController] Checking if object exists: " << uploadName << ", MD5: " << uploadObject->getMD5() << ", Request: " << uploadObject->getStatus();
    }
}

bool NetworkController::containsTrack(AudioTrack* track)
{
    if(!track)
        return false;

    CampaignObjectBase* baseObject = dynamic_cast<CampaignObjectBase*>(track);
    for(int i = 0; i < _tracks.count(); ++i)
    {
        if(_tracks.at(i).getObject() == baseObject)
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
        if(_tracks.at(i).getObject())
        {
            if(_tracks.at(i).getStatus() == UploadObject::Status_Complete)
            {
                QDomElement trackElement = _tracks.at(i).getObject()->outputNextworkXML(doc);
                doc.appendChild(trackElement);
            }
            else if(_tracks.at(i).getStatus() == UploadObject::Status_NotStarted)
            {
                UploadObject uploadObject = _tracks.at(i);
                startObjectUpload(&uploadObject);
                _tracks[i] = uploadObject;
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

    for(int i = 0; i < _tracks.count(); ++i)
    {
        if(_tracks.at(i).getStatus() == UploadObject::Status_Error)
            _tracks[i].setStatus(UploadObject::Status_NotStarted);
    }
}

int NetworkController::uploadImage(QImage image, const QString& imageName)
{
    if((!_enabled) || (image.isNull()))
        return UploadObject::Status_Error;

    QByteArray data;
    QBuffer buffer(&data);
    if(!buffer.open(QIODevice::WriteOnly))
        return UploadObject::Status_Error;

    if(!image.save(&buffer, "PNG"))
        return UploadObject::Status_Error;

    QByteArray byteHash = QCryptographicHash::hash(data, QCryptographicHash::Md5);
    qDebug() << "[NetworkController] Uploading image " << image << "data with MD5 hash HEX: " << byteHash.toHex(0);

    int requestId = _networkManager->uploadData(data);
    if(requestId > 0)
        emit uploadStarted(requestId, _networkManager->getNetworkReply(requestId), imageName);

    return requestId;
}

void NetworkController::updateImagePayload()
{
    if(!_enabled)
        return;

    QString imagePayload;

    if(_backgroundUpload.getStatus() < UploadObject::Status_Complete)
        startObjectUpload(&_backgroundUpload);

    if(_backgroundUpload.hasMD5())
    {
        imagePayload += QString("<background>") + _backgroundUpload.getMD5() + QString("</background>");
        if(_fowUpload.isValid())
        {
            if(_fowUpload.getStatus() < UploadObject::Status_Complete)
                startObjectUpload(&_fowUpload);

            imagePayload += QString("<fow>") + _fowUpload.getMD5() + QString("</fow>");
        }
    }

    qDebug() << "[NetworkController] Image payload set to: " << imagePayload;
    _payload.setImageFile(imagePayload);
    uploadPayload();
}

bool NetworkController::validateLogon(const DMHLogon& logon)
{
    if(logon.isValid())
        return true;

    QMessageBox msgBox(QMessageBox::Warning,
                       QString("DMHelper Network Connection"),
                       QString("Warning: The network client publishing is enabled, but the network URL, username, password and/or session ID are not properly set. Network publishing will not work unless these values are correct.\n\nWould you like to correct these settings or disable network publishing?"));
    QAbstractButton* openButton = msgBox.addButton(QString("Open Network Settings"), QMessageBox::AcceptRole);
    msgBox.addButton(QString("Disable Network Publish"), QMessageBox::RejectRole);
    msgBox.exec();
    if(msgBox.clickedButton() == openButton)
        emit requestSettings(DMHelper::OptionsTab_Network);
    else
        enableNetworkController(false);

    return false;
}

#endif // INCLUDE_NETWORK_SUPPORT
