#ifndef SOUNDBOARDTRACK_H
#define SOUNDBOARDTRACK_H

#include <QFrame>

class AudioTrack;
class AudioPlayer;

namespace Ui {
class SoundboardTrack;
}

class SoundboardTrack : public QFrame
{
    Q_OBJECT

public:
    explicit SoundboardTrack(AudioTrack* track, QWidget *parent = nullptr);
    ~SoundboardTrack();

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
    Ui::SoundboardTrack *ui;

    AudioTrack* _track;
    bool _localMute;
    bool _currentMute;
    QString _trackLength;
    QString _trackPosition;
};

#endif // SOUNDBOARDTRACK_H
