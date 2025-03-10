#ifndef SOUNDBOARDTRACKFRAME_H
#define SOUNDBOARDTRACKFRAME_H

#include <QFrame>

class SoundboardTrack;

namespace Ui {
class SoundboardTrackFrame;
}

class SoundboardTrackFrame : public QFrame
{
    Q_OBJECT

public:
    explicit SoundboardTrackFrame(SoundboardTrack* track, QWidget *parent = nullptr);
    ~SoundboardTrackFrame();

    SoundboardTrack* getTrack() const;
    int getAudioType() const;

public slots:
    void setMute(bool mute);
    void parentMuteChanged(bool mute);
    void setTrackLength(int trackLength);
    void setTrackPosition(int trackPosition);
    void setRepeat(bool repeat);

signals:
    void play();
    void stop();
    void muteChanged(bool mute);
    void repeatChanged(bool repeat);
    void removeTrack(SoundboardTrack* track);

protected slots:
    void togglePlay(bool checked);
    void toggleMute();
    void setCurrentMute(bool mute);
    void updateProgress();
    void handleRemove();

private:
    void setTrack(SoundboardTrack* track);

    Ui::SoundboardTrackFrame *ui;

    SoundboardTrack* _track;
    bool _localMute;
    bool _currentMute;
    QString _trackLength;
    QString _trackPosition;
};

#endif // SOUNDBOARDTRACKFRAME_H
