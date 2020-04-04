#include "mapfactory.h"
#include "map.h"


MapFactory::MapFactory(QObject *parent) :
    ObjectFactory(parent)
{
}

CampaignObjectBase* MapFactory::createObject(int objectType, int subType, const QString& objectName, bool isImport)
{
    Q_UNUSED(isImport);
    Q_UNUSED(subType);

    switch(objectType)
    {
        case DMHelper::CampaignType_Map:
            return new Map(objectName);
        default:
            return nullptr;
    }
}

CampaignObjectBase* MapFactory::createObject(const QDomElement& element, bool isImport)
{
    Q_UNUSED(isImport);

    if(element.tagName() == QString("map"))
        return new Map();
    else
        return nullptr;
}
