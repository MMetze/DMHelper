#ifndef AUDIOTRACK_H
#define AUDIOTRACK_H

#include "campaignobjectbase.h"
#include <QUrl>

class AudioTrack : public CampaignObjectBase
{
    Q_OBJECT
public:
    explicit AudioTrack(const QString& trackName, const QUrl& trackUrl, QObject *parent = 0);
    explicit AudioTrack(QDomElement &element, QObject *parent = 0);
    explicit AudioTrack(const AudioTrack &obj);  // copy constructor
    ~AudioTrack();

    // From CampaignObjectBase
    virtual void outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory);
    virtual void inputXML(const QDomElement &element);
    virtual void postProcessXML(const QDomElement &element);

    // Local
    virtual QString getName() const;
    virtual void setName(const QString& trackName);

    virtual QUrl getUrl() const;
    virtual void setUrl(const QUrl& trackUrl);

    virtual QString getMD5() const;
    virtual void setMD5(const QString& md5);

protected:
    QString _name;
    QUrl _url;
    QString _md5;
};

#endif // AUDIOTRACK_H
