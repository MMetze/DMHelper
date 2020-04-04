#include "party.h"
#include "dmconstants.h"
#include <QDomElement>

Party::Party(const QString& name, QObject *parent) :
    EncounterText(name, parent)
{
}

Party::~Party()
{
}

int Party::getObjectType() const
{
    return DMHelper::CampaignType_Party;
}

QDomElement Party::createOutputXML(QDomDocument &doc)
{
    return doc.createElement("party");
}
