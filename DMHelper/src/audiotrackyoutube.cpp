#include "audiotrackyoutube.h"
#include "dmconstants.h"
#include "dmversion.h"
#include <QUrlQuery>
#include <QMessageBox>
#include <QDomDocument>
#include <QDomElement>

const int stopCallComplete = 0x01;
const int stopConfirmed = 0x02;
const int stopComplete = stopCallComplete | stopConfirmed;

void youtubeEventCallback(const struct libvlc_event_t *p_event, void *p_data);

AudioTrackYoutube::AudioTrackYoutube(const QString& trackName, const QUrl& trackUrl, QObject *parent) :
    AudioTrackUrl(trackName, trackUrl, parent),
    _manager(nullptr),
    _urlString(),
    _vlcInstance(nullptr),
    _vlcListPlayer(nullptr),
    _vlcPlayer(nullptr),
    _stopStatus(0),
    _lastVolume(100),
    _timerId(0),
    _repeat(false)
{
}

AudioTrackYoutube::~AudioTrackYoutube()
{
    if(_timerId)
        killTimer(_timerId);

    stop();
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
        internalStopCheck(stopConfirmed);
    }
    if(p_event->type == libvlc_MediaPlayerPlaying)
    {
        libvlc_time_t length_full = libvlc_media_player_get_length(_vlcPlayer);
        emit trackLengthChanged(length_full / 1000);
    }
}

void AudioTrackYoutube::play()
{
    // Check if the track is already playing
    if((_vlcInstance) || (_vlcListPlayer) || (_vlcPlayer))
        return;

    if(_urlString.isEmpty())
        findDirectUrl(extractYoutubeIDFromUrl()); //findDirectUrl(QString("9bMTK0ml9ZI"));
    else
        playDirectUrl();
}

void AudioTrackYoutube::stop()
{
    if((!_vlcInstance) || (!_vlcListPlayer) || (!_vlcPlayer))
        return;

    _stopStatus = 0;
    libvlc_media_list_player_stop(_vlcListPlayer);
    internalStopCheck(stopCallComplete);

    /*
    libvlc_media_list_player_stop(_vlcListPlayer);


    libvlc_media_list_player_release(_vlcListPlayer);
    _vlcListPlayer = nullptr;
    if(_vlcInstance)
    {
        libvlc_release(_vlcInstance);
        _vlcInstance = nullptr;
    }
    */

}

void AudioTrackYoutube::setMute(bool mute)
{
    if((!_vlcInstance) || (!_vlcListPlayer) || (!_vlcPlayer))
        return;

    if(mute)
    {
        _lastVolume = libvlc_audio_get_volume(_vlcPlayer);
        libvlc_audio_set_volume(_vlcPlayer, 0);
    }
    else
    {
        libvlc_audio_set_volume(_vlcPlayer, _lastVolume);
    }
}

void AudioTrackYoutube::setVolume(int volume)
{
    if((!_vlcInstance) || (!_vlcListPlayer) || (!_vlcPlayer))
        return;

    libvlc_audio_set_volume(_vlcPlayer, volume);
}

void AudioTrackYoutube::setRepeat(bool repeat)
{
    if(repeat == _repeat)
        return;

    _repeat = repeat;

    if(!_vlcListPlayer)
        return;

    libvlc_media_list_player_set_playback_mode(_vlcListPlayer, _repeat ? libvlc_playback_mode_loop : libvlc_playback_mode_default);
}

