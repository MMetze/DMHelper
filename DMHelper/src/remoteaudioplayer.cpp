#include "remoteaudioplayer.h"
#include "audiotrack.h"
#include "dmc_constants.h"
#include "dmconstants.h"
#include "audiofactory.h"
#include "audiotrackurl.h"
#include <QDomDocument>
#include <QDebug>

RemoteAudioPlayer::RemoteAudioPlayer(QObject *parent) :
    QObject(parent),
    _tracks()
{
}

RemoteAudioPlayer::~RemoteAudioPlayer()
{
    qDeleteAll(_tracks);
}

void RemoteAudioPlayer::parseAudioString(const QString& audioString)
{
    QString expandedString = QString("<root>") + audioString + QString("</root>");
    QDomDocument doc;
    QString errorMsg;
    int errorLine;
    int errorColumn;
    if(!doc.setContent(expandedString, &errorMsg, &errorLine, &errorColumn))
    {
        qDebug() << "[RemoteAudioPlayer] ERROR identified reading audio data: unable to parse XML at line " << errorLine << ", column " << errorColumn << ": " << errorMsg;
        qDebug() << "[RemoteAudioPlayer] Audio String: " << expandedString;
        return;
    }

    setChecked(false);

    QDomElement rootElement = doc.documentElement();
    QDomElement trackElement = rootElement.firstChildElement("audio-track");
    while(!trackElement.isNull())
    {
        RemoteAudioPlayer_FileWrapper* wrapper = findTrack(QUuid(trackElement.attribute(QString("id"))));
        if(wrapper)
            wrapper->setChecked(true);
        else
            _tracks.append(new RemoteAudioPlayer_FileWrapper(trackElement));

        trackElement = trackElement.nextSiblingElement("audio-track");
    }

    clearUnchecked();
}

void RemoteAudioPlayer::setChecked(bool checked)
{
    for(int i = 0; i < _tracks.count(); ++i)
    {
        if(_tracks.at(i))
            _tracks[i]->setChecked(checked);
    }
}

void RemoteAudioPlayer::clearUnchecked()
{
    for(int i = _tracks.count() - 1; i >= 0; --i)
    {
        if((_tracks.at(i)) && (!_tracks.at(i)->getChecked()))
        {
            RemoteAudioPlayer_FileWrapper* uncheckedFile = _tracks.takeAt(i);
            if(uncheckedFile)
            {
                qDebug() << "[RemoteAudioPlayer] Stopping audio: " << uncheckedFile->getId() << ", status: " << uncheckedFile->getStatus();
                uncheckedFile->deleteLater();
            }
        }
    }
}

RemoteAudioPlayer_FileWrapper* RemoteAudioPlayer::findTrack(const QUuid& id)
{
    for(int i = 0; i < _tracks.count(); ++i)
    {
        if((_tracks.at(i)) && (_tracks.at(i))->getId() == id)
            return _tracks[i];
    }

    return nullptr;
}

RemoteAudioPlayer_FileWrapper::RemoteAudioPlayer_FileWrapper(const QDomElement& element) :
    _track(nullptr),
    _id(element.attribute(QString("id"))),
    _status(DMC_SERVER_CONNECTION_UPLOAD_NOT_STARTED),
    _checked(true)
{
    bool ok = false;
    int type = element.attribute("type").toInt(&ok);
    if(!ok)
        return;

    switch(type)
    {
        case DMHelper::AudioType_File:
            break;
        case DMHelper::AudioType_Syrinscape:
        case DMHelper::AudioType_Youtube:
        {
            QDomCDATASection urlData = element.firstChild().toCDATASection();
            if(urlData.isNull())
                return;

            QUrl url(urlData.data());
            if(url.isEmpty())
                return;

            qDebug() << "[RemoteAudioPlayer] Creating audio: " << url.toString() << " with name: " << url.fileName();
            AudioFactory audioFactory;
            CampaignObjectBase* trackObject = audioFactory.createObject(DMHelper::CampaignType_AudioTrack,
                                                                        type,
                                                                        url.fileName(),
                                                                        false);
            AudioTrackUrl* urlTrack = dynamic_cast<AudioTrackUrl*>(trackObject);
            if(urlTrack)
            {
                if(urlTrack)
                    urlTrack->setUrl(url);
                _track = urlTrack;
                qDebug() << "[Main] Playing track: " << _track << " with id: " << _id;
                _track->play();
            }
            break;
        }
        default:
            return;
    }
}

RemoteAudioPlayer_FileWrapper::~RemoteAudioPlayer_FileWrapper()
{
    delete _track;
}

AudioTrack* RemoteAudioPlayer_FileWrapper::getTrack() const
{
    return _track;
}

void RemoteAudioPlayer_FileWrapper::setTrack(AudioTrack* track)
{
    _track = track;
}

QUuid RemoteAudioPlayer_FileWrapper::getId() const
{
    return _id;
}

void RemoteAudioPlayer_FileWrapper::setId(QUuid id)
{
    _id = id;
}

int RemoteAudioPlayer_FileWrapper::getStatus() const
{
    return _status;
}

void RemoteAudioPlayer_FileWrapper::setStatus(int status)
{
    _status = status;
}

bool RemoteAudioPlayer_FileWrapper::getChecked() const
{
    return _checked;
}

void RemoteAudioPlayer_FileWrapper::setChecked(bool checked)
{
    _checked = checked;
}
