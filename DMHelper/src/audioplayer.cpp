#include "audioplayer.h"
#include "audiotrack.h"
#include <QAudioOutput>
#include <QMediaDevices>
#include <QDebug>

//track changed, volume, position, duration signals, show them in the control frame

AudioPlayer::AudioPlayer(QObject *parent) :
    QObject(parent),
    _player(nullptr),
    _currentTrack(nullptr)
{
    _player = new QMediaPlayer(this);
    _player->setLoops(QMediaPlayer::Infinite);

    QAudioOutput audioOutput;
    _player->setAudioOutput(&audioOutput);

    connect(_player, &QMediaPlayer::positionChanged, this, &AudioPlayer::handlePositionChanged);
    connect(_player, &QMediaPlayer::durationChanged, this, &AudioPlayer::handleDurationChanged);
    connect(&audioOutput, &QAudioOutput::volumeChanged, this, &AudioPlayer::handleVolumeChanged);
    connect(_player, &QMediaPlayer::mediaStatusChanged, this, &AudioPlayer::playerStatusChanged);
    connect(_player, &QMediaPlayer::sourceChanged, this, &AudioPlayer::handleSourceChanged);
    connect(_player, &QMediaPlayer::playbackStateChanged, this, &AudioPlayer::handleStateChanged);
}

float AudioPlayer::getVolume()
{
    if((!_player) || (!_player->audioOutput()))
        return 0;

    return _player->audioOutput()->volume();
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
    if(!_player)
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

void AudioPlayer::setVolume(float volume)
{
    if((!_player) || (!_player->audioOutput()))
        return;

    if(volume != _player->audioOutput()->volume())
        _player->audioOutput()->setVolume(volume);
}

void AudioPlayer::setPosition(qint64 position)
{
    if(position != _player->position())
        _player->setPosition(position);
}

void AudioPlayer::playerPlayUrl(QUrl url)
{
    _player->setSource(url);
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

void AudioPlayer::handleSourceChanged(const QUrl &media)
{
    if(media.isValid())
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

void AudioPlayer::handleVolumeChanged(float volume)
{
    emit volumeChanged(volume);
}

void AudioPlayer::handleStateChanged(QMediaPlayer::PlaybackState playerState)
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
