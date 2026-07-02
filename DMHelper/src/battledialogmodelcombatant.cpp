#include "battledialogmodelcombatant.h"
#include <QDomElement>

namespace
{
    static const char* OVERRIDE_ELEMENT = "override";
    static const char* OVERRIDE_ATTR_KEY = "key";
    static const char* OVERRIDE_ATTR_VALUE = "value";

    bool overrideKeyUsesBool(const QString& key)
    {
        return (key == QLatin1String(BattleDialogModelCombatant::DMH_KEY_IS_SHOWN))
            || (key == QLatin1String(BattleDialogModelCombatant::DMH_KEY_IS_KNOWN))
            || (key == QLatin1String(BattleDialogModelCombatant::DMH_KEY_IS_DONE));
    }

    bool overrideKeyUsesDouble(const QString& key)
    {
        return key == QLatin1String(BattleDialogModelCombatant::DMH_KEY_MOVED);
    }

    bool overrideKeyUsesInt(const QString& key)
    {
        return (key == QLatin1String(BattleDialogModelCombatant::DMH_KEY_INITIATIVE))
            || (key == QLatin1String(BattleDialogModelCombatant::DMH_KEY_HEALTH));
    }

    QVariant overrideValueFromString(const QString& key, const QString& value)
    {
        if(overrideKeyUsesBool(key))
            return QVariant((value == QLatin1String("1")) || (value.toLower() == QLatin1String("true")));
        if(overrideKeyUsesDouble(key))
            return QVariant(value.toDouble());
        if(overrideKeyUsesInt(key))
            return QVariant(value.toInt());
        return QVariant(value);
    }
}

const char* BattleDialogModelCombatant::DMH_KEY_NAME                = "name";
const char* BattleDialogModelCombatant::DMH_KEY_INITIATIVE          = "dmh:initiative";
const char* BattleDialogModelCombatant::DMH_KEY_MOVED               = "dmh:moved";
const char* BattleDialogModelCombatant::DMH_KEY_IS_SHOWN            = "dmh:isShown";
const char* BattleDialogModelCombatant::DMH_KEY_IS_KNOWN            = "dmh:isKnown";
const char* BattleDialogModelCombatant::DMH_KEY_IS_DONE             = "dmh:isDone";
const char* BattleDialogModelCombatant::DMH_KEY_HEALTH              = "dmh:health";
const char* BattleDialogModelCombatant::DMH_KEY_CONDITIONS          = "dmh:conditions";
const char* BattleDialogModelCombatant::DMH_KEY_PER_ROUND_RESOURCES = "dmh:perRoundResources";

BattleDialogModelCombatant::BattleDialogModelCombatant(const QString& name, QObject *parent) :
    BattleDialogModelObject(QPointF(), 0.0, name, parent),
    _combatant(nullptr),
    _initiative(0),
    _sortPosition(-1),
    _groupId(),
    _moved(0.0),
    _isShown(true),
    _isKnown(true),
    _isSelected(false),
    _isDone(false),
    _overrides()
{
}

BattleDialogModelCombatant::BattleDialogModelCombatant(Combatant* combatant) :
    BattleDialogModelObject(),
    _combatant(combatant),
    _initiative(0),
    _sortPosition(-1),
    _groupId(),
    _moved(0.0),
    _isShown(true),
    _isKnown(true),
    _isSelected(false),
    _isDone(false),
    _overrides()
{
}

BattleDialogModelCombatant::BattleDialogModelCombatant(Combatant* combatant, int initiative, const QPointF& position) :
    BattleDialogModelObject(position),
    _combatant(combatant),
    _initiative(initiative),
    _sortPosition(-1),
    _groupId(),
    _moved(0.0),
    _isShown(true),
    _isKnown(true),
    _isSelected(false),
    _isDone(false),
    _overrides()
{
}

BattleDialogModelCombatant::~BattleDialogModelCombatant()
{
}

