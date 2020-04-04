#include "placeholder.h"
#include "dmconstants.h"
#include <QDomElement>
#include <QDebug>

Placeholder::Placeholder(const QString& name, QObject *parent) :
    CampaignObjectBase(name, parent),
    _tagName(),
    _attributes()
{
}

Placeholder::~Placeholder()
{
}

void Placeholder::inputXML(const QDomElement &element, bool isImport)
{
    qDebug() << "[Placeholder] ERROR: unexpected element received, parked in a placeholder: " << element.tagName() << ", import: " << isImport;

    _tagName = element.tagName();
    _attributes.clear();

    QDomNamedNodeMap map = element.attributes();
    for(int i = 0 ; i < map.length() ; ++i)
    {
        if(!(map.item(i).isNull()))
        {
            QDomNode debug = map.item(i);
            QDomAttr attr = debug.toAttr();
            if(!attr.isNull())
            {
                _attributes.append(PlaceHolderAttribute(attr.name(), attr.value()));
            }
        }
    }

    CampaignObjectBase::inputXML(element, isImport);
}

int Placeholder::getObjectType() const
{
    return DMHelper::CampaignType_Placeholder;
}

QDomElement Placeholder::createOutputXML(QDomDocument &doc)
{
    return doc.createElement(_tagName);
}

void Placeholder::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    for(int i = 0; i < _attributes.count(); ++i)
    {
        element.setAttribute(_attributes.at(i)._name, _attributes.at(i)._value);
    }

    CampaignObjectBase::internalOutputXML(doc, element, targetDirectory, isExport);
}

