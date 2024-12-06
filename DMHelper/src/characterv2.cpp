#include "characterv2.h"
#include "combatantfactory.h"
#include "globalsearch.h"
#include <QIcon>
#include <QDomElement>
#include <QTextDocument>
#include <QDebug>

Characterv2::Characterv2(const QString& name, QObject *parent) :
    Combatant(name, parent),
    TemplateObject(CombatantFactory::Instance()),
    _dndBeyondID(-1),
    _iconChanged(false),
    _allValues()
{
}

void Characterv2::inputXML(const QDomElement &element, bool isImport)
{
    beginBatchChanges();

    setDndBeyondID(element.attribute(QString("dndBeyondID"), QString::number(-1)).toInt());
    readXMLValues(element, isImport);

    Combatant::inputXML(element, isImport);

    endBatchChanges();
}

void Characterv2::copyValues(const CampaignObjectBase* other)
{
    const Characterv2* otherCharacter = dynamic_cast<const Characterv2*>(other);
    if(!otherCharacter)
        return;

    _dndBeyondID = otherCharacter->_dndBeyondID;

    Combatant::copyValues(other);
}

QIcon Characterv2::getDefaultIcon()
{
    if(_iconPixmap.isValid())
        return QIcon(_iconPixmap.getPixmap(DMHelper::PixmapSize_Battle).scaled(128,128, Qt::KeepAspectRatio));
    else
        return isInParty() ? QIcon(":/img/data/icon_contentcharacter.png") : QIcon(":/img/data/icon_contentnpc.png");
}

bool Characterv2::matchSearch(const QString& searchString, QString& result) const
{
    if(Combatant::matchSearch(searchString, result))
        return true;

    QHash<QString, DMHAttribute> elementAttributes = CombatantFactory::Instance()->getElements();
    QString searchResult;
    for(auto keyIt = elementAttributes.keyBegin(), end = elementAttributes.keyEnd(); keyIt != end; ++keyIt)
    {
        DMHAttribute attribute = elementAttributes.value(*keyIt);
        if(attribute._type == CombatantFactory::TemplateType_html)
        {
            QString value = getStringValue(*keyIt);
            if(GlobalSearch_Interface::compareStringValue(value, searchString, searchResult))
            {
                result = *keyIt + ": " + searchResult;
                return true;
            }
        }
    }

    return false;
}

void Characterv2::beginBatchChanges()
{
    _iconChanged = false;

    Combatant::beginBatchChanges();
}

void Characterv2::endBatchChanges()
{
    if((_batchChanges) && (_iconChanged))
        emit iconChanged(this);

    Combatant::endBatchChanges();
}

Combatant* Characterv2::clone() const
{
    qDebug() << "[Characterv2] WARNING: Character cloned - this is a highly questionable action!";

    Characterv2* newCharacter = new Characterv2(getName());
    newCharacter->copyValues(this);

    return newCharacter;
}

int Characterv2::getCombatantType() const
{
    return DMHelper::CombatantType_Character;
}

int Characterv2::getDndBeyondID() const
{
    return _dndBeyondID;
}

void Characterv2::setDndBeyondID(int id)
{
    _dndBeyondID = id;
}

bool Characterv2::isInParty() const
{
    return (getParentByType(DMHelper::CampaignType_Party) != nullptr);
}

void Characterv2::setIcon(const QString &newIcon)
{
    if(newIcon != _icon)
    {
        _icon = newIcon;
        _iconPixmap.setBasePixmap(_icon);
        registerChange();

        if(_batchChanges)
            _iconChanged = true;
        else
            emit iconChanged(this);
    }
}

int Characterv2::getSpeed() const
{
    return getIntValue(QString("speed"));
}

int Characterv2::getStrength() const
{
    return getIntValue(QString("strength"));
}

int Characterv2::getDexterity() const
{
    return getIntValue(QString("dexterity"));
}

int Characterv2::getConstitution() const
{
    return getIntValue(QString("constitution"));
}

int Characterv2::getIntelligence() const
{
    return getIntValue(QString("intelligence"));
}

int Characterv2::getWisdom() const
{
    return getIntValue(QString("wisdom"));
}

int Characterv2::getCharisma() const
{
    return getIntValue(QString("charisma"));
}

void Characterv2::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("dndBeyondID", getDndBeyondID());

    writeXMLValues(doc, element, targetDirectory, isExport);

    Combatant::internalOutputXML(doc, element, targetDirectory, isExport);
}

bool Characterv2::belongsToObject(QDomElement& element)
{
    if((CombatantFactory::Instance()) && (CombatantFactory::Instance()->hasEntry(element.tagName())))
        return true;
    else if((element.tagName() == QString("actions")) || (element.tagName() == QString("spell-data"))) // for backwards compatibility
        return true;
    else
        return Combatant::belongsToObject(element);
}

QHash<QString, QVariant>* Characterv2::valueHash()
{
    return &_allValues;
}

const QHash<QString, QVariant>* Characterv2::valueHash() const
{
    return &_allValues;
}

void Characterv2::declareDirty()
{
    registerChange();
}

