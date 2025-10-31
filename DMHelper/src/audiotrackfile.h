#ifndef AUDIOTRACKFILE_H
#define AUDIOTRACKFILE_H

#include "audiotrackurl.h"
#include <QMediaPlayer>

class AudioTrackFile : public AudioTrackUrl
{
    Q_OBJECT
public:
    explicit AudioTrackFile(const QString& trackName = QString(), const QUrl& trackUrl = QUrl(), QObject *parent = nullptr);

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;

    virtual int getAudioType() const override;

    virtual int getTrackStatus() const override;
    virtual bool isPlaying() const override;
    virtual bool isRepeat() const override;
    virtual bool isMuted() const override;
    virtual float getVolume() const override;

public slots:
    virtual void play() override;
    virtual void pause() override;
    virtual void stop() override;
    virtual void setMute(bool mute) override;
    virtual void setVolume(float volume) override;
    virtual void setRepeat(bool repeat) override;

protected slots:
    void handleDurationChanged(qint64 position);
    void handlePositionChanged(qint64 position);
    void handleErrorOccurred(QMediaPlayer::Error error, const QString &errorString);

protected:
    // From CampaignObjectBase
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    QMediaPlayer* _player;
    bool _repeat;
    float _volume;
    bool _mute;

};

#endif // AUDIOTRACKFILE_H
