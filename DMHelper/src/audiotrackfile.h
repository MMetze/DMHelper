#ifndef AUDIOTRACKFILE_H
#define AUDIOTRACKFILE_H

#include "audiotrackurl.h"

class QMediaPlayer;
class QMediaPlaylist;

class AudioTrackFile : public AudioTrackUrl
{
    Q_OBJECT
public:
    explicit AudioTrackFile(const QString& trackName = QString(), const QUrl& trackUrl = QUrl(), QObject *parent = nullptr);

    virtual int getAudioType() const override;

    virtual void play() override;
    virtual void stop() override;
    virtual void setMute(bool mute) override;
    virtual void setVolume(int volume) override;

signals:

protected:
    QMediaPlayer* _player;
    QMediaPlaylist* _playlist;

};

#endif // AUDIOTRACKFILE_H
