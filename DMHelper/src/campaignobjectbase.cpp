#include "campaignobjectbase.h"
#include "campaign.h"
#include <QDomDocument>
#include <QDomElement>
#include <QDebug>

CampaignObjectBase::CampaignObjectBase(QObject *parent) :
    DMHObjectBase(parent)
{
}

CampaignObjectBase::CampaignObjectBase(const CampaignObjectBase &obj) :
    DMHObjectBase(obj)
{
}

CampaignObjectBase::~CampaignObjectBase()
{
}

void CampaignObjectBase::inputXML(const QDomElement &element, bool isImport)
{
    DMHObjectBase::inputXML(element, isImport);

    if(getID().isNull())
    {
        setID(findUuid(getIntID()));
    }
}

void CampaignObjectBase::resolveReferences()
{
}

const Campaign* CampaignObjectBase::getCampaign() const
{
    const Campaign* result = qobject_cast<const Campaign*>(this);
    if(result)
        return result;

    const CampaignObjectBase* parentObject = qobject_cast<const CampaignObjectBase*>(parent());
    if(parentObject)
        return parentObject->getCampaign();
    else
        return nullptr;
}

Campaign* CampaignObjectBase::getCampaign()
{
    Campaign* result = qobject_cast<Campaign*>(this);
    if(result)
        return result;

    CampaignObjectBase* parentObject = qobject_cast<CampaignObjectBase*>(parent());
    if(parentObject)
        return parentObject->getCampaign();
    else
        return nullptr;
}

QUuid CampaignObjectBase::parseIdString(QString idString, int* intId, bool isLocal)
{
    QUuid result(idString);
    if(result.isNull())
    {
        bool ok = false;
        int tempIntId = idString.toInt(&ok);
        if(ok)
        {
            if(intId)
                *intId = tempIntId;

            if(!isLocal)
                result = findUuid(tempIntId);
        }
    }

    return result;
}

QUuid CampaignObjectBase::findUuid(int intId) const
{
    const Campaign* campaign = getCampaign();
    if(!campaign)
        return QUuid();

    return campaign->getUuidFromIntId(intId);
}
