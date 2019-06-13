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

void AdventureItem::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport)
{
    CampaignObjectBase::outputXML(doc, parent, targetDirectory, isExport);
}

void AdventureItem::inputXML(const QDomElement &element, bool isImport)
{
    CampaignObjectBase::inputXML(element, isImport);
}
