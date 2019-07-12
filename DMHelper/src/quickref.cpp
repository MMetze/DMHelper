#include "quickref.h"

QuickRefSection::QuickRefSection(QDomElement &element) :
    _name(element.firstChildElement(QString("name")).text()),
    _limitation(element.firstChildElement(QString("limitation")).text()),
    _subSections()
{
    QDomElement subSectionElement = element.firstChildElement( QString("subsection") );
    while( !subSectionElement.isNull() )
    {
        _subSections.append(QuickRefSubsection(subSectionElement));
        subSectionElement = subSectionElement.nextSiblingElement( QString("subsection") );
    }
}

QString QuickRefSection::getName() const
{
    return _name;
}

QString QuickRefSection::getLimitation() const
{
    return _limitation;
}

QList<QuickRefSubsection> QuickRefSection::getSubsections() const
{
    return _subSections;
}


QuickRefSubsection::QuickRefSubsection(QDomElement &element) :
    _description(element.firstChildElement(QString("description")).text()),
    _data()
{
    QDomElement dataElement = element.firstChildElement( QString("data") );
    while( !dataElement.isNull() )
    {
        _data.append(QuickRefData(dataElement));
        dataElement = dataElement.nextSiblingElement( QString("data") );
    }
}

QString QuickRefSubsection::getDescription() const
{
    return _description;
}

QList<QuickRefData> QuickRefSubsection::getData() const
{
    return _data;
}

QuickRefData::QuickRefData(QDomElement &element) :
    _title(element.attribute("title")),
    _icon(element.firstChildElement(QString("icon")).text()),
    _subtitle(element.firstChildElement(QString("subtitle")).text()),
    _description(element.firstChildElement(QString("description")).text()),
    _reference(element.firstChildElement(QString("reference")).text()),
    _bullets()
{
    QDomElement bulletsElement = element.firstChildElement( QString("bullets") );
    if( !bulletsElement.isNull() )
    {
        QDomElement bulletElement = bulletsElement.firstChildElement( QString("bullet") );
        while( !bulletElement.isNull() )
        {
            QDomCDATASection bulletData = bulletElement.firstChild().toCDATASection();
            _bullets.append(bulletData.data());
            bulletElement = bulletElement.nextSiblingElement( QString("bullet") );
        }
    }
}

QString QuickRefData::getTitle() const
{
    return _title;
}

QString QuickRefData::getIcon() const
{
    return _icon;
}

QString QuickRefData::getSubtitle() const
{
    return _subtitle;
}

QString QuickRefData::getDescription() const
{
    return _description;
}

QString QuickRefData::getReference() const
{
    return _reference;
}

QStringList QuickRefData::getBullets() const
{
    return _bullets;
}
