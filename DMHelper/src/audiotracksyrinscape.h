#ifndef AUDIOTRACKSYRINSCAPE_H
#define AUDIOTRACKSYRINSCAPE_H

#include "audiotrackurl.h"

class AudioTrackSyrinscape : public AudioTrackUrl
{
    Q_OBJECT
public:
    explicit AudioTrackSyrinscape(const QString& trackName = QString(), const QUrl& trackUrl = QUrl(), QObject *parent = nullptr);

    virtual int getAudioType() const override;

public slots:
    virtual void play() override;
    virtual void stop() override;
    virtual void setMute(bool mute) override;
    virtual void setVolume(int volume) override;
    virtual void setRepeat(bool repeat) override;

signals:

};

#endif // AUDIOTRACKSYRINSCAPE_H