void BattleDialogModelCombatant::inputXML(const QDomElement &element, bool isImport)
{
    BattleDialogModelObject::inputXML(element, isImport);

    _initiative = element.attribute("initiative", QString::number(0)).toInt();
    _sortPosition = element.attribute("sort", QString::number(-1)).toInt();
    _isShown = static_cast<bool>(element.attribute("isShown", QString::number(1)).toInt());
    _isKnown = static_cast<bool>(element.attribute("isKnown", QString::number(1)).toInt());
    _isDone = static_cast<bool>(element.attribute("done", QString::number(0)).toInt());

    QString groupIdStr = element.attribute("groupId");
    _groupId = groupIdStr.isEmpty() ? QUuid() : QUuid(groupIdStr);

    _overrides.clear();
    QDomElement overrideElement = element.firstChildElement(QString::fromLatin1(OVERRIDE_ELEMENT));
    while(!overrideElement.isNull())
    {
        const QString key = overrideElement.attribute(QString::fromLatin1(OVERRIDE_ATTR_KEY));
        if(!key.isEmpty())
        {
            const QString value = overrideElement.attribute(QString::fromLatin1(OVERRIDE_ATTR_VALUE));
            _overrides.insert(key, overrideValueFromString(key, value));
        }

        overrideElement = overrideElement.nextSiblingElement(QString::fromLatin1(OVERRIDE_ELEMENT));
    }
}

void BattleDialogModelCombatant::copyValues(const CampaignObjectBase* other)
{
    const BattleDialogModelCombatant* otherCombatant = dynamic_cast<const BattleDialogModelCombatant*>(other);
    if(!otherCombatant)
        return;

    _combatant = otherCombatant->_combatant;
    _initiative = otherCombatant->_initiative;
    _moved = otherCombatant->_moved;
    _groupId = otherCombatant->_groupId;
    _isShown = otherCombatant->_isShown;
    _isKnown = otherCombatant->_isKnown;
    _isSelected = otherCombatant->_isSelected;
    _isDone = otherCombatant->_isDone;
    _overrides = otherCombatant->_overrides;

    BattleDialogModelObject::copyValues(other);
}

int BattleDialogModelCombatant::getObjectType() const
{
    return DMHelper::CampaignType_BattleContentCombatant;
}

bool BattleDialogModelCombatant::getShown() const
{
    return _isShown;
}

bool BattleDialogModelCombatant::getKnown() const
{
    return _isKnown;
}

bool BattleDialogModelCombatant::getSelected() const
{
    return _isSelected;
}

bool BattleDialogModelCombatant::getDone() const
{
    return _isDone;
}

int BattleDialogModelCombatant::getInitiative() const
{
    return _initiative;
}

void BattleDialogModelCombatant::setInitiative(int initiative)
{
    if(_initiative != initiative)
    {
        _initiative = initiative;
        setOverride(QString::fromLatin1(DMH_KEY_INITIATIVE), initiative);
        emit initiativeChanged(this);
    }
}

int BattleDialogModelCombatant::getSortPosition() const
{
    return _sortPosition;
}

void BattleDialogModelCombatant::setSortPosition(int sortPosition)
{
    _sortPosition = sortPosition;
}

QUuid BattleDialogModelCombatant::getGroupId() const
{
    return _groupId;
}

void BattleDialogModelCombatant::setGroupId(const QUuid& groupId)
{
    if(_groupId != groupId)
    {
        _groupId = groupId;
    }
}

Combatant* BattleDialogModelCombatant::getCombatant() const
{
    return _combatant;
}

int BattleDialogModelCombatant::getAbilityValue(Combatant::Ability ability) const
{
    switch(ability)
    {
        case Combatant::Ability_Strength:
            return getStrength();
        case Combatant::Ability_Dexterity:
            return getDexterity();
        case Combatant::Ability_Constitution:
            return getConstitution();
        case Combatant::Ability_Intelligence:
            return getIntelligence();
        case Combatant::Ability_Wisdom:
            return getWisdom();
        case Combatant::Ability_Charisma:
            return getCharisma();
        default:
            return -1;
    }
}

qreal BattleDialogModelCombatant::getMoved()
{
    return _moved;
}

