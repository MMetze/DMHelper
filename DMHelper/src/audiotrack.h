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
    virtual int getObjectType() const override;

    // Local
    virtual int getAudioType() const = 0;

    virtual QUrl getUrl() const = 0;
    virtual void setUrl(const QUrl& trackUrl) = 0;

    virtual QString getMD5() const;
    virtual void setMD5(const QString& md5);

    virtual void play() = 0;
    virtual void stop() = 0;
    virtual void setMute(bool mute) = 0;
    virtual void setVolume(int volume) = 0;

signals:

protected:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;

    QString _md5;
};

#endif // AUDIOTRACK_H
