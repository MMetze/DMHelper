#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>
#include <QUrl>
#include <QList>
#include <QMediaPlayer>
#include "audiotrack.h"

class AudioPlayer : public QObject
{
    Q_OBJECT
public:
    explicit AudioPlayer(QObject *parent = nullptr);

    float getVolume();
    qint64 getPosition();
    qint64 getDuration();

    static int MS2SEC(qint64 ms);
    static int MS2MIN(qint64 ms);

    enum State
    {
        Stopped,
        Playing,
        Paused
    };

signals:
    void volumeChanged(float volume);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void trackChanged(AudioTrack* track);
    void stateChanged(AudioPlayer::State state);

public slots:
    void playTrack(AudioTrack* track);
    void play();
    void pause();
    void stop();
    void setVolume(float volume);
    void setPosition(qint64 position);

    void playerPlayUrl(QUrl url);
    void playerStop();

private slots:
    void playerStatusChanged(QMediaPlayer::MediaStatus status);
    void handleSourceChanged(const QUrl &media);
    void handlePositionChanged(qint64 position);
    void handleDurationChanged(qint64 duration);
    void handleVolumeChanged(float volume);
    void handleStateChanged(QMediaPlayer::PlaybackState playerState);

private:
    QMediaPlayer* _player;
    AudioTrack* _currentTrack;
};

#endif // AUDIOPLAYER_H
