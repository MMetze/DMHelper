#ifndef CAMPAIGNOBJECTBASE_H
#define CAMPAIGNOBJECTBASE_H

#include "dmhobjectbase.h"

class Campaign;

class CampaignObjectBase : public DMHObjectBase
{
    Q_OBJECT
public:

    // Allow full access to the Campaign class

    explicit CampaignObjectBase(QObject *parent = nullptr);
    explicit CampaignObjectBase(const CampaignObjectBase &obj);  // copy constructor
    virtual ~CampaignObjectBase();

    virtual void inputXML(const QDomElement &element, bool isImport);
    virtual void resolveReferences();

    const Campaign* getCampaign() const;
    Campaign* getCampaign();

signals:

public slots:

protected:
    QUuid parseIdString(QString idString, int* intId = nullptr, bool isLocal = false);
    QUuid findUuid(int intId) const;

};

#endif // CAMPAIGNOBJECTBASE_H
