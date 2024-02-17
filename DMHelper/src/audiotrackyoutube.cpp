#include "audiotrackyoutube.h"
#include "dmconstants.h"
#include "dmversion.h"
#include <QUrlQuery>
#include <QMessageBox>
#include <QDomDocument>
#include <QDomElement>
#include <QIcon>

const int AUDIOTRACKYOUTUBE_STOPCALLCOMPLETE = 0x01;
const int AUDIOTRACKYOUTUBE_STOPCALLCONFIRMED = 0x02;
const int AUDIOTRACKYOUTUBE_STOPCOMPLETE = AUDIOTRACKYOUTUBE_STOPCALLCOMPLETE | AUDIOTRACKYOUTUBE_STOPCALLCONFIRMED;

void youtubeEventCallback(const struct libvlc_event_t *p_event, void *p_data);

AudioTrackYoutube::AudioTrackYoutube(const QString& trackName, const QUrl& trackUrl, QObject *parent) :
    AudioTrackUrl(trackName, trackUrl, parent),
    _manager(nullptr),
    _urlString(),
    _vlcPlayer(nullptr),
    _stopStatus(0),
    _volume(100),
    _timerId(0),
    _repeat(true),
    _mute(false)
{
}

AudioTrackYoutube::~AudioTrackYoutube()
{
    if(_timerId)
        killTimer(_timerId);

    AudioTrackYoutube::stop();
}

void AudioTrackYoutube::inputXML(const QDomElement &element, bool isImport)
{
    _volume = element.attribute("volume", QString::number(100)).toInt();
    _repeat = static_cast<bool>(element.attribute("repeat", QString::number(1)).toInt());
    _mute = static_cast<bool>(element.attribute("mute", QString::number(0)).toInt());

    AudioTrackUrl::inputXML(element, isImport);
}

void AudioTrackYoutube::copyValues(const CampaignObjectBase* other)
{
    const AudioTrackYoutube* otherAudioTrack = dynamic_cast<const AudioTrackYoutube*>(other);
    if(!otherAudioTrack)
        return;

    _volume = otherAudioTrack->_volume;
    _repeat = otherAudioTrack->_repeat;
    _mute = otherAudioTrack->_mute;

    AudioTrackUrl::copyValues(other);
}

QIcon AudioTrackYoutube::getDefaultIcon()
{
    return QIcon(":/img/data/icon_playerswindow.png");
}

int AudioTrackYoutube::getAudioType() const
{
    return DMHelper::AudioType_Youtube;
}

void AudioTrackYoutube::eventCallback(const struct libvlc_event_t *p_event)
{
    if(!p_event)
        return;

    if(p_event->type == libvlc_MediaPlayerStopped)
    {
        internalStopCheck(AUDIOTRACKYOUTUBE_STOPCALLCONFIRMED);
    }
    if(p_event->type == libvlc_MediaPlayerPlaying)
    {
        libvlc_time_t length_full = libvlc_media_player_get_length(_vlcPlayer);
        emit trackLengthChanged(length_full / 1000);
    }
}

bool AudioTrackYoutube::isPlaying() const
{
    return((_vlcPlayer) && (_stopStatus < AUDIOTRACKYOUTUBE_STOPCALLCOMPLETE));
}

bool AudioTrackYoutube::isRepeat() const
{
    return _repeat;
}

bool AudioTrackYoutube::isMuted() const
{
    return _mute;
}

float AudioTrackYoutube::getVolume() const
{
    return static_cast<float>(_volume) / 100.f;
}

void AudioTrackYoutube::play()
{
    // Check if the track is already playing
    if(isPlaying())
        return;

    if(_urlString.isEmpty())
        findDirectUrl(extractYoutubeIDFromUrl()); //findDirectUrl(QString("9bMTK0ml9ZI"));
    else
        playDirectUrl();
}

void AudioTrackYoutube::stop()
{
    if(!AudioTrackYoutube::isPlaying())
        return;

    _stopStatus = 0;
    libvlc_media_player_stop_async(_vlcPlayer);
    internalStopCheck(AUDIOTRACKYOUTUBE_STOPCALLCOMPLETE);
}

void AudioTrackYoutube::setMute(bool mute)
{
    if(mute == _mute)
        return;

    _mute = mute;

    if(isPlaying())
    {
        //_lastVolume = libvlc_audio_get_volume(_vlcPlayer);
        if(_mute)
            libvlc_audio_set_volume(_vlcPlayer, 0);
        else
            libvlc_audio_set_volume(_vlcPlayer, _volume);
    }

    emit dirty();
}

