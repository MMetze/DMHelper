#include "quickref.h"
#include <QFile>
#include <QDebug>

QuickRefSection::QuickRefSection(QDomElement &element, const QString& iconDir) :
    _name(element.firstChildElement(QString("name")).text()),
    _limitation(element.firstChildElement(QString("limitation")).text()),
    _subSections()
{
    QDomElement subSectionElement = element.firstChildElement( QString("subsection") );
    while( !subSectionElement.isNull() )
    {
        _subSections.append(QuickRefSubsection(subSectionElement, iconDir));
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


QuickRefSubsection::QuickRefSubsection(QDomElement &element, const QString& iconDir) :
    _description(element.firstChildElement(QString("description")).text()),
    _data()
{
    QDomElement dataElement = element.firstChildElement( QString("data") );
    while( !dataElement.isNull() )
    {
        _data.append(QuickRefData(dataElement, iconDir));
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

QuickRefData::QuickRefData(QDomElement &element, const QString& iconDir) :
    _title(element.attribute("title")),
    _icon(),
    _subtitle(element.firstChildElement(QString("subtitle")).text()),
    _description(element.firstChildElement(QString("description")).text()),
    _reference(element.firstChildElement(QString("reference")).text()),
    _bullets()
{
    QString iconName = element.firstChildElement(QString("icon")).text();
    QString resourceIcon = QString(":/img/data/img/") + iconName + QString(".png");
    if(QFile::exists(resourceIcon))
    {
        _icon = resourceIcon;
    }
    else
    {
        QString fileIcon = iconDir + iconName + QString(".png");
        if(QFile::exists(fileIcon))
            _icon = fileIcon;
        else
            qDebug() << "[QuickRefData] ERROR: Unable to find icon '" << iconName << "' in path: " << fileIcon;
    }

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
