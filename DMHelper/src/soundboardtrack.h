#ifndef SOUNDBOARDTRACK_H
#define SOUNDBOARDTRACK_H

#include <QObject>

class AudioTrack;
class QDomElement;
class QDomDocument;
class QDir;

class SoundboardTrack : public QObject
{
    Q_OBJECT
public:
    explicit SoundboardTrack(AudioTrack* track, int volume = 100, bool mute = false, QObject *parent = nullptr);
    virtual ~SoundboardTrack();

    //Public Interface
    AudioTrack* getTrack() const;
    int getAudioType() const;

    int getVolume() const;
    bool getMute() const;

    QString getTrackName() const;
    QString getTrackDetails() const;

signals:
    void trackLengthChanged(int length);
    void trackPositionChanged(int length);
    void volumeChanged(int volume);
    void muteChanged(bool mute);

public slots:
    void play();
    void stop();
    void setVolume(int volume);
    void setMute(bool mute);

protected:
    AudioTrack* _track;
    int _volume;
    bool _mute;

};

#endif // SOUNDBOARDTRACK_H
