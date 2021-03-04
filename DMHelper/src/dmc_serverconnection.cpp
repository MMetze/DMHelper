#include "dmc_serverconnection.h"
#include "dmc_constants.h"
#include "dmhnetworkmanager.h"
#include "dmhnetworkobserver.h"
#include "dmhpayload.h"
#include "dmhlogon.h"
#include "dmconstants.h"
#include "dmversion.h"
#include "remoteaudioplayer.h"
#include "audiofactory.h"
#include "audiotrack.h"
#include <QFileDialog>
#include <QFile>
#include <QDomDocument>
#include <QDebug>

const int DUMMY_DOWNLOAD_ID = 1;

DMC_ServerConnection::DMC_ServerConnection(QObject *parent) :
    QObject(parent),
    _networkManager(nullptr),
    _networkObserver(nullptr),
    _audioPlayer(new RemoteAudioPlayer(this)),
    _imageMD5client(),
    _audioMD5client(),
    _currentImageRequest(0),
    _currentAudioRequest(0),
    _track(nullptr),
    _pmp(),
    _lastPayload()
{
}

DMC_ServerConnection::DMC_ServerConnection(const DMHLogon& logon, QObject *parent) :
    QObject(parent),
    _networkManager(nullptr),
    _networkObserver(nullptr),
    _audioPlayer(new RemoteAudioPlayer(this)),
    _imageMD5client(),
    _audioMD5client(),
    _currentImageRequest(0),
    _currentAudioRequest(0),
    _track(nullptr),
    _pmp(),
    _lastPayload()
{
    startServer(logon);
}

DMC_ServerConnection::DMC_ServerConnection(const QString& urlString, const QString& username, const QString& password, const QString& session, QObject *parent) :
    QObject(parent),
    _networkManager(nullptr),
    _networkObserver(nullptr),
    _audioPlayer(new RemoteAudioPlayer(this)),
    _imageMD5client(),
    _audioMD5client(),
    _currentImageRequest(0),
    _currentAudioRequest(0),
    _track(nullptr),
    _pmp(),
    _lastPayload()
{
    startServer(DMHLogon(urlString, username, password, session));
}

DMC_ServerConnection::~DMC_ServerConnection()
{
    stopServer();
}

void DMC_ServerConnection::downloadComplete(int requestID, const QString& fileMD5, const QByteArray& data)
{
    qDebug() << "[DMC_ServerConnection] Download complete " << requestID << ": " << fileMD5 << ", " << data.size() << " bytes";
    if(requestID == _currentImageRequest)
    {
        _currentImageRequest = 0;
        if(data.size() > 0)
        {
            if(_pmp.loadFromData(data))
                //ui->lblImage->setPixmap(_pmp.scaled(ui->lblImage->size(), Qt::KeepAspectRatio));
                emit pixmapActive(_pmp);
            else
                qDebug() << "[DMC_ServerConnection] Download complete Pixmap loading failed";
        }
        else
        {
            qDebug() << "[DMC_ServerConnection] WARNING: Download complete for image download with no data received, no pixmap set";
        }
    }
    else if(requestID == _currentAudioRequest)
    {
        stopAudio();
        _currentAudioRequest = 0;

        QUrl receivedUrl(fileMD5);
        AudioFactory audioFactory;
        if(audioFactory.identifyAudioSubtype(receivedUrl) == DMHelper::AudioType_File)
        {
            QFile outputFile(fileMD5 + QString(".mp3"));
            if(data.size() > 0)
            {
                if(!outputFile.exists())
                {
                    if(outputFile.open(QIODevice::WriteOnly))
                    {
                        outputFile.write(data);
                        outputFile.close();
                    }
                }
            }

            if(outputFile.exists())
            {
                QFileInfo fileInfo(outputFile);
                _track = audioFactory.createTrackFromUrl(QUrl(fileInfo.filePath()), fileInfo.fileName());
            }
            else
            {
                qDebug() << "[DMC_ServerConnection] WARNING: Download complete for audio download with no data received, no playback possible";
            }
        }
        else
        {
            _track = audioFactory.createTrackFromUrl(receivedUrl, receivedUrl.fileName());
        }

        if(_track)
        {
            qDebug() << "[Main] Playing track: " << _track;
            //connect(ui->sliderVolume, SIGNAL(valueChanged(int)), _track, SLOT(setVolume(int)));
            //connect(ui->btnMute, SIGNAL(toggled(bool)), _track, SLOT(setMute(bool)));
            _track->play();
            emit trackActive(_track);
            //enableAudio(true);
        }
        else
        {
            qDebug() << "[DMC_ServerConnection] WARNING: No track played.";
        }
    }
    else
    {
        qDebug() << "[DMC_ServerConnection] ERROR: Unexpected request ID received!";
    }
}

/*
void DMC_ServerConnection::uploadComplete(int requestID, const QString& fileMD5)
{
    qDebug() << "[Main] Upload complete " << requestID << ": " << fileMD5;

    if(requestID == _currentImageRequest)
    {
        _currentImageRequest = 0;
        if(fileMD5.isEmpty())
        {
            qDebug() << "[Main] Upload complete for Image with invalid MD5 value, no payload uploaded.";
        }
        else
        {
            DMHPayload payload;
            _imageMD5server = fileMD5;
            payload.setImageFile(_imageMD5server);
            payload.setAudioFile(_audioMD5server);
            _networkManager->uploadPayload(payload);
        }
    }
    else if(requestID == _currentAudioRequest)
    {
        _currentAudioRequest = 0;
        if(fileMD5.isEmpty())
        {
            qDebug() << "[Main] Upload complete for Audio with invalid MD5 value, no payload uploaded.";
        }
        else
        {
            DMHPayload payload;
            _audioMD5server = fileMD5;
            payload.setImageFile(_imageMD5server);
            payload.setAudioFile(_audioMD5server);
            _networkManager->uploadPayload(payload);
        }
    }
    else
    {
        qDebug() << "[Main] ERROR: Unexpected request ID received!";
    }
}
*/

