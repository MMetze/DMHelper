#include "characterv2.h"
#include "combatantfactory.h"
#include <QIcon>
#include <QDomElement>
#include <QTextDocument>
#include <QDebug>

Characterv2::Characterv2(const QString& name, QObject *parent) :
    Combatant(name, parent),
    _dndBeyondID(-1),
    _iconChanged(false),
/*
    _stringValues(),
    _intValues(),
    _boolValues(),
    _diceValues(),
    _resourceValues()
*/
    _allValues()
{
}

void Characterv2::inputXML(const QDomElement &element, bool isImport)
{
    beginBatchChanges();

    setDndBeyondID(element.attribute(QString("dndBeyondID"), QString::number(-1)).toInt());
    readXMLValues(element, isImport);

    Combatant::inputXML(element, isImport);

    //readActionList(element, QString("actions"), _actions, isImport);

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

QString Characterv2::getValueAsString(const QString& key) const
{
    if(isAttributeSpecial(key))
        return getAttributeSpecialAsString(key);

    if((!_allValues.contains(key)) || (!CombatantFactory::Instance()->hasEntry(key)))
        qDebug() << "[Characterv2] Attempting to read the value for the unknown key " << key;

    DMHAttribute attribute = CombatantFactory::Instance()->getAttribute(key);
    switch(attribute._type)
    {
        case CombatantFactory::TemplateType_string:
        case CombatantFactory::TemplateType_html:
            return getStringValue(key);
        case CombatantFactory::TemplateType_integer:
            return QString::number(getIntValue(key));
        case CombatantFactory::TemplateType_boolean:
            return getBoolValue(key) ? "true" : "false";
        case CombatantFactory::TemplateType_dice:
            return getDiceValue(key).toString();
        case CombatantFactory::TemplateType_resource:
        {
            ResourcePair pair = getResourceValue(key);
            return QString::number(pair.first) + QString(",") + QString::number(pair.second);
        }
        default:
            qDebug() << "[Characterv2] WARNING: Trying to read value for unexpected key: " << key << " with type " << attribute._type;
            return QString();
    }
}

QString Characterv2::getStringValue(const QString& key) const
{
    if(isAttributeSpecial(key))
        return getAttributeSpecialAsString(key);
    else
        return _allValues.value(key, QString()).toString();
}

int Characterv2::getIntValue(const QString& key) const
{
    if(isAttributeSpecial(key))
        return getAttributeSpecial(key).toInt();
    else
        return _allValues.value(key, 0).toInt();
}

bool Characterv2::getBoolValue(const QString& key) const
{
    if(isAttributeSpecial(key))
        return getAttributeSpecial(key).toBool();
    else
        return _allValues.value(key, false).toBool();
}

Dice Characterv2::getDiceValue(const QString& key) const
{
    if(isAttributeSpecial(key))
        return getAttributeSpecial(key).value<Dice>();
    else
        return _allValues.value(key, QVariant()).value<Dice>();
}

ResourcePair Characterv2::getResourceValue(const QString& key) const
{
    return _allValues.value(key, QVariant()).value<ResourcePair>();
}

QList<QVariant> Characterv2::getListValue(const QString& key) const
{
    return _allValues.value(key).toList();
}

void Characterv2::setValue(const QString& key, const QVariant& value)
{
    if(!_allValues.contains(key))
    {
        qDebug() << "[Characterv2] Unexpected Request to set " << key << " to " << value;
        return;
    }

    if(_allValues.value(key) == value)
        return;

    _allValues.insert(key, value);
    registerChange();
}

void Characterv2::setValue(const QString& key, const QString& value)
{
    if(isAttributeSpecial(key))
        setAttributeSpecial(key, value);

    if((!_allValues.contains(key)) || (!CombatantFactory::Instance()->hasEntry(key)))
    {
        qDebug() << "[Characterv2] Attempting to set the value \"" << value << "\" for the unknown key " << key;
        return;
    }

    DMHAttribute attribute = CombatantFactory::Instance()->getAttribute(key);
    switch(attribute._type)
    {
        case CombatantFactory::TemplateType_string:
        case CombatantFactory::TemplateType_html:
            setValue(key, QVariant(value));
            break;
        case CombatantFactory::TemplateType_integer:
            setValue(key, QVariant(value.toInt()));
            break;
        case CombatantFactory::TemplateType_boolean:
            setValue(key, QVariant(static_cast<bool>(value.toInt())));
            break;
        case CombatantFactory::TemplateType_dice:
        {
            QVariant newValue;
            newValue.setValue(Dice(value));
            setValue(key, newValue);
            break;
        }
        case CombatantFactory::TemplateType_resource:
        {
            QStringList resourceString = value.split(QString(","));
            if(resourceString.size() == 2)
            {
                QVariant newValue;
                newValue.setValue(ResourcePair(resourceString.at(0).toInt(), resourceString.at(1).toInt()));
                setValue(key, newValue);
            }
            else
                qDebug() << "[Characterv2] WARNING: Resource attribute missing values: " << key << " with type " << attribute._type << ": " << value;
            break;
        }
        default:
            qDebug() << "[Characterv2] WARNING: Setting value for unexpected key: " << key << " with type " << attribute._type << " and value " << value;
            break;
    }
}

void Characterv2::setStringValue(const QString& key, const QString& value)
{
    setValue(key, QVariant(value));
}

void Characterv2::setIntValue(const QString& key, int value)
{
    setValue(key, QVariant(value));
}

void Characterv2::setBoolValue(const QString& key, bool value)
{
    setValue(key, QVariant(value));
}

void Characterv2::setDiceValue(const QString& key, const Dice& value)
{
    QVariant newValue;
    newValue.setValue(value);
    setValue(key, newValue);
}

void Characterv2::setResourceValue(const QString& key, const ResourcePair& value)
{
    QVariant newValue;
    newValue.setValue(value);
    setValue(key, newValue);
}

void Characterv2::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("dndBeyondID", getDndBeyondID());

    //writeActionList(doc, element, QString("actions"), _actions, isExport);

    Combatant::internalOutputXML(doc, element, targetDirectory, isExport);
}

