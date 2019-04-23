#ifndef AUDIOPLAYBACKFRAME_H
#define AUDIOPLAYBACKFRAME_H

#include <QFrame>
#include "audioplayer.h"

namespace Ui {
class AudioPlaybackFrame;
}

class AudioTrack;

class AudioPlaybackFrame : public QFrame
{
    Q_OBJECT

public:
    explicit AudioPlaybackFrame(QWidget *parent = nullptr);
    ~AudioPlaybackFrame();

signals:
    void play();
    void pause();
    void positionChanged(qint64 position);
    void volumeChanged(int volume);

public slots:
    void setDuration(qint64 duration);
    void setPosition(qint64 position);
    void trackChanged(AudioTrack* track);
    void stateChanged(AudioPlayer::State state);
    void setVolume(int volume);

protected:
    // from QObject
    virtual bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void togglePlay(bool checked);

private:
    void setPlayerEnabled(bool enabled);

    Ui::AudioPlaybackFrame *ui;

    qint64 _currentDuration;
    qint64 _currentPosition;
    int _currentVolume;
    bool _sliderGrabbed;

};

#endif // AUDIOPLAYBACKFRAME_H
