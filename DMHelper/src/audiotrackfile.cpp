#include "audiotrackfile.h"
#include "dmconstants.h"
#include <QMediaPlayer>
#include <QMediaPlaylist>

AudioTrackFile::AudioTrackFile(const QString& trackName, const QUrl& trackUrl, QObject *parent) :
    AudioTrackUrl(trackName, trackUrl, parent),
    _player(nullptr),
    _playlist(nullptr),
    _repeat(false)
{
}

int AudioTrackFile::getAudioType() const
{
    return DMHelper::AudioType_File;
}

void AudioTrackFile::play()
{
    if((_player) || (_playlist))
        return;

    _player = new QMediaPlayer(this);
    _playlist = new QMediaPlaylist(this);
    _playlist->setPlaybackMode(_repeat ? QMediaPlaylist::Loop : QMediaPlaylist::CurrentItemOnce);

    connect(_player, &QMediaPlayer::durationChanged, this, &AudioTrackFile::handleDurationChanged);
    connect(_player, &QMediaPlayer::positionChanged, this, &AudioTrackFile::handlePositionChanged);

    _playlist->addMedia(getUrl());
    _player->setPlaylist(_playlist);
    _player->play();
}

void AudioTrackFile::stop()
{
    if((!_player) || (!_playlist))
        return;

    _player->stop();

    _playlist->deleteLater();
    _player->deleteLater();
    _playlist = nullptr;
    _player = nullptr;
}

void AudioTrackFile::setMute(bool mute)
{
    if(_player)
        _player->setMuted(mute);
}

void AudioTrackFile::setVolume(int volume)
{
    if(_player)
        _player->setVolume(volume);
}

void AudioTrackFile::setRepeat(bool repeat)
{
    if(_repeat != repeat)
        return;

    if(!_playlist)
        return;

    _playlist->setPlaybackMode(_repeat ? QMediaPlaylist::Loop : QMediaPlaylist::CurrentItemOnce);
}

void AudioTrackFile::handleDurationChanged(qint64 position)
{
    emit trackLengthChanged(position / 1000);
}

void AudioTrackFile::handlePositionChanged(qint64 position)
{
    emit trackPositionChanged(position / 1000);
}

