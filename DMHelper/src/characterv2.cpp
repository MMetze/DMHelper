#include "characterv2.h"
#include "combatantfactory.h"
#include "globalsearch.h"
#include "bestiary.h"
#include "monsterclassv2.h"
#include <QIcon>
#include <QDomElement>
#include <QTextDocument>
#include <QDebug>

Characterv2::Characterv2(const QString& name, QObject *parent) :
    Combatant(name, parent),
    TemplateObject(CombatantFactory::Instance()),
    _dndBeyondID(-1),
    _iconChanged(false),
    _allValues(),
    _icons(),
    _scaledPixmaps()
{
}

void Characterv2::inputXML(const QDomElement &element, bool isImport)
{
    beginBatchChanges();

    setDndBeyondID(element.attribute(QString("dndBeyondID"), QString::number(-1)).toInt());
    readXMLValues(element, isImport);
    readIcons(element, isImport);

    Combatant::inputXML(element, isImport);

    endBatchChanges();
}

void Characterv2::copyValues(const CampaignObjectBase* other)
{
    const Characterv2* otherCharacter = dynamic_cast<const Characterv2*>(other);
    if(!otherCharacter)
        return;

    _dndBeyondID = otherCharacter->_dndBeyondID;
    foreach(const QString& key, _allValues.keys())
    {
        _allValues.insert(key, otherCharacter->_allValues.value(key));
    }

    _icons = otherCharacter->_icons;
    _scaledPixmaps = otherCharacter->_scaledPixmaps;

    Combatant::copyValues(other);
}

QIcon Characterv2::getDefaultIcon()
{
    if(!_scaledPixmaps.isEmpty() && _scaledPixmaps[0].isValid())
        return QIcon(_scaledPixmaps[0].getPixmap(DMHelper::PixmapSize_Battle).scaled(128,128, Qt::KeepAspectRatio));
    else if(_iconPixmap.isValid())
        return QIcon(_iconPixmap.getPixmap(DMHelper::PixmapSize_Battle).scaled(128,128, Qt::KeepAspectRatio));
    else
        return isInParty() ? QIcon(":/img/data/icon_contentcharacter.png") : QIcon(":/img/data/icon_contentnpc.png");
}