void DMC_ServerConnection::payloadReceived(const DMHPayload& payload, const QString& timestamp)
{
    if(timestamp == _lastPayload)
        return;

    if(_currentImageRequest <= 0)
    {
        if(payload.getImageFile() != _imageMD5client)
        {
            qDebug() << "[DMC_ServerConnection] Payload received with new Image file. Image: " << payload.getImageFile() << ", Audio: " << payload.getAudioFile() << ", Timestamp: " << timestamp;
            _imageMD5client = payload.getImageFile();
            if(_imageMD5client.isEmpty())
                //ui->lblImage->setPixmap(QPixmap());
                emit pixmapActive(QPixmap());
            else
                _currentImageRequest = _networkManager->downloadFile(_imageMD5client);
        }
    }

    if(_currentAudioRequest <= 0)
    {
        if(payload.getAudioFile() != _audioMD5client)
        {
            qDebug() << "[DMC_ServerConnection] Payload received with new Audio file. Image: " << payload.getImageFile() << ", Audio: " << payload.getAudioFile() << ", Timestamp: " << timestamp;
            if(_audioPlayer)
                _audioPlayer->parseAudioString(payload.getAudioFile());

            /*
            _audioMD5client = payload.getAudioFile();
            if(_audioMD5client.isEmpty())
            {
                stopAudio();
            }
            else
            {
                QUrl audioUrl = QUrl::fromUserInput(_audioMD5client);
                if((!audioUrl.isValid()) && (QFile::exists(_audioMD5client + QString(".mp3"))))
                {
                    _currentAudioRequest = _networkManager->downloadFile(_audioMD5client);
                }
                else
                {
                    _currentAudioRequest = DUMMY_DOWNLOAD_ID;
                    downloadComplete(DUMMY_DOWNLOAD_ID, _audioMD5client, QByteArray());
                }
            }
        */
        }
    }

    _lastPayload = timestamp;
}

void DMC_ServerConnection::startServer(const DMHLogon& logon)
{
    qDebug() << "[DMC_ServerConnection] Starting server with logon: " << logon;

    stopServer();

    _networkManager = new DMHNetworkManager(logon, this);
    connect(_networkManager, SIGNAL(downloadComplete(int, const QString&, const QByteArray&)), this, SLOT(downloadComplete(int, const QString&, const QByteArray&)));

    _networkObserver = new DMHNetworkObserver(logon, this);
    connect(_networkObserver, SIGNAL(payloadReceived(const DMHPayload&, const QString&)), this, SLOT(payloadReceived(const DMHPayload&, const QString&)));
    _networkObserver->start();
}

void DMC_ServerConnection::stopServer()
{
    qDebug() << "[DMC_ServerConnection] Stop server.";

    if(_networkManager)
    {
        disconnect(_networkManager, nullptr, this, nullptr);
        _networkManager->deleteLater();
        _networkManager = nullptr;
    }

    if(_networkObserver)
    {
        disconnect(_networkObserver, nullptr, this, nullptr);
        _networkObserver->deleteLater();
        _networkObserver = nullptr;
    }

    stopAudio();
}

void DMC_ServerConnection::parseAudioData(const QString& audioData)
{
    QDomDocument doc;
    QString contentError;
    int contentErrorLine = 0;
    int contentErrorColumn = 0;
    bool contentResult = doc.setContent(audioData, &contentError, &contentErrorLine, &contentErrorColumn);

    if(contentResult == false)
    {
        qDebug() << "[DMC_ServerConnection] Failure parsing audio data: Error reading XML (line " << contentErrorLine << ", column " << contentErrorColumn << "): " << contentError;
        return;
    }

    QDomElement audioElement = doc.firstChildElement("audio-track");
    while(!audioElement.isNull())
    {
        int type = audioElement.attribute("type").toInt();
        QString md5 = audioElement.attribute("md5");
        QString id = audioElement.attribute("id");
        bool repeat = static_cast<bool>(audioElement.attribute("repeat").toInt());
        bool mute = static_cast<bool>(audioElement.attribute("mute").toInt());

        QDomCDATASection urlData = audioElement.firstChild().toCDATASection();
        QUrl url(urlData.data());

        audioElement = audioElement.nextSiblingElement("audio-track");
    }
}

void DMC_ServerConnection::loadBattle()
{
    QFile localPayload("_dmhpayload.txt");
    localPayload.open(QIODevice::ReadOnly);
    QTextStream in(&localPayload);
    in.setCodec("UTF-8");
    QDomDocument doc("DMHelperXML");
    doc.setContent( in.readAll() );
    localPayload.close();

    QDomElement root = doc.documentElement();
    QDomElement battleElement = root.firstChildElement( QString("battle") );

    /*
    _battleDlgModel = new BattleDialogModel(this);
    _battleDlgModel->inputXML(battleElement);

    _battleDlg = new BattleDialog(*_battleDlgModel, this);
    _battleDlg->updateMap();
    _battleDlg->show();
    */
}

void DMC_ServerConnection::stopAudio()
{
    if(!_track)
        return;

    _track->stop();
    _track->deleteLater();
    _track = nullptr;

    emit trackActive(nullptr);
}