void AudioTrackYoutube::setVolume(float volume)
{
    if(volume == _volume)
        return;

    _volume = static_cast<int>(volume * 100.f);

    if(isPlaying())
        libvlc_audio_set_volume(_vlcPlayer, _volume);

    emit dirty();
}

void AudioTrackYoutube::setRepeat(bool repeat)
{
    if(repeat == _repeat)
        return;

    _repeat = repeat;
    emit dirty();
}

void AudioTrackYoutube::urlRequestFinished(QNetworkReply *reply)
{
    if(!reply)
    {
        QMessageBox::critical(nullptr,
        QString("DMHelper Audio Error"),
        QString("An unexpected and unknown error was encountered trying to find the requested YouTube video for playback!"));
        qDebug() << "[AudioTrackYoutube] ERROR identified in reply, unexpected null pointer reply received!";
        return;
    }

    if(!isPlaying())
    {
        if(reply->error() != QNetworkReply::NoError)
        {
            if(reply->error() == QNetworkReply::HostNotFoundError)
            {
                QMessageBox::critical(nullptr,
                                      QString("DMHelper Audio Error"),
                                      QString("A network error was encountered trying to find the requested YouTube video. It was not possible to reach the server!"));
            }
            else
            {
                QMessageBox::critical(nullptr,
                                      QString("DMHelper Audio Error"),
                                      QString("A network error was encountered trying to find the requested YouTube video:") + QChar::LineFeed + QChar::LineFeed + reply->errorString());
            }

            qDebug() << "[AudioTrackYoutube] ERROR identified in network reply: " << QString::number(reply->error()) << ", Error string " << reply->errorString();
        }
        else
        {
            QByteArray bytes = reply->readAll();
            qDebug() << "[AudioTrackYoutube] Request received; payload " << bytes.size() << " bytes";

            handleReplyDirect(bytes);
        }
    }

    reply->deleteLater();
}

void AudioTrackYoutube::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("volume", QString::number(_volume));
    element.setAttribute("repeat", QString::number(_repeat));
    element.setAttribute("mute", QString::number(_mute));

    AudioTrackUrl::internalOutputXML(doc, element, targetDirectory, isExport);
}

void AudioTrackYoutube::findDirectUrl(const QString& youtubeId)
{
    if(isPlaying())
        return;

    if(!_manager)
    {
        _manager = new QNetworkAccessManager(this);
        connect(_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(urlRequestFinished(QNetworkReply*)));
    }

    QString getString("https://api.dmhh.net/youtube?id=");
    getString.append(youtubeId);
    getString.append("&version=");
    getString.append(QString("%1.%2").arg(DMHelper::DMHELPER_MAJOR_VERSION)
                                   .arg(DMHelper::DMHELPER_MINOR_VERSION));
    if(DMHelper::DMHELPER_ENGINEERING_VERSION > 0)
        getString.append("&debug=true");

    // Request format: https://api.dmhh.net/youtube?id=t2nhQRYUGy0&version=2.0&debug=true
    QUrl serviceUrl = QUrl(getString);
    QNetworkRequest request(serviceUrl);
    qDebug() << "[AudioTrackYoutube] Asked for youtube direct link. Request: " << getString;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    qDebug() << "[AudioTrackYoutube] Asked for youtube direct link. Request: " << serviceUrl;
    _manager->get(request);
}

void AudioTrackYoutube::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    if((!_vlcPlayer) || (_stopStatus == AUDIOTRACKYOUTUBE_STOPCOMPLETE))
    {
        if(_timerId)
        {
            killTimer(_timerId);
            _timerId = 0;
        }

        return;
    }

    libvlc_time_t currentTime = libvlc_media_player_get_time(_vlcPlayer);
    emit trackPositionChanged(currentTime / 1000);
}

