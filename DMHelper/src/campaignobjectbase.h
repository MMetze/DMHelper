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

//    virtual void outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport);
    virtual void inputXML(const QDomElement &element, bool isImport);
//    virtual void postProcessXML(const QDomElement &element, bool isImport);
//    int getID() const;

    const Campaign* getCampaign() const;
    Campaign* getCampaign();

signals:

public slots:

protected:
    QUuid parseIdString(QString idString, int* intId = nullptr, bool isLocal = false);
    QUuid findUuid(int intId) const;

};

#endif // CAMPAIGNOBJECTBASE_H
