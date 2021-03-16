#include "networkcontroller.h"

#ifdef INCLUDE_NETWORK_SUPPORT

#include "audiotrack.h"
#include "dmhnetworkmanager.h"
#include "dmhpayload.h"
#include "dmhlogon.h"
#include <QFile>
#include <QFileInfo>
#include <QBuffer>
#include <QMessageBox>
#include <QDomDocument>
#include <QPushButton>
#include <QNetworkReply>
#include <QDebug>

// Uncomment this define to output payload data to a local file "_dmhpayload.txt"
#define DMH_NETWORK_CONTROLLER_LOCAL_PAYLOAD

const int DMH_NETWORK_CONTROLLER_UPLOAD_COMPLETE = 0;
const int DMH_NETWORK_CONTROLLER_UPLOAD_NOT_STARTED = -1;
const int DMH_NETWORK_CONTROLLER_UPLOAD_ERROR = -2;

NetworkController::NetworkController(QObject *parent) :
    QObject(parent),
    _networkManager(new DMHNetworkManager(DMHLogon(), this)),
    _payload(),
    _currentImageRequest(DMH_NETWORK_CONTROLLER_UPLOAD_COMPLETE),
    _tracks(),
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

    AudioTrackUpload trackPair(DMH_NETWORK_CONTROLLER_UPLOAD_COMPLETE, track);
    if(trackPair.second->getAudioType() == DMHelper::AudioType_File)
    {
        trackPair.first = DMH_NETWORK_CONTROLLER_UPLOAD_NOT_STARTED;
        uploadTrack(&trackPair);
        _tracks.append(trackPair);
    }
    else
    {
        _tracks.append(trackPair);
        updateAudioPayload();
    }
}

