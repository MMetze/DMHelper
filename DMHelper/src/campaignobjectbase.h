#ifndef CAMPAIGNOBJECTBASE_H
#define CAMPAIGNOBJECTBASE_H

#include "dmhobjectbase.h"

class Campaign;
//class QDomDocument;
//class QDomElement;
//class QDir;

class CampaignObjectBase : public DMHObjectBase
{
    Q_OBJECT
public:

    // Allow full access to the Campaign class
//    friend class Campaign;

    explicit CampaignObjectBase(QObject *parent = nullptr);
    explicit CampaignObjectBase(const CampaignObjectBase &obj);  // copy constructor
    virtual ~CampaignObjectBase();

//    virtual void outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory);
//    virtual void inputXML(const QDomElement &element);
//    virtual void postProcessXML(const QDomElement &element);
//    int getID() const;

    const Campaign* getCampaign() const;
    Campaign* getCampaign();

signals:

public slots:

private:

};

#endif // CAMPAIGNOBJECTBASE_H
