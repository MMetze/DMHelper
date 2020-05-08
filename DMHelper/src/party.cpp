#include "party.h"
#include "dmconstants.h"
#include <QDir>
#include <QDomElement>

Party::Party(const QString& name, QObject *parent) :
    EncounterText(name, parent),
    _icon(),
    _iconPixmap()
{
}

Party::~Party()
{
}

void Party::inputXML(const QDomElement &element, bool isImport)
{
    setIcon(element.attribute("icon"));

    CampaignObjectBase::inputXML(element, isImport);
}

int Party::getObjectType() const
{
    return DMHelper::CampaignType_Party;
}

QString Party::getIcon(bool localOnly) const
{
    Q_UNUSED(localOnly);
    return _icon;
}

QPixmap Party::getIconPixmap(DMHelper::PixmapSize iconSize)
{
    if(!_iconPixmap.isValid())
        _iconPixmap.setBasePixmap(QString(":/img/data/icon_party.png"));

    return _iconPixmap.getPixmap(iconSize);
}

void Party::setIcon(const QString &newIcon)
{
    if(newIcon != _icon)
    {
        _icon = newIcon;
        _iconPixmap.setBasePixmap(_icon.isEmpty() ? QString(":/img/data/icon_party.png") : _icon);
        emit dirty();
    }
}

QDomElement Party::createOutputXML(QDomDocument &doc)
{
    return doc.createElement("party");
}

void Party::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    QString iconPath = getIcon(true);
    if(iconPath.isEmpty())
        element.setAttribute("icon", QString(""));
    else
        element.setAttribute("icon", targetDirectory.relativeFilePath(iconPath));

    CampaignObjectBase::internalOutputXML(doc, element, targetDirectory, isExport);
}
