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
//    qDebug() << "[CampaignObjectBase] WARNING: Object copied - this is a highly questionable action leading to duplicate IDs! ID: " << _id;
}

CampaignObjectBase::~CampaignObjectBase()
{
}

/*
void CampaignObjectBase::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory)
{
}

void CampaignObjectBase::inputXML(const QDomElement &element)
{
}

void CampaignObjectBase::postProcessXML(const QDomElement &element)
{
}

int CampaignObjectBase::getID() const
{
    return _id;
}
*/

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

/*
void CampaignObjectBase::setBaseId(int baseId)
{
    _id_global = baseId;
}

void CampaignObjectBase::resetBaseId()
{
    _id_global = DMHelper::GLOBAL_BASE_ID;
}

int CampaignObjectBase::createId()
{
    return _id_global++;
}
*/
