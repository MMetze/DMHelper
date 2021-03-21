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
    virtual ~AudioTrackFile() override;

    virtual int getAudioType() const override;

    virtual QString getFileName() const override;
    virtual void setFileName(const QString& newFileName) override;

    virtual bool isPlaying() const override;
    virtual bool isRepeat() const override;
    virtual bool isMuted() const override;
    virtual int getVolume() const override;

public slots:
    virtual void play() override;
    virtual void stop() override;
    virtual void setMute(bool mute) override;
    virtual void setVolume(int volume) override;
    virtual void setRepeat(bool repeat) override;

signals:

protected slots:
    void handleDurationChanged(qint64 position);
    void handlePositionChanged(qint64 position);

protected:
    QMediaPlayer* _player;
    QMediaPlaylist* _playlist;
    bool _mute;
    bool _repeat;
};

#endif // AUDIOTRACKFILE_H