void AudioTrackYoutube::urlRequestFinished(QNetworkReply *reply)
{
    if((!_vlcInstance) && (!_vlcListPlayer) && (!_vlcPlayer))
    {
        if(!reply)
        {
            QMessageBox::critical(nullptr,
                                  QString("DM Helper Audio Error"),
                                  QString("An unexpected and unknown error was encountered trying to find the requested YouTube video for playback!"));
            qDebug() << "[AudioTrackYoutube] ERROR identified in reply, unexpected null pointer reply received!";
        }
        else if(reply->error() != QNetworkReply::NoError)
        {
            if(reply->error() == QNetworkReply::HostNotFoundError)
            {
                QMessageBox::critical(nullptr,
                                      QString("DM Helper Audio Error"),
                                      QString("A network error was encountered trying to find the requested YouTube video. It was not possible to reach the server!"));
            }
            else
            {
                QMessageBox::critical(nullptr,
                                      QString("DM Helper Audio Error"),
                                      QString("A network error was encountered trying to find the requested YouTube video:") + QChar::LineFeed + QChar::LineFeed + reply->errorString());
            }

            qDebug() << "[AudioTrackYoutube] ERROR identified in network reply: " << QString::number(reply->error()) << ", Error string " << reply->errorString();
        }
        else
        {
            QByteArray bytes = reply->readAll();
            qDebug() << "[AudioTrackYoutube] Request received; payload " << bytes.size() << " bytes";
            //qDebug() << "[AudioTrackYoutube] Payload contents: " << QString(bytes.left(20000));

            handleReplyDirect(bytes);
        }
    }

    reply->deleteLater();
}

void AudioTrackYoutube::findDirectUrl(const QString& youtubeId)
{
    if((_vlcInstance) || (_vlcListPlayer) || (_vlcPlayer))
        return;

    if(!_manager)
        _manager = new QNetworkAccessManager(this);

    connect(_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(urlRequestFinished(QNetworkReply*)));

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

    if(!_vlcPlayer)
        return;

    libvlc_time_t currentTime = libvlc_media_player_get_time(_vlcPlayer);
    emit trackPositionChanged(currentTime / 1000);
}

bool AudioTrackYoutube::handleReplyDirect(const QByteArray& data)
{
    if((_vlcInstance) || (_vlcListPlayer) || (_vlcPlayer))
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
    if((_vlcInstance) || (_vlcListPlayer) || (_vlcPlayer))
        return;

    _vlcInstance = libvlc_new(0, nullptr);
    libvlc_media_list_t *vlcMediaList = libvlc_media_list_new(_vlcInstance);

    libvlc_media_t *vlcMedia = libvlc_media_new_location(_vlcInstance, _urlString.toUtf8().constData());
    libvlc_media_list_add_media(vlcMediaList, vlcMedia);
    libvlc_media_release(vlcMedia);

    _vlcListPlayer = libvlc_media_list_player_new(_vlcInstance);
    if(!_vlcListPlayer)
        return;

    _vlcPlayer = libvlc_media_player_new(_vlcInstance);
    if(!_vlcPlayer)
        return;

    libvlc_media_list_player_set_media_list(_vlcListPlayer, vlcMediaList);
    libvlc_media_list_player_set_media_player(_vlcListPlayer, _vlcPlayer);
    libvlc_media_list_player_set_playback_mode(_vlcListPlayer, _repeat ? libvlc_playback_mode_loop : libvlc_playback_mode_default);

    libvlc_event_manager_t* eventManager = libvlc_media_player_event_manager(_vlcPlayer);
    if(eventManager)
    {
        //libvlc_event_attach(eventManager, libvlc_MediaPlayerOpening, playerEventCallback, this);
        //libvlc_event_attach(eventManager, libvlc_MediaPlayerBuffering, playerEventCallback, this);
        libvlc_event_attach(eventManager, libvlc_MediaPlayerPlaying, youtubeEventCallback, this);
        //libvlc_event_attach(eventManager, libvlc_MediaPlayerPaused, playerEventCallback, this);
        libvlc_event_attach(eventManager, libvlc_MediaPlayerStopped, youtubeEventCallback, this);
    }

    libvlc_media_list_player_play(_vlcListPlayer);

    if(_timerId == 0)
        _timerId = startTimer(500);
}

void AudioTrackYoutube::internalStopCheck(int status)
{
    _stopStatus |= status;

    qDebug() << "[VideoPlayer] Internal Stop Check called with status " << status << ", overall status: " << _stopStatus;

    if(_stopStatus != stopComplete)
        return;

    libvlc_media_list_player_release(_vlcListPlayer);
    _vlcListPlayer = nullptr;
    libvlc_media_player_release(_vlcPlayer);
    _vlcPlayer = nullptr;
    libvlc_release(_vlcInstance);
    _vlcInstance = nullptr;
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