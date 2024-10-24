#ifndef AUDIOTRACK_H
#define AUDIOTRACK_H

#include "campaignobjectbase.h"
#include <QUrl>

class AudioTrack : public CampaignObjectBase
{
    Q_OBJECT
public:
    explicit AudioTrack(const QString& trackName = QString(), QObject *parent = nullptr);
    virtual ~AudioTrack() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;
    virtual int getObjectType() const override;
    virtual QIcon getDefaultIcon() override;

    // Local
    virtual int getAudioType() const = 0;

    virtual QUrl getUrl() const = 0;
    virtual void setUrl(const QUrl& trackUrl) = 0;

    virtual QString getMD5() const;
    virtual void setMD5(const QString& md5);

    virtual bool isPlaying() const;
    virtual bool isRepeat() const;
    virtual bool isMuted() const;
    virtual float getVolume() const;

public slots:
    virtual void play() = 0;
    virtual void stop() = 0;
    virtual void setMute(bool mute) = 0;
    virtual void setVolume(float volume) = 0;
    virtual void setRepeat(bool repeat) = 0;

signals:
    void trackLengthChanged(int length);
    void trackPositionChanged(int length);

protected:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;

    QString _md5;
};

#endif // AUDIOTRACK_H
