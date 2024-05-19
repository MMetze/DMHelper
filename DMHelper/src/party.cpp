#include "party.h"
#include "dmconstants.h"
#include "characterv2.h"
#include <QDir>
#include <QDomElement>
#include <QIcon>

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

void Party::copyValues(const CampaignObjectBase* other)
{
    const Party* otherParty = dynamic_cast<const Party*>(other);
    if(!otherParty)
        return;

    _icon = otherParty->_icon;
    _iconPixmap.setBasePixmap(_icon.isEmpty() ? QString(":/img/data/icon_party.png") : _icon);

    EncounterText::copyValues(other);
}

int Party::getObjectType() const
{
    return DMHelper::CampaignType_Party;
}

QIcon Party::getDefaultIcon()
{
    if(_iconPixmap.isValid())
        return QIcon(_iconPixmap.getPixmap(DMHelper::PixmapSize_Battle).scaled(128,128, Qt::KeepAspectRatio));
    else
        return QIcon(":/img/data/icon_contentparty.png");
}

QString Party::getPartyIcon(bool localOnly) const
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

QList<Characterv2*> Party::getActiveCharacters()
{
    QList<Characterv2*> actives;

    QList<Characterv2*> characterList = findChildren<Characterv2*>();
    for(int i = 0; i < characterList.count(); ++i)
    {
        if(characterList.at(i)->getBoolValue(QString("active")))
            actives.append(characterList.at(i));
    }

    return actives;
}

void Party::setIcon(const QString &newIcon)
{
    if(newIcon != _icon)
    {
        _icon = newIcon;
        _iconPixmap.setBasePixmap(_icon.isEmpty() ? QString(":/img/data/icon_party.png") : _icon);
        emit dirty();
        emit iconChanged(this);
    }
}

QDomElement Party::createOutputXML(QDomDocument &doc)
{
    return doc.createElement("party");
}

void Party::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    QString iconPath = getPartyIcon(true);
    if(iconPath.isEmpty())
        element.setAttribute("icon", QString(""));
    else
        element.setAttribute("icon", targetDirectory.relativeFilePath(iconPath));

    CampaignObjectBase::internalOutputXML(doc, element, targetDirectory, isExport);
}