void BattleDialogModelCombatant::setMoved(qreal moved)
{
    if(_moved != moved)
    {
        _moved = moved;
        setOverride(QString::fromLatin1(DMH_KEY_MOVED), moved);
        emit moveUpdated();
    }
}

void BattleDialogModelCombatant::incrementMoved(qreal moved)
{
    if(moved != 0.0)
    {
        _moved += moved;
        setOverride(QString::fromLatin1(DMH_KEY_MOVED), _moved);
        emit moveUpdated();
    }
}

void BattleDialogModelCombatant::resetMoved()
{
    if(_moved != 0)
    {
        _moved = 0;
        setOverride(QString::fromLatin1(DMH_KEY_MOVED), _moved);
        emit moveUpdated();
    }
}

QVariant BattleDialogModelCombatant::getOverride(const QString& key) const
{
    return _overrides.value(key);
}

bool BattleDialogModelCombatant::hasOverride(const QString& key) const
{
    return _overrides.contains(key);
}

void BattleDialogModelCombatant::setOverride(const QString& key, const QVariant& value)
{
    if(key.isEmpty())
        return;

    const QVariant existing = _overrides.value(key);
    if(existing == value)
        return;

    _overrides.insert(key, value);
    emit overrideChanged(this, key);
    emit dirty();
}

void BattleDialogModelCombatant::clearOverride(const QString& key)
{
    if(_overrides.remove(key))
    {
        emit overrideChanged(this, key);
        emit dirty();
    }
}

QStringList BattleDialogModelCombatant::overrideKeys() const
{
    return _overrides.keys();
}

void BattleDialogModelCombatant::setShown(bool isShown)
{
    if(_isShown != isShown)
    {
        _isShown = isShown;
        setOverride(QString::fromLatin1(DMH_KEY_IS_SHOWN), isShown);
        emit visibilityChanged();
    }
}

void BattleDialogModelCombatant::setKnown(bool isKnown)
{
    if(_isKnown != isKnown)
    {
        _isKnown = isKnown;
        setOverride(QString::fromLatin1(DMH_KEY_IS_KNOWN), isKnown);
        emit visibilityChanged();
    }
}

void BattleDialogModelCombatant::setSelected(bool isSelected)
{
    if(_isSelected != isSelected)
    {
        _isSelected = isSelected;
        emit combatantSelected(this);
    }
}

void BattleDialogModelCombatant::setDone(bool isDone)
{
    if(_isDone != isDone)
    {
        _isDone = isDone;
        setOverride(QString::fromLatin1(DMH_KEY_IS_DONE), isDone);
        emit combatantDoneChanged(this);
    }
}

QDomElement BattleDialogModelCombatant::createOutputXML(QDomDocument &doc)
{
    return doc.createElement("battlecombatant");
}

void BattleDialogModelCombatant::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("combatantId", getCombatant() ? getCombatant()->getID().toString() : QUuid().toString());
    element.setAttribute("type", getCombatantType());
    element.setAttribute("initiative", _initiative);
    element.setAttribute("sort", _sortPosition);
    element.setAttribute("isShown", _isShown);
    element.setAttribute("isKnown", _isKnown);
    element.setAttribute("done", _isDone);

    if(!_groupId.isNull())
        element.setAttribute("groupId", _groupId.toString());

    for(auto it = _overrides.constBegin(); it != _overrides.constEnd(); ++it)
    {
        QDomElement overrideElement = doc.createElement(QString::fromLatin1(OVERRIDE_ELEMENT));
        overrideElement.setAttribute(QString::fromLatin1(OVERRIDE_ATTR_KEY), it.key());
        overrideElement.setAttribute(QString::fromLatin1(OVERRIDE_ATTR_VALUE), it.value().toString());
        element.appendChild(overrideElement);
    }

    BattleDialogModelObject::internalOutputXML(doc, element, targetDirectory, isExport);
}

bool BattleDialogModelCombatant::belongsToObject(QDomElement& element)
{
    Q_UNUSED(element);

    // Don't auto-input any child objects of the battle. The battle will handle this itself.
    return true;
}

void BattleDialogModelCombatant::setCombatant(Combatant* combatant)
{
    _combatant = combatant;
}
