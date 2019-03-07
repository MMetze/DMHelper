#include "networkcontroller.h"

#ifdef INCLUDE_NETWORK_SUPPORT

#include "audiotrack.h"
#include "dmhnetworkmanager.h"
#include "dmhpayload.h"
#include "dmhlogon.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QBuffer>
#include <QMessageBox>

// Uncomment this define to output payload data to a local file "_dmhpayload.txt"
#define DMH_NETWORK_CONTROLLER_LOCAL_PAYLOAD

NetworkController::NetworkController(QObject *parent) :
    QObject(parent),
    _networkManager(nullptr),
    _imageMD5(),
    _audioMD5(),
    _currentImageRequest(0),
    _currentAudioRequest(0),
    _currentPayloadRequest(0),
    _currentTrack(nullptr),
    _enabled(false)
{
    //DMHLogon logon("c.turner", "Ente12345", "7B3AA550-649A-4D51-920E-CAB465616995");

    _networkManager = new DMHNetworkManager(DMHLogon(), this);
    connect(_networkManager, SIGNAL(uploadComplete(int, const QString&)), this, SLOT(uploadComplete(int, const QString&)));
    connect(_networkManager, SIGNAL(existsComplete(int, const QString&, const QString&, bool)), this, SLOT(existsComplete(int, const QString&, const QString&, bool)));
    connect(_networkManager, SIGNAL(requestError(int)), this, SLOT(uploadError(int)));
}

NetworkController::~NetworkController()
{
    disconnect(_networkManager, nullptr, this, nullptr);
}

void NetworkController::uploadTrack(AudioTrack* track)
{
//    if((_currentAudioRequest > 0) || (!_enabled))
    if(!_enabled)
        return;

    if((!track) || (track->getUrl().isEmpty()))
        return;

    _currentTrack = track;
    if(track->getMD5().isEmpty())
    {
        _currentAudioRequest = _networkManager->uploadFile(track->getUrl().toString());
    }
    else
    {
        _currentAudioRequest = _networkManager->fileExists(track->getMD5());
    }
}

void NetworkController::uploadImage(QImage img)
{
//    if((_currentImageRequest > 0) || (!_enabled))
    if(!_enabled)
        return;

    if(img.isNull())
        return;

    QByteArray data;
    QBuffer buffer(&data);
    if(!buffer.open(QIODevice::WriteOnly))
        return;
    if(!img.save(&buffer, "PNG"))
        return;
    _currentImageRequest = _networkManager->uploadData(data);
}

void NetworkController::setPayload(const QString& command, const QString& payload)
{
    if(!_enabled)
        return;

    _command = command;
    //_payload = payload;
    //uploadPayload();

#ifdef DMH_NETWORK_CONTROLLER_LOCAL_PAYLOAD
    QFile localPayload("_dmhpayload.txt");
    localPayload.open(QIODevice::WriteOnly);
    localPayload.write(payload.toUtf8());
    localPayload.close();

    QFileInfo fi(localPayload);
    qDebug() <<"[NetworkController] DEBUG payload output written to location: " << fi.absoluteFilePath();
#endif

    _currentPayloadRequest = _networkManager->uploadData(payload.toUtf8());
}

void NetworkController::clearTrack()
{
    if((_currentAudioRequest > 0) || (!_enabled))
        return;

    _audioMD5 = QString("");
    uploadPayload();
}

void NetworkController::clearImage()
{
    if((_currentImageRequest > 0) || (!_enabled))
        return;

    _imageMD5 = QString("");
    uploadPayload();
}

void NetworkController::clearPayload()
{
    if((_currentPayloadRequest > 0) || (!_enabled))
        return;

    _command = QString("");
    _payload = QString("");
    uploadPayload();
}

void NetworkController::enableNetworkController(bool enabled)
{
    if(enabled != _enabled)
    {
        _enabled = enabled;
        if(_enabled)
            uploadPayload();
    }
}

