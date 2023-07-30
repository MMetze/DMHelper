#include "audioplayer.h"
#include "audiotrack.h"
#include "dmconstants.h"
#include <QDebug>

//track changed, volume, position, duration signals, show them in the control frame

AudioPlayer::AudioPlayer(QObject *parent) :
    QObject(parent),
    _player(nullptr),
    _playlist(nullptr),
    _currentTrack(nullptr)
{
    _player = new QMediaPlayer(this);

    _playlist = new QMediaPlaylist(this);
    _playlist->setPlaybackMode(QMediaPlaylist::Loop);

    connect(_player, SIGNAL(positionChanged(qint64)), this, SLOT(handlePositionChanged(qint64)));
    connect(_player, SIGNAL(durationChanged(qint64)), this, SLOT(handleDurationChanged(qint64)));
    connect(_player, SIGNAL(volumeChanged(int)), this, SLOT(handleVolumeChanged(int)));
    connect(_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(playerStatusChanged(QMediaPlayer::MediaStatus)));
    connect(_player, SIGNAL(currentMediaChanged(const QMediaContent &)), this, SLOT(handleCurrentMediaChanged(const QMediaContent &)));
    connect(_player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(handleStateChanged(QMediaPlayer::State)));
}

int AudioPlayer::getVolume()
{
    return _player ? _player->volume() : 0;
}

qint64 AudioPlayer::getPosition()
{
    return _player ? _player->position() : 0;
}

qint64 AudioPlayer::getDuration()
{
    return _player ? _player->duration() : 0;
}

int AudioPlayer::MS2SEC(qint64 ms)
{
    return static_cast<int>((ms / 1000) % 60);
}

int AudioPlayer::MS2MIN(qint64 ms)
{
    return static_cast<int>(ms / 60000);
}

void AudioPlayer::playTrack(AudioTrack* track)
{
    if((!_player) || (!_playlist))
        return;

    if(track)
    {
        qDebug() << "[Audio Player] loading " << track->getName() << " from " << track->getUrl();
        if(_currentTrack == track)
        {
            qDebug() << "[Audio Player] track " << track->getName() << " is already playing.";
            return;
        }
    }
    else
    {
        qDebug() << "[Audio Player] loading empty track - stopping music";
    }

    stop();

    _currentTrack = track;
    play();
}

void AudioPlayer::play()
{
    if(_currentTrack)
        //_currentTrack->play(this);
        _currentTrack->play();
}

void AudioPlayer::pause()
{
    _player->pause();
}

void AudioPlayer::stop()
{
    if(_currentTrack)
        _currentTrack->stop();
}

void AudioPlayer::setVolume(int volume)
{
    if(volume != _player->volume())
        _player->setVolume(volume);
}

void AudioPlayer::setPosition(qint64 position)
{
    if(position != _player->position())
        _player->setPosition(position);
}

void AudioPlayer::playerPlayUrl(QUrl url)
{
    _playlist->clear();
    _playlist->addMedia(url);
    _player->setPlaylist(_playlist);
    _player->play();
}

void AudioPlayer::playerStop()
{
    _player->stop();
}

void AudioPlayer::playerStatusChanged(QMediaPlayer::MediaStatus status)
{
    if(!_player)
        return;

    switch(status)
    {
        case QMediaPlayer::UnknownMediaStatus:
        case QMediaPlayer::NoMedia:
            emit trackChanged(nullptr);
            break;
        case QMediaPlayer::LoadingMedia:
            //emit trackChanged(nullptr);
            break;
        case QMediaPlayer::LoadedMedia:
            //_player->play();
            break;
        default:
            break;
    }
}

void AudioPlayer::handleCurrentMediaChanged(const QMediaContent &media)
{
    if(!media.isNull())
        emit trackChanged(_currentTrack);
}

void AudioPlayer::handlePositionChanged(qint64 position)
{
    emit positionChanged(position);
}

void AudioPlayer::handleDurationChanged(qint64 duration)
{
    emit durationChanged(duration);
}

void AudioPlayer::handleVolumeChanged(int volume)
{
    emit volumeChanged(volume);
}

void AudioPlayer::handleStateChanged(QMediaPlayer::State playerState)
{
    AudioPlayer::State state;

    switch(playerState)
    {
        case QMediaPlayer::PlayingState:
            state = AudioPlayer::Playing;
            break;
        case QMediaPlayer::PausedState:
            state = AudioPlayer::Paused;
            break;
        case QMediaPlayer::StoppedState:
        default:
            state = AudioPlayer::Stopped;
            break;
    }

    emit stateChanged(state);
}