bool Characterv2::matchSearch(const QString& searchString, QString& result) const
{
    if(Combatant::matchSearch(searchString, result))
        return true;

    return matchSearchString(searchString, result);
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

QString Characterv2::getIconFile() const
{
    if(!_icons.isEmpty())
        return _icons.at(0);
    return _icon;
}

QPixmap Characterv2::getIconPixmap(DMHelper::PixmapSize iconSize)
{
    return getIconPixmap(iconSize, 0);
}

QPixmap Characterv2::getIconPixmap(DMHelper::PixmapSize iconSize, int index)
{
    if((index >= 0) && (index < _scaledPixmaps.count()))
        return _scaledPixmaps[index].getPixmap(iconSize);
    else if(_iconPixmap.isValid())
        return _iconPixmap.getPixmap(iconSize);
    else
        return ScaledPixmap::defaultPixmap()->getPixmap(iconSize);
}

int Characterv2::getIconCount() const
{
    return _icons.count();
}

QStringList Characterv2::getIconList() const
{
    return _icons;
}

QString Characterv2::getIcon(int index) const
{
    if((index < 0) || (index >= _icons.count()))
        return QString();
    else
        return _icons.at(index);
}

void Characterv2::setIcon(const QString &newIcon)
{
    if(newIcon.isEmpty())
        return;

    if(_icons.isEmpty())
    {
        addIcon(newIcon);
    }
    else
    {
        if(newIcon == _icons.at(0))
            return;

        ScaledPixmap newPixmap;
        newPixmap.setBasePixmap(newIcon);
        _icons[0] = newIcon;
        _scaledPixmaps[0] = newPixmap;

        _icon = newIcon;
        _iconPixmap.setBasePixmap(_icon);
        registerChange();

        if(_batchChanges)
            _iconChanged = true;
        else
            emit iconChanged(this);
    }
}

void Characterv2::addIcon(const QString &iconFile)
{
    if((iconFile.isEmpty()) || (_icons.contains(iconFile)))
        return;

    ScaledPixmap newPixmap;
    if(!newPixmap.setBasePixmap(iconFile))
    {
        qDebug() << "[Characterv2] ERROR: Unable to set icon pixmap for character: " << getName() << " - " << iconFile;
        return;
    }

    _icons.append(iconFile);
    _scaledPixmaps.append(newPixmap);

    // Keep base class in sync with first icon
    if(_icons.count() == 1)
    {
        _icon = iconFile;
        _iconPixmap.setBasePixmap(_icon);
    }

    registerChange();

    if(_batchChanges)
        _iconChanged = true;
    else
        emit iconChanged(this);
}

void Characterv2::setIcon(int index, const QString& iconFile)
{
    if((index < 0) || (index >= _icons.count()) || (iconFile.isEmpty()))
        return;

    ScaledPixmap newPixmap;
    if(!newPixmap.setBasePixmap(iconFile))
    {
        qDebug() << "[Characterv2] ERROR: Unable to set icon pixmap for character: " << getName() << " - " << iconFile;
        return;
    }

    _icons[index] = iconFile;
    _scaledPixmaps[index] = newPixmap;

    // Keep base class in sync with first icon
    if(index == 0)
    {
        _icon = iconFile;
        _iconPixmap.setBasePixmap(_icon);
    }

    registerChange();

    if(_batchChanges)
        _iconChanged = true;
    else
        emit iconChanged(this);
}

void Characterv2::removeIcon(int index)
{
    if((index < 0) || (index >= _icons.count()))
        return;

    _icons.removeAt(index);
    _scaledPixmaps.removeAt(index);

    // Keep base class in sync with first icon
    if(_icons.isEmpty())
    {
        _icon.clear();
        _iconPixmap = ScaledPixmap();
    }
    else if(index == 0)
    {
        _icon = _icons.at(0);
        _iconPixmap.setBasePixmap(_icon);
    }

    registerChange();

    if(_batchChanges)
        _iconChanged = true;
    else
        emit iconChanged(this);
}

void Characterv2::removeIcon(const QString& iconFile)
{
    removeIcon(_icons.indexOf(iconFile));
}

void Characterv2::clearIcon()
{
    _icons.clear();
    _scaledPixmaps.clear();
    _icon.clear();
    _iconPixmap = ScaledPixmap();
    registerChange();

    if(_batchChanges)
        _iconChanged = true;
    else
        emit iconChanged(this);
}

void Characterv2::refreshIconPixmaps()
{
    _scaledPixmaps.clear();
    for(int i = 0; i < _icons.count(); ++i)
    {
        ScaledPixmap newPixmap;
        newPixmap.setBasePixmap(_icons.at(i));
        _scaledPixmaps.append(newPixmap);
    }

    // Keep base class in sync
    if(!_icons.isEmpty())
        _iconPixmap.setBasePixmap(_icons.at(0));

    registerChange();

    if(_batchChanges)
        _iconChanged = true;
    else
        emit iconChanged(this);
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

void Characterv2::copyMonsterValues(MonsterClassv2& monster)
{
    beginBatchChanges();

    setIcon(Bestiary::Instance()->getDirectory().filePath(Bestiary::Instance()->findMonsterImage(monster.getStringValue("name"), monster.getIcon())));

    QHash<QString, DMHAttribute> attributeHash = CombatantFactory::Instance()->getAttributes();
    for(auto keyIt = attributeHash.keyBegin(), end = attributeHash.keyEnd(); keyIt != end; ++keyIt)
    {
        if(monster.hasValue(*keyIt))
            setValue(*keyIt, monster.getValueAsString(*keyIt));
    }

    QHash<QString, DMHAttribute> elementHash = CombatantFactory::Instance()->getElements();
    for(auto keyIt = elementHash.keyBegin(), end = elementHash.keyEnd(); keyIt != end; ++keyIt)
    {
        if(monster.hasValue(*keyIt))
            setValue(*keyIt, monster.getValueAsString(*keyIt));
    }

    // Special case for 5e
    if((attributeHash.contains("armorClass")) && (monster.hasValue("armor_class")))
        setValue("armorClass", monster.getValueAsString("armor_class"));

    if((attributeHash.contains("hitPoints")) && (monster.hasValue("hit_points")))
        setValue("hitPoints", monster.getValueAsString("hit_points"));

    if((attributeHash.contains("hitPoints")) && (monster.hasValue("hit_points")))
        setValue("maximumHp", monster.getValueAsString("hit_points"));

    if((attributeHash.contains("race")) && (monster.hasValue("name")))
        setValue("race", monster.getValueAsString("name"));

    if((attributeHash.contains("speed")) && (monster.hasValue("speed")))
        setValue("speed", monster.getValueAsString("speed").left(monster.getValueAsString("speed").indexOf(" ")).toInt());

    if((attributeHash.contains("movement")) && (monster.hasValue("speed")))
        setValue("movement", monster.getValueAsString("speed"));

    endBatchChanges();
}

void Characterv2::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("dndBeyondID", getDndBeyondID());

    writeXMLValues(doc, element, targetDirectory, isExport);
    writeIcons(doc, element, targetDirectory, isExport);

    Combatant::internalOutputXML(doc, element, targetDirectory, isExport);
}

bool Characterv2::belongsToObject(QDomElement& element)
{
    if(element.tagName() == QString("icon"))
        return true;
    else if((CombatantFactory::Instance()) && (CombatantFactory::Instance()->hasEntry(element.tagName())))
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
        value = QVariant(getConditionList().join(QStringLiteral(",")));
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
        return getConditionList().join(QStringLiteral(","));
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
        return;
    else if(key == QString("row"))
        return;
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
    {
        setConditionList(value.split(QStringLiteral(","), Qt::SkipEmptyParts));
    }
    else if(key == QString("initiative"))
        setInitiative(value.toInt());
    else if(key == QString("icon"))
        setIcon(value);
    else
        qDebug() << "[Characterv2] ERROR: Attempt to set unknown special attribute " << key << " to " << value;
}

void Characterv2::readIcons(const QDomElement& element, bool isImport)
{
    Q_UNUSED(isImport);

    // Read multi-icon child elements first
    QDomElement iconElement = element.firstChildElement("icon");
    while(!iconElement.isNull())
    {
        addIcon(iconElement.attribute("filename"));
        iconElement = iconElement.nextSiblingElement("icon");
    }

    // Legacy single-icon attribute — only use if no child icon elements were found
    if((_icons.isEmpty()) && (element.hasAttribute("icon")) && (!element.attribute("icon").isEmpty()))
        addIcon(element.attribute("icon"));
}

void Characterv2::writeIcons(QDomDocument &doc, QDomElement& element, QDir& targetDirectory, bool isExport) const
{
    Q_UNUSED(isExport);

    if(_icons.count() <= 1)
    {
        // Single icon or no icons — use legacy attribute format (handled by Combatant::internalOutputXML)
        return;
    }

    // Multiple icons — write as child elements
    for(int i = 0; i < _icons.count(); ++i)
    {
        if(!_icons.at(i).isEmpty())
        {
            QDomElement iconElement = doc.createElement("icon");
            iconElement.setAttribute("filename", targetDirectory.relativeFilePath(_icons.at(i)));
            element.appendChild(iconElement);
        }
    }
}
