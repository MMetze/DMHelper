#ifndef AUDIOTRACK_H
#define AUDIOTRACK_H

#include "campaignobjectbase.h"
#include <QUrl>

class AudioTrack : public CampaignObjectBase
{
    Q_OBJECT
public:
    explicit AudioTrack(const QString& trackName, const QUrl& trackUrl, QObject *parent = nullptr);
    explicit AudioTrack(QDomElement &element, bool isImport, QObject *parent = nullptr);
    explicit AudioTrack(const AudioTrack &obj);  // copy constructor
    ~AudioTrack();

    // From CampaignObjectBase
    virtual void outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport);
    virtual void inputXML(const QDomElement &element, bool isImport);
    virtual void postProcessXML(const QDomElement &element, bool isImport);

    // Local
    virtual QString getName() const;
    virtual void setName(const QString& trackName);

    virtual QUrl getUrl() const;
    virtual void setUrl(const QUrl& trackUrl);

    virtual QString getMD5() const;
    virtual void setMD5(const QString& md5);

signals:
    void changed();
    void dirty();

protected:
    QString _name;
    QUrl _url;
    QString _md5;
};

#endif // AUDIOTRACK_H