void NetworkController::setNetworkLogin(const QString& urlString, const QString& username, const QString& password, const QString& sessionID, const QString& inviteID)
{
    Q_UNUSED(inviteID);

    if(!_networkManager)
        return;

    if((username.isEmpty()) || (password.isEmpty()) || (sessionID.isEmpty()))
        QMessageBox::warning(nullptr, QString("DM Helper Network Connection"), QString("Warning: The network client publishing is enabled, but username, password and/or session ID are not set. Network publishing will not work unless these values are correct."));

    DMHLogon logon(urlString, username, password, sessionID);
    _networkManager->setLogon(logon);
}

void NetworkController::uploadComplete(int requestID, const QString& fileMD5)
{
    qDebug() << "[NetworkController] Upload complete " << requestID << ": " << fileMD5;

    if(requestID == _currentImageRequest)
    {
        _currentImageRequest = 0;
        if(fileMD5.isEmpty())
        {
            qDebug() << "[NetworkController] Upload complete for Image with invalid MD5 value, no payload uploaded.";
        }
        else
        {
            _imageMD5 = fileMD5;
            qDebug() << "[NetworkController] Upload complete for Image: " << _imageMD5;
            uploadPayload();
        }
    }
    else if(requestID == _currentAudioRequest)
    {
        _currentAudioRequest = 0;
        if(fileMD5.isEmpty())
        {
            _currentTrack = nullptr;
            qDebug() << "[NetworkController] Upload complete for Audio with invalid MD5 value, no payload uploaded.";
        }
        else
        {
            _currentTrack->setMD5(fileMD5);
            _currentTrack = nullptr;
            _audioMD5 = fileMD5;
            qDebug() << "[NetworkController] Upload complete for Audio: " << _audioMD5;
            uploadPayload();
        }
    }
    else if(requestID == _currentPayloadRequest)
    {
        _currentPayloadRequest = 0;
        if(fileMD5.isEmpty())
        {
            qDebug() << "[NetworkController] Upload complete for Payload with invalid MD5 value, no payload uploaded.";
        }
        else
        {
            _payload = fileMD5;
            qDebug() << "[NetworkController] Upload complete for Payload: " << _payload;
            uploadPayload();
        }
    }
    else
    {
        qDebug() << "[NetworkController] ERROR: Unexpected request ID received!";
    }
}

void NetworkController::existsComplete(int requestID, const QString& fileMD5, const QString& filename, bool exists)
{
    Q_UNUSED(filename);

    qDebug() << "[NetworkController] Exists complete " << requestID << ": " << fileMD5;

    if(requestID == _currentAudioRequest)
    {
        if(exists)
        {
            qDebug() << "[NetworkController] Audio track already exists, updating payload directly.";
            _currentTrack = nullptr;
            _currentAudioRequest = 0;
            _audioMD5 = fileMD5;
            uploadPayload();
        }
        else
        {
            qDebug() << "[NetworkController] Audio track does not exist, beginning upload.";
            _currentAudioRequest = _networkManager->uploadFile(_currentTrack->getUrl().toString());
        }
    }
    else
    {
        qDebug() << "[NetworkController] ERROR: Unexpected file exists request ID received!";
    }
}

void NetworkController::uploadError(int requestID)
{
    qDebug() << "[NetworkController] Upload error discovered for " << requestID;

    if(requestID == _currentImageRequest)
        _currentImageRequest = 0;
    else if(requestID == _currentAudioRequest)
        _currentAudioRequest = 0;
    else
        qDebug() << "[NetworkController] ERROR: Unexpected request ID received!";
}

void NetworkController::uploadPayload()
{
    if((!_networkManager) || (!_enabled))
        return;

    qDebug() << "[NetworkController] Uploading payload. Audio: " << _audioMD5 << ", Image: " << _imageMD5 << ", Command: " << _command << ", Payload: " << _payload;
    DMHPayload payload;
    payload.setImageFile(_imageMD5);
    payload.setAudioFile(_audioMD5);
    payload.setCommand(_command);
    payload.setPayload(_payload);
    _networkManager->uploadPayload(payload);
}

#endif // INCLUDE_NETWORK_SUPPORT