void Characterv2::handleOldXMLs(const QDomElement& element)
{
    if(element.isNull())
        return;

    // HACK - need something that is more general and future-proof, need to update the minor version of the campaign file and detect this

    if(element.hasAttribute(QString("equipment")))
        _allValues.insert(QString("equipment"), Qt::convertFromPlainText(element.attribute(QString("equipment"))));

    if(element.hasAttribute(QString("notes")))
        _allValues.insert(QString("notes"), Qt::convertFromPlainText(element.attribute(QString("notes"))));

    if(element.hasAttribute(QString("proficiencies")))
        _allValues.insert(QString("proficiencies"), Qt::convertFromPlainText(element.attribute(QString("proficiencies"))));

    if((element.hasAttribute(QString("class2"))) && (element.attribute(QString("class2")) != QString("N/A")) && (!element.attribute(QString("class2")).isEmpty()))
        _allValues.insert(QString("class"), _allValues.value(QString("class")).toString() + QString("/") + element.attribute(QString("class2")));

    if((element.hasAttribute(QString("class3"))) && (element.attribute(QString("class3")) != QString("N/A")) && (!element.attribute(QString("class3")).isEmpty()))
        _allValues.insert(QString("class"), _allValues.value(QString("class")).toString() + QString("/") + element.attribute(QString("class3")));

    if((element.hasAttribute(QString("level2"))) && (element.attribute(QString("level2")) != QString("0")) && (!element.attribute(QString("level2")).isEmpty()))
        _allValues.insert(QString("level"), _allValues.value(QString("level")).toString() + QString("/") + element.attribute(QString("level2")));

    if((element.hasAttribute(QString("level3"))) && (element.attribute(QString("level3")) != QString("0")) && (!element.attribute(QString("level3")).isEmpty()))
        _allValues.insert(QString("level"), _allValues.value(QString("level")).toString() + QString("/") + element.attribute(QString("level3")));

    if(element.hasAttribute(QString("slots1")))
    {
        QList<QVariant> listValues;

        for(int i = 1; i <= 9; i++)
        {
            QString slotsKey = QString("slots") + QString::number(i);
            QString slotsUsedKey = QString("slotsused") + QString::number(i);
            if(element.hasAttribute(slotsKey))
            {
                QHash<QString, QVariant> listEntryValues;
                listEntryValues.insert(QString("level"), QVariant(i));
                QVariant resourceVariant;
                resourceVariant.setValue(ResourcePair(element.attribute(slotsUsedKey).toInt(), element.attribute(slotsKey).toInt()));
                listEntryValues.insert(QString("slots"), resourceVariant);
                listValues.append(listEntryValues);
            }
        }

        _allValues.insert(QString("spellSlots"), listValues);
    }
}

bool Characterv2::isAttributeSpecial(const QString& attribute) const
{
    return ((attribute == QString("expanded")) ||
            (attribute == QString("row")) ||
            (attribute == QString("name")) ||
            (attribute == QString("base-icon")) ||
            (attribute == QString("armorClass")) ||
            (attribute == QString("hitPoints")) ||
            (attribute == QString("hitDice")) ||
            (attribute == QString("conditions")) ||
            (attribute == QString("initiative")) ||
            (attribute == QString("icon"))); //||
            // TODO: (attribute == QString("attacks")));
}

QVariant Characterv2::getAttributeSpecial(const QString& attribute) const
{
    QVariant value;

    if(attribute == QString("expanded"))
        value = QVariant(getExpanded());
    else if(attribute == QString("row"))
        value = QVariant(getRow());
    else if(attribute == QString("name"))
        value = QVariant(getName());
    else if(attribute == QString("base-icon"))
        value = QVariant(getIconFile());
    else if(attribute == QString("armorClass"))
        value = QVariant(getArmorClass());
    else if(attribute == QString("hitPoints"))
        value = QVariant(getHitPoints());
    else if(attribute == QString("hitDice"))
        value.setValue(getHitDice());
    else if(attribute == QString("conditions"))
        value = QVariant(getConditions());
    else if(attribute == QString("initiative"))
        value = QVariant(getInitiative());
    else if(attribute == QString("icon"))
        value = QVariant(getIconFile());
// TODO:    else if(attribute == QString("attacks")) ?
//        value = QVariant(getAttacks());

    return value;
}

QString Characterv2::getAttributeSpecialAsString(const QString& attribute) const
{
    if(attribute == QString("expanded"))
        return QString::number(getExpanded());
    else if(attribute == QString("row"))
        return QString::number(getRow());
    else if(attribute == QString("name"))
        return getName();
    else if(attribute == QString("base-icon"))
        return getIconFile();
    else if(attribute == QString("armorClass"))
        return QString::number(getArmorClass());
    else if(attribute == QString("hitPoints"))
        return QString::number(getHitPoints());
    else if(attribute == QString("hitDice"))
        return getHitDice().toString();
    else if(attribute == QString("conditions"))
        return QString::number(getConditions());
    else if(attribute == QString("initiative"))
        return QString::number(getInitiative());
    else if(attribute == QString("icon"))
        return getIconFile();
    else
        return QString();
}

void Characterv2::setAttributeSpecial(const QString& key, const QString& value)
{
    if(key == QString("expanded"))
        qDebug() << "[Characterv2] Unexpected Request to set " << key << " to " << value;
    else if(key == QString("row"))
        qDebug() << "[Characterv2] Unexpected Request to set " << key << " to " << value;
    else if(key == QString("name"))
        setName(value);
    else if(key == QString("base-icon"))
        setIconFile(value);
    else if(key == QString("armorClass"))
        setArmorClass(value.toInt());
    else if(key == QString("hitPoints"))
        setHitPoints(value.toInt());
    else if(key == QString("hitDice"))
        setHitDice(Dice(value));
    else if(key == QString("conditions"))
        setConditions(value.toInt()); // TODO: conditions are not well-solved
    else if(key == QString("initiative"))
        setInitiative(value.toInt());
    else if(key == QString("icon"))
        setIcon(value);
    else
        qDebug() << "[Characterv2] ERROR: Attempt to set unknown special attribute " << key << " to " << value;
}
