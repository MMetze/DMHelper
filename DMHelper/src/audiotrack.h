#ifndef AUDIOTRACK_H
#define AUDIOTRACK_H

#include "campaignobjectbase.h"
#include <QUrl>

class AudioTrack : public CampaignObjectBase
{
    Q_OBJECT
public:
    explicit AudioTrack(const QString& trackName = QString(), const QUrl& trackUrl = QUrl(), QObject *parent = nullptr);
//    explicit AudioTrack(QDomElement &element, bool isImport, QObject *parent = nullptr);
//    explicit AudioTrack(const AudioTrack &obj);  // copy constructor
    virtual ~AudioTrack() override;

    // From CampaignObjectBase
//    virtual void outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport) override;
    virtual void inputXML(const QDomElement &element, bool isImport) override;
//    virtual void postProcessXML(const QDomElement &element, bool isImport) override;

    // Local
//    virtual QString getName() const;
//    virtual void setName(const QString& trackName);

    virtual QUrl getUrl() const;
    virtual void setUrl(const QUrl& trackUrl);

    virtual QString getMD5() const;
    virtual void setMD5(const QString& md5);

    virtual int getObjectType() const override;
    virtual int getAudioType() const;

signals:
    //void changed();
    //void dirty();

protected:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;

//    QString _name;
    QUrl _url;
    QString _md5;
};

#endif // AUDIOTRACK_H
