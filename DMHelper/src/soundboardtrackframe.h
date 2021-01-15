#ifndef SOUNDBOARDTRACKFRAME_H
#define SOUNDBOARDTRACKFRAME_H

#include <QFrame>

class AudioTrack;
class AudioPlayer;

namespace Ui {
class SoundboardTrackFrame;
}

class SoundboardTrackFrame : public QFrame
{
    Q_OBJECT

public:
    explicit SoundboardTrackFrame(AudioTrack* track, QWidget *parent = nullptr);
    ~SoundboardTrackFrame();

    void setTrack(AudioTrack* track);
    AudioTrack* getTrack() const;

public slots:
    void setMute(bool mute);
    void parentMuteChanged(bool mute);
    void setTrackLength(int trackLength);
    void setTrackPosition(int trackPosition);
    void setRepeat(bool repeat);

signals:
    void play(AudioPlayer* player);
    void stop();
    void muteChanged(bool mute);
    void volumeChanged(int volume);
    void repeatChanged(bool repeat);
    void removeTrack(AudioTrack* track);

protected slots:
    void togglePlay(bool checked);
    void toggleMute();
    void setCurrentMute(bool mute);
    void updateProgress();
    void handleRemove();

private:
    Ui::SoundboardTrackFrame *ui;

    AudioTrack* _track;
    bool _localMute;
    bool _currentMute;
    QString _trackLength;
    QString _trackPosition;
};

#endif // SOUNDBOARDTRACKFRAME_H