void NetworkController::removeTrack(AudioTrack* track)
{
    if(!track)
        return;

    for(int i = 0; i < _tracks.count(); ++i)
    {
        if(_tracks.at(i).second == track)
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

void NetworkController::uploadImage(QImage img)
{
    if(!_enabled)
        return;

    if(img.isNull())
        return;

    qDebug() << "[NetworkController] Uploading image...";

    QByteArray data;
    QBuffer buffer(&data);
    if(!buffer.open(QIODevice::WriteOnly))
        return;
    if(!img.save(&buffer, "PNG"))
        return;

    if(_currentImageRequest > 0)
        _networkManager->abortRequest(_currentImageRequest);

    _currentImageRequest = _networkManager->uploadData(data);
    if(_currentImageRequest > 0)
        emit uploadStarted(_currentImageRequest, _networkManager->getNetworkReply(_currentImageRequest), QString("Published Image"));
}

void NetworkController::uploadImage(QImage img, QColor color)
{
    uploadImage(img);
//    <<<<<<<need to also upload the background color
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

    if(requestID == _currentImageRequest)
    {
        if(fileMD5.isEmpty())
        {
            _currentImageRequest = DMH_NETWORK_CONTROLLER_UPLOAD_ERROR;
            qDebug() << "[NetworkController] Upload complete for Image with invalid MD5 value, no payload uploaded.";
        }
        else
        {
            _currentImageRequest = DMH_NETWORK_CONTROLLER_UPLOAD_COMPLETE;
            _payload.setImageFile(fileMD5);
            qDebug() << "[NetworkController] Upload complete for Image: " << fileMD5;
            uploadPayload();
        }
        return;
    }

    for(int i = 0; i < _tracks.count(); ++i)
    {
        if(_tracks.at(i).first == requestID)
        {
            AudioTrackUpload uploadPair = _tracks.at(i);
            if(fileMD5.isEmpty())
            {
                uploadPair.first = DMH_NETWORK_CONTROLLER_UPLOAD_ERROR;
                qDebug() << "[NetworkController] ERROR: Upload complete for Audio with invalid MD5 value: " << requestID;
                _tracks[i] = uploadPair;
            }
            else
            {
                uploadPair.first = DMH_NETWORK_CONTROLLER_UPLOAD_COMPLETE;
                if(uploadPair.second)
                    uploadPair.second->setMD5(fileMD5);

                qDebug() << "[NetworkController] Upload complete for track: " << uploadPair.second << ", MD5: " << fileMD5;
                uploadPayload();
                _tracks[i] = uploadPair;
                updateAudioPayload();
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
        if(_tracks.at(i).first == requestID)
        {
            AudioTrackUpload uploadPair = _tracks.at(i);
            if(exists)
            {
                qDebug() << "[NetworkController] Audio track already exists, updating payload directly.";
                uploadPair.first = DMH_NETWORK_CONTROLLER_UPLOAD_COMPLETE;
                if(uploadPair.second)
                {
                    uploadPair.second->setMD5(fileMD5);
                }
                _tracks[i] = uploadPair;
                updateAudioPayload();
            }
            else
            {
                qDebug() << "[NetworkController] Audio track does not exist, beginning upload.";
                uploadPair.first = DMH_NETWORK_CONTROLLER_UPLOAD_NOT_STARTED;
                if(uploadPair.second)
                {
                    uploadPair.second->setMD5(QString());
                    uploadTrack(&uploadPair);
                }
                _tracks[i] = uploadPair;
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
        if(_tracks.at(i).first == requestID)
        {
            qDebug() << "[NetworkController] Upload audio error discovered for request " << requestID;
            _tracks[i].first = DMH_NETWORK_CONTROLLER_UPLOAD_ERROR;
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
        if((_tracks.at(i).second) && (_tracks.at(i).second->getID() == id))
        {
            removeTrack(_tracks.at(i).second);
            return;
        }
    }
}

void NetworkController::uploadTrack(AudioTrackUpload* trackPair)
{
    if((!trackPair) ||(!_enabled) || (!trackPair->second) || (trackPair->second->getAudioType() != DMHelper::AudioType_File))
        return;

    QString trackFile = trackPair->second->getUrl().toString();
    if((trackFile.isEmpty()) || (!QFile::exists(trackFile)))
    {
        qDebug() << "[NetworkController] Uploading local file failed! File not found: " << trackFile;
        trackPair->first = DMH_NETWORK_CONTROLLER_UPLOAD_ERROR;
        return;
    }

    if(trackPair->second->getMD5().isEmpty())
    {
        trackPair->first = _networkManager->uploadFile(trackFile);
        if(trackPair->first > 0)
            emit uploadStarted(trackPair->first, _networkManager->getNetworkReply(trackPair->first), trackPair->second->getName());
    }
    else
    {
        trackPair->first = _networkManager->fileExists(trackPair->second->getMD5());
    }

    qDebug() << "[NetworkController] Uploading audio track: " << trackFile << ", MD5: " << trackPair->second->getMD5() << ", Request: " << trackPair->first;
}

bool NetworkController::containsTrack(AudioTrack* track)
{
    if(!track)
        return false;

    for(int i = 0; i < _tracks.count(); ++i)
    {
        if(_tracks.at(i).second == track)
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
        if(_tracks.at(i).second)
        {
            if(_tracks.at(i).first == DMH_NETWORK_CONTROLLER_UPLOAD_COMPLETE)
            {
                QDomElement trackElement = doc.createElement("audio-track");

                trackElement.setAttribute("type", _tracks.at(i).second->getAudioType());
                trackElement.setAttribute("id", _tracks.at(i).second->getID().toString());
                trackElement.setAttribute("md5", _tracks.at(i).second->getMD5());
                trackElement.setAttribute("trackname", _tracks.at(i).second->getName());
                trackElement.setAttribute("repeat", _tracks.at(i).second->isRepeat());
                trackElement.setAttribute("mute", _tracks.at(i).second->isMuted());

                QDomCDATASection urlData = doc.createCDATASection(_tracks.at(i).second->getUrl().toString());
                trackElement.appendChild(urlData);

                doc.appendChild(trackElement);
            }
            else if(_tracks.at(i).first == DMH_NETWORK_CONTROLLER_UPLOAD_NOT_STARTED)
            {
                AudioTrackUpload uploadPair = _tracks.at(i);
                uploadTrack(&uploadPair);
                _tracks[i] = uploadPair;
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
        if(_tracks.at(i).first == DMH_NETWORK_CONTROLLER_UPLOAD_ERROR)
            _tracks[i].first = DMH_NETWORK_CONTROLLER_UPLOAD_NOT_STARTED;
    }
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