void Characterv2::setListValue(const QString& key, int index, const QString& listEntryKey, const QVariant& listEntryValue)
{
    QList<QVariant> list = getListValue(key);

    if((index < 0) || (index >= list.size()))
        return;

    QHash<QString, QVariant> listEntryValues = list.at(index).toHash();
    listEntryValues.insert(listEntryKey, listEntryValue);
    list.replace(index, listEntryValues);
    setValue(key, QVariant(list));
}

bool Characterv2::belongsToObject(QDomElement& element)
{
    if((CombatantFactory::Instance()) && (CombatantFactory::Instance()->hasElement(element.tagName())))
        return true;
    else if((element.tagName() == QString("actions")) || (element.tagName() == QString("spell-data"))) // for backwards compatibility
        return true;
    else
        return Combatant::belongsToObject(element);

    /*
    if((element.tagName() == QString("actions")) || (element.tagName() == QString("spell-data")))
        return true;
    else
        return Combatant::belongsToObject(element);
    */
}

void Characterv2::readXMLValues(const QDomElement& element, bool isImport)
{
    Q_UNUSED(isImport);

    if((!CombatantFactory::Instance()) || (element.isNull()))
        return;

    // Iterate through expected attributes
    QHash<QString, DMHAttribute> attributeHash = CombatantFactory::Instance()->getAttributes();
    for(auto keyIt = attributeHash.keyBegin(), end = attributeHash.keyEnd(); keyIt != end; ++keyIt)
    {
        if(!isAttributeSpecial(*keyIt))
        {
            QVariant attributeValue = readAttributeValue(element, *keyIt);
            if(!attributeValue.isNull())
                _allValues.insert(*keyIt, attributeValue);
        }
    }

    // Child elements
    QDomElement childElement = element.firstChildElement();
    while(!childElement.isNull())
    {
        QString tagName = childElement.tagName();
        if(CombatantFactory::Instance()->hasElement(tagName))
        {
            DMHAttribute attribute = CombatantFactory::Instance()->getElement(tagName);
            if(attribute._type == CombatantFactory::TemplateType_html)
            {
                QDomNode dataChildNode = childElement.firstChild();
                if((!dataChildNode.isNull()) && (dataChildNode.isCDATASection()))
                {
                    QDomCDATASection dataSection = dataChildNode.toCDATASection();
                    _allValues.insert(tagName, dataSection.data());
                }
                else
                {
                    qDebug() << "[Characterv2] WARNING: HTML child element missing a data node: " << tagName << " with type " << attribute._type;
                }
            }
            else
            {
                qDebug() << "[Characterv2] WARNING: Unexpected child element: " << tagName << " with type " << attribute._type;
            }
        }
        else if(CombatantFactory::Instance()->hasElementList(tagName))
        {
            QList<QVariant> listValues;

            // Iterate through the list and create the individual attributes
            QHash<QString, DMHAttribute> listAttributes = CombatantFactory::Instance()->getElementList(tagName);
            QDomElement listEntry = childElement.firstChildElement();
            while(!listEntry.isNull())
            {
                QHash<QString, QVariant> listEntryValues;
                for(auto keyIt = listAttributes.keyBegin(), end = listAttributes.keyEnd(); keyIt != end; ++keyIt)
                {
                    QVariant attributeValue = readAttributeValue(element, *keyIt);
                    if(!attributeValue.isNull())
                        listEntryValues.insert(*keyIt, attributeValue);
                }

                // Add the list entry to the list
                listValues.append(listEntryValues);

                listEntry = listEntry.nextSiblingElement();
            }

            // Add the list entry to the main list
            _allValues.insert(tagName, listValues);
        }
        else
        {
            qDebug() << "[Characterv2] WARNING: Unknown value type: " << tagName;
        }

        childElement = childElement.nextSiblingElement();
    }

    handleOldXMLs(element);
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

QVariant Characterv2::readAttributeValue(const QDomElement& element, const QString& name)
{
    QVariant result;

    DMHAttribute attribute = CombatantFactory::Instance()->getAttribute(name);
    switch(attribute._type)
    {
        case CombatantFactory::TemplateType_string:
            result = element.attribute(name, attribute._default);
            break;
        case CombatantFactory::TemplateType_integer:
            result = element.attribute(name, attribute._default).toInt();
            break;
        case CombatantFactory::TemplateType_boolean:
            result = static_cast<bool>(element.attribute(name, attribute._default).toInt());
            break;
        case CombatantFactory::TemplateType_dice:
        {
            result.setValue(Dice(element.attribute(name, attribute._default)));
            break;
        }
        case CombatantFactory::TemplateType_resource:
        {
            QStringList resourceString = element.attribute(name, attribute._default).split(QString(","));
            if(resourceString.size() == 2)
                result.setValue(ResourcePair(resourceString.at(0).toInt(), resourceString.at(1).toInt()));
            else
                qDebug() << "[Characterv2] WARNING: Resource attribute missing values: " << name << " with type " << attribute._type << ": " << element.attribute(name, attribute._default);
            break;
        }
        default:
            qDebug() << "[Characterv2] WARNING: Unexpected attribute: " << name << " with type " << attribute._type;
            break;
    }

    return result;
}
