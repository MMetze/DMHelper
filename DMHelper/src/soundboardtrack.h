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

signals:
    void play(AudioPlayer* player);
    void stop();
    void muteChanged(bool mute);
    void volumeChanged(int volume);

protected slots:
    void togglePlay(bool checked);
    void toggleMute();
    void setCurrentMute(bool mute);

private:
    Ui::SoundboardTrack *ui;

    AudioTrack* _track;
    bool _localMute;
    bool _currentMute;
};

#endif // SOUNDBOARDTRACK_H
