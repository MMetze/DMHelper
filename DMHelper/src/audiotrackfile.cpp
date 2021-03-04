#include "audiotrackfile.h"
#include "dmconstants.h"
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QFile>
#include <QMessageBox>
#include <QDebug>

AudioTrackFile::AudioTrackFile(const QString& trackName, const QUrl& trackUrl, QObject *parent) :
    AudioTrackUrl(trackName, trackUrl, parent),
    _player(nullptr),
    _playlist(nullptr),
    _repeat(false)
{
}

AudioTrackFile::~AudioTrackFile()
{
    stop();
}

int AudioTrackFile::getAudioType() const
{
    return DMHelper::AudioType_File;
}

bool AudioTrackFile::isPlaying() const
{
    return ((_player) && (_player->state() == QMediaPlayer::PlayingState));
}

bool AudioTrackFile::isRepeat() const
{
    return ((_playlist) && (_playlist->playbackMode() == QMediaPlaylist::Loop));
}

bool AudioTrackFile::isMuted() const
{
    return ((_player) && (_player->isMuted()));
}

int AudioTrackFile::getVolume() const
{
    return _player ? _player->volume() : 0;
}

void AudioTrackFile::play()
{
    if((_player) || (_playlist))
        return;

    QString fileString = getUrl().toString();
    if(!QFile::exists(fileString))
    {
        QMessageBox::critical(nullptr,
                              QString("DMHelper Audio Track File Not Found"),
                              QString("The audio track could not be found: ") + fileString);
        qDebug() << "[AudioTrackFile] Audio track file not found: " << fileString;
        return;
    }

    _player = new QMediaPlayer(this);
    _playlist = new QMediaPlaylist(this);
    _playlist->setPlaybackMode(_repeat ? QMediaPlaylist::Loop : QMediaPlaylist::CurrentItemOnce);

    connect(_player, &QMediaPlayer::durationChanged, this, &AudioTrackFile::handleDurationChanged);
    connect(_player, &QMediaPlayer::positionChanged, this, &AudioTrackFile::handlePositionChanged);

    _playlist->addMedia(getUrl());
    _player->setPlaylist(_playlist);
    _player->play();

    emit trackStarted(this);
}

void AudioTrackFile::stop()
{
    if((!_player) || (!_playlist))
        return;

    emit trackStopped(this);
    _player->stop();

    _playlist->deleteLater();
    _player->deleteLater();
    _playlist = nullptr;
    _player = nullptr;
}

void AudioTrackFile::setMute(bool mute)
{
    if((!_player) || (_player->isMuted() == mute))
        return;

    _player->setMuted(mute);
    emit muteChanged(mute);
}

void AudioTrackFile::setVolume(int volume)
{
    if((!_player) && (_player->volume() == volume))
        return;

    _player->setVolume(volume);
    emit volumeChanged(volume);
}

void AudioTrackFile::setRepeat(bool repeat)
{
    if((!_playlist) || (_repeat != repeat))
        return;

    _playlist->setPlaybackMode(_repeat ? QMediaPlaylist::Loop : QMediaPlaylist::CurrentItemOnce);
    emit repeatChanged(repeat);
}

void AudioTrackFile::handleDurationChanged(qint64 position)
{
    emit trackLengthChanged(position / 1000);
}

void AudioTrackFile::handlePositionChanged(qint64 position)
{
    emit trackPositionChanged(position / 1000);
}