bool AudioTrackYoutube::handleReplyDirect(const QByteArray& data)
{
    if(isPlaying())
        return false;

    QDomDocument doc;
    QString errorMsg;
    int errorLine;
    int errorColumn;
    if(!doc.setContent(data, &errorMsg, &errorLine, &errorColumn))
    {
        qDebug() << "[AudioTrackYoutube] ERROR identified reading data: unable to parse network reply XML at line " << errorLine << ", column " << errorColumn << ": " << errorMsg;
        qDebug() << "[AudioTrackYoutube] Data: " << data;
        return false;
    }

    QDomElement root = doc.documentElement();
    if(root.isNull())
    {
        qDebug() << "[AudioTrackYoutube] ERROR identified reading data: unable to find root element: " << doc.toString();
        return false;
    }

    QDomElement formats = root.firstChildElement(QString("adaptiveFormats"));
    if(formats.isNull())
    {
        qDebug() << "[AudioTrackYoutube] ERROR identified reading data: unable to find adaptiveFormats element: " << doc.toString();
        return false;
    }

    QDomElement format = formats.firstChildElement(QString("adaptiveFormat"));
    while(!format.isNull())
    {
        QDomElement mimeElement = format.firstChildElement(QString("mimeType"));
        QString mimeString = mimeElement.text();
        if(mimeString.left(9) == QString("audio/mp4"))
        {
            QDomElement urlElement = format.firstChildElement(QString("url"));
            if(!urlElement.isNull())
            {
                _urlString = urlElement.text();
                playDirectUrl();
                return true;
            }
        }
        format = format.nextSiblingElement(QString("adaptiveFormat"));
    }

    return false;
}

void AudioTrackYoutube::playDirectUrl()
{
    if(isPlaying())
        return;

#if defined(Q_OS_WIN64) || defined(Q_OS_MAC)
    libvlc_media_t *vlcMedia = libvlc_media_new_location(_urlString.toUtf8().constData());
#else
    libvlc_media_t *vlcMedia = libvlc_media_new_location(DMH_VLC::vlcInstance(), _urlString.toUtf8().constData());
#endif

#if defined(Q_OS_WIN64) || defined(Q_OS_MAC)
    _vlcPlayer = libvlc_media_player_new_from_media(DMH_VLC::vlcInstance(), vlcMedia);
#else
    _vlcPlayer = libvlc_media_player_new_from_media(vlcMedia);
#endif
    if(!_vlcPlayer)
        return;

    libvlc_event_manager_t* eventManager = libvlc_media_player_event_manager(_vlcPlayer);
    if(eventManager)
    {
        libvlc_event_attach(eventManager, libvlc_MediaPlayerOpening, youtubeEventCallback, this);
        libvlc_event_attach(eventManager, libvlc_MediaPlayerBuffering, youtubeEventCallback, this);
        libvlc_event_attach(eventManager, libvlc_MediaPlayerPlaying, youtubeEventCallback, this);
        libvlc_event_attach(eventManager, libvlc_MediaPlayerPaused, youtubeEventCallback, this);
        libvlc_event_attach(eventManager, libvlc_MediaPlayerStopped, youtubeEventCallback, this);
    }

    // Start playback
    if(_mute)
        libvlc_audio_set_volume(_vlcPlayer, 0);
    else
        libvlc_audio_set_volume(_vlcPlayer, _volume);
    libvlc_media_player_play(_vlcPlayer);

    if(_timerId == 0)
        _timerId = startTimer(500);
}

void AudioTrackYoutube::internalStopCheck(int status)
{
    _stopStatus |= status;

    qDebug() << "[AudioTrackYoutube] Internal Stop Check called with status " << status << ", overall status: " << _stopStatus;

    if((_stopStatus == AUDIOTRACKYOUTUBE_STOPCALLCONFIRMED) && (_repeat))
    {
        qDebug() << "[AudioTrackYoutube] Internal Stop Check: Audio ended, restarting playback";
        _stopStatus = 0;
        libvlc_media_player_release(_vlcPlayer);
        _vlcPlayer = nullptr;
        playDirectUrl();
        return;
    }

    if(_stopStatus != AUDIOTRACKYOUTUBE_STOPCOMPLETE)
        return;

    libvlc_media_player_release(_vlcPlayer);
    _vlcPlayer = nullptr;
}

QString AudioTrackYoutube::extractYoutubeIDFromUrl()
{
    if(_url.hasQuery())
    {
        QString urlQuery = _url.query();
        if(urlQuery.left(2) == QString("v="))
        {
            return urlQuery.remove(0, 2);
        }
    }

    return _url.toString();
}


/**
 * Callback function notification
 * \param p_event the event triggering the callback
 */
void youtubeEventCallback(const struct libvlc_event_t *p_event, void *p_data)
{
    if(!p_data)
        return;

    AudioTrackYoutube* player = static_cast<AudioTrackYoutube*>(p_data);
    player->eventCallback(p_event);
}
