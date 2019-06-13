#include "encounter.h"
#include "dmconstants.h"
#include <QDomDocument>
#include <QDomElement>
#include <QDomCDATASection>

Encounter::Encounter(const QString& encounterName, QObject *parent) :
    AdventureItem(parent),
    _name(encounterName),
    _widget(nullptr)
{
}

void Encounter::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport)
{
    Q_UNUSED(targetDirectory);

    QDomElement element = doc.createElement( "encounter" );

    AdventureItem::outputXML(doc, element, targetDirectory, isExport);

    element.setAttribute( "name", getName() );
    element.setAttribute( "type", getType() );

    internalOutputXML(doc, element, targetDirectory, isExport);

    parent.appendChild(element);
}

void Encounter::inputXML(const QDomElement &element, bool isImport)
{
    AdventureItem::inputXML(element, isImport);

    setName(element.attribute("name"));
}

int Encounter::getType() const
{
    return DMHelper::EncounterType_Blank;
}

QString Encounter::getName() const
{
    return _name;
}

void Encounter::setName(const QString& newName)
{
    if(_name != newName)
    {
        _name = newName;
        //emit dirty();
        emit changed();
    }
}

bool Encounter::hasData() const
{
    return false;
}
