#ifndef AUDIOTRACKFILE_H
#define AUDIOTRACKFILE_H

#include "audiotrackurl.h"

class QMediaPlayer;

class AudioTrackFile : public AudioTrackUrl
{
    Q_OBJECT
public:
    explicit AudioTrackFile(const QString& trackName = QString(), const QUrl& trackUrl = QUrl(), QObject *parent = nullptr);

    virtual int getAudioType() const override;

    virtual bool isPlaying() const override;
    virtual bool isRepeat() const override;
    virtual bool isMuted() const override;
    virtual float getVolume() const override;

public slots:
    virtual void play() override;
    virtual void stop() override;
    virtual void setMute(bool mute) override;
    virtual void setVolume(float volume) override;
    virtual void setRepeat(bool repeat) override;

protected slots:
    void handleDurationChanged(qint64 position);
    void handlePositionChanged(qint64 position);

protected:
    QMediaPlayer* _player;
    bool _repeat;

};

#endif // AUDIOTRACKFILE_H
