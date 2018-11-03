#include "adventureitem.h"
#include "adventure.h"

AdventureItem::AdventureItem(QObject *parent) :
    CampaignObjectBase(parent)
{
}

Adventure* AdventureItem::getAdventure() const
{
    return dynamic_cast<Adventure*>(parent());
}

void AdventureItem::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory)
{
    CampaignObjectBase::outputXML(doc, parent, targetDirectory);
}

void AdventureItem::inputXML(const QDomElement &element)
{
    CampaignObjectBase::inputXML(element);
}
