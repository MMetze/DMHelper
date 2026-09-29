#include "combatanttemplateadapter.h"
#include "battledialogmodelcombatant.h"
#include "battledialogmodelmonsterbase.h"
#include "battledialogmodelcharacter.h"
#include "templatefactory.h"
#include "monsterclassv2.h"
#include "characterv2.h"
#include "perroundresource.h"
#include "dmconstants.h"
#include "templateobjectnotifier.h"
#include <QStringList>
#include <QHash>

namespace
{
    bool parseResourcePair(const QString& value, int& current, int& maximum)
    {
        const QString trimmed = value.trimmed();
        if(trimmed.isEmpty())
            return false;

        const QChar separator = trimmed.contains(QLatin1Char('/')) ? QLatin1Char('/') : QLatin1Char(',');
        const QStringList parts = trimmed.split(separator);
        if(parts.size() == 1)
        {
            bool ok = false;
            const int single = parts.at(0).trimmed().toInt(&ok);
            if(!ok)
                return false;
            current = 0;
            maximum = single;
            return true;
        }
        if(parts.size() < 2)
            return false;

        bool okCurrent = false;
        bool okMaximum = false;
        const int parsedCurrent = parts.at(0).trimmed().toInt(&okCurrent);
        const int parsedMaximum = parts.at(1).trimmed().toInt(&okMaximum);
        if((!okCurrent) || (!okMaximum))
            return false;

        current = parsedCurrent;
        maximum = parsedMaximum;
        return true;
    }
}

const char* CombatantTemplateAdapter::CONDITION_KEY_ID        = "conditionId";
const char* CombatantTemplateAdapter::RESOURCE_KEY_NAME       = "name";
const char* CombatantTemplateAdapter::RESOURCE_KEY_MAX        = "max";
const char* CombatantTemplateAdapter::RESOURCE_KEY_CURRENT    = "current";
const char* CombatantTemplateAdapter::RESOURCE_KEY_RECHARGE   = "recharge";

// Legacy alias table. The pre-v3 (unprefixed) attribute names map to their
// canonical dmh:-prefixed equivalents. This table is reserved for
// XML-level compatibility conversion when reading old campaign/bestiary
// files; it MUST NOT be applied to runtime adapter lookups. The whole
// purpose of the dmh: prefix is to keep combatant model state in a separate
// namespace from arbitrary template attributes, and aliasing at lookup time
// would defeat that. Runtime callers (.ui dmhValue bindings, signal
// payloads, etc.) must use the canonical dmh:* key directly.
const QHash<QString, QString>& CombatantTemplateAdapter::legacyAliasTable()
{
    static const QHash<QString, QString> table = {
        { QStringLiteral("initiative"),        QStringLiteral("dmh:initiative") },
        { QStringLiteral("isShown"),           QStringLiteral("dmh:isShown") },
        { QStringLiteral("isKnown"),           QStringLiteral("dmh:isKnown") },
        { QStringLiteral("isDone"),            QStringLiteral("dmh:isDone") },
        { QStringLiteral("hitPoints"),         QStringLiteral("dmh:health") },
        { QStringLiteral("conditions"),        QStringLiteral("dmh:conditions") },
        { QStringLiteral("perRoundResources"), QStringLiteral("dmh:perRoundResources") }
    };
    return table;
}

QString CombatantTemplateAdapter::canonicalKey(const QString& key)
{
    // Identity. Aliasing is reserved for XML compatibility-mode conversion
    // (see legacyAliasTable). Runtime keys must already be canonical.
    return key;
}

CombatantTemplateAdapter::CombatantTemplateAdapter(BattleDialogModelCombatant* combatant, QObject* parent) :
    QObject(parent),
    TemplateObject(nullptr),
    _combatant(combatant),
    _dummyHash()
{
    if(TemplateObject* inner = getInner())
        setFactory(inner->getFactory());

    wireModelToNotifier();
}

CombatantTemplateAdapter::~CombatantTemplateAdapter()
{
}

BattleDialogModelCombatant* CombatantTemplateAdapter::getCombatant() const
{
    return _combatant;
}

TemplateObject* CombatantTemplateAdapter::getInner() const
{
    if(MonsterClassv2* mc = monsterClass())
        return static_cast<TemplateObject*>(mc);
    if(Characterv2* c = character())
        return static_cast<TemplateObject*>(c);
    return nullptr;
}

bool CombatantTemplateAdapter::hasValue(const QString& key) const
{
    const QString k = canonicalKey(key);
    if(isModelKey(k))
        return _combatant != nullptr;
    if(TemplateObject* inner = getInner())
        return inner->hasValue(k);
    return false;
}

QString CombatantTemplateAdapter::getValueAsString(const QString& key) const
{
    return getStringValue(key);
}

QString CombatantTemplateAdapter::getStringValue(const QString& key) const
{
    if(!_combatant)
        return QString();

    const QString k = canonicalKey(key);
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_NAME))
        return _combatant->getName();
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_INITIATIVE))
        return QString::number(_combatant->getInitiative());
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_MOVED))
        return QString::number(qRound(_combatant->getMoved()));
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_IS_SHOWN))
        return _combatant->getShown() ? QStringLiteral("1") : QStringLiteral("0");
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_IS_KNOWN))
        return _combatant->getKnown() ? QStringLiteral("1") : QStringLiteral("0");
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_IS_DONE))
        return _combatant->getDone() ? QStringLiteral("1") : QStringLiteral("0");
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_HEALTH))
        return QString::number(_combatant->getHitPoints());
    if(isResourceKey(k))
    {
        return resourceStorageString(k);
    }
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_PER_ROUND_RESOURCES))
    {
        if(TemplateObject* inner = getInner())
            return inner->getValueAsString(k);
        return QString();
    }
    // Armor class is read-only and lives entirely on the underlying class /
    // character sheet; there is no canonical dmh: key for it.
    if(key == QLatin1String("armorClass"))
        return QString::number(_combatant->getArmorClass());

    if(TemplateObject* inner = getInner())
        return inner->getStringValue(k);
    return QString();
}

int CombatantTemplateAdapter::getIntValue(const QString& key) const
{
    if(!_combatant)
        return 0;

    const QString k = canonicalKey(key);
    if(isResourceKey(k))
    {
        const QString resourceValue = getStringValue(k);
        return resourceValue.section(QLatin1Char(','), 0, 0).trimmed().toInt();
    }
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_INITIATIVE))
        return _combatant->getInitiative();
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_MOVED))
        return static_cast<int>(_combatant->getMoved());
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_HEALTH))
        return _combatant->getHitPoints();
    if(key == QLatin1String("armorClass"))
        return _combatant->getArmorClass();
    if(isModelKey(k))
        return getStringValue(k).toInt();

    if(TemplateObject* inner = getInner())
        return inner->getIntValue(k);
    return 0;
}

bool CombatantTemplateAdapter::getBoolValue(const QString& key) const
{
    if(!_combatant)
        return false;

    const QString k = canonicalKey(key);
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_IS_SHOWN))
        return _combatant->getShown();
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_IS_KNOWN))
        return _combatant->getKnown();
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_IS_DONE))
        return _combatant->getDone();

    if(TemplateObject* inner = getInner())
        return inner->getBoolValue(k);
    return false;
}

QList<QVariant> CombatantTemplateAdapter::getListValue(const QString& key) const
{
    if(!_combatant)
        return QList<QVariant>();

    const QString k = canonicalKey(key);
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_CONDITIONS))
    {
        QList<QVariant> result;
        const QStringList conditions = _combatant->getConditionList();
        for(const QString& conditionId : conditions)
        {
            QHash<QString, QVariant> entry;
            entry.insert(QString::fromLatin1(CONDITION_KEY_ID), conditionId);
            result.append(entry);
        }
        return result;
    }

    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_PER_ROUND_RESOURCES))
    {
        QList<QVariant> result;
        MonsterClassv2* mc = monsterClass();
        BattleDialogModelMonsterBase* mb = monsterBase();
        if(!mc)
            return result;

        const QList<PerRoundResource> resources = mc->getPerRoundResources();
        for(const PerRoundResource& res : resources)
        {
            QHash<QString, QVariant> entry;
            entry.insert(QString::fromLatin1(RESOURCE_KEY_NAME),     res.name);
            entry.insert(QString::fromLatin1(RESOURCE_KEY_MAX),      res.max);
            entry.insert(QString::fromLatin1(RESOURCE_KEY_RECHARGE), res.recharge);
            const int current = (mb && mb->hasResourceCounter(res.name)) ? mb->getResourceCount(res.name) : res.max;
            entry.insert(QString::fromLatin1(RESOURCE_KEY_CURRENT),  current);
            result.append(entry);
        }
        return result;
    }

    if(TemplateObject* inner = getInner())
        return inner->getListValue(k);
    return QList<QVariant>();
}

void CombatantTemplateAdapter::setValue(const QString& key, const QVariant& value)
{
    setValue(key, value.toString());
}

void CombatantTemplateAdapter::setValue(const QString& key, const QString& value)
{
    if(!_combatant)
        return;

    const QString k = canonicalKey(key);
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_NAME))
    {
        _combatant->setName(value);
        return;
    }
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_INITIATIVE))
    {
        _combatant->setInitiative(value.toInt());
        return;
    }
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_MOVED))
    {
        _combatant->setMoved(value.toDouble());
        return;
    }
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_IS_SHOWN))
    {
        _combatant->setShown((value == QLatin1String("1")) || (value.toLower() == QLatin1String("true")));
        return;
    }
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_IS_KNOWN))
    {
        _combatant->setKnown((value == QLatin1String("1")) || (value.toLower() == QLatin1String("true")));
        return;
    }
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_IS_DONE))
    {
        _combatant->setDone((value == QLatin1String("1")) || (value.toLower() == QLatin1String("true")));
        return;
    }
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_HEALTH))
    {
        _combatant->setHitPoints(value.toInt());
        return;
    }
    if(isResourceKey(k))
    {
        _combatant->setOverride(k, normalizeResourceStorageString(k, value));
        return;
    }

    if(TemplateObject* inner = getInner())
        inner->setValue(k, value);
}

void CombatantTemplateAdapter::setListValue(const QString& key, int index, const QString& listEntryKey, const QVariant& listEntryValue)
{
    if(!_combatant)
        return;

    const QString k = canonicalKey(key);
    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_PER_ROUND_RESOURCES))
    {
        BattleDialogModelMonsterBase* mb = monsterBase();
        MonsterClassv2* mc = monsterClass();
        if((!mb) || (!mc))
            return;

        const QList<PerRoundResource> resources = mc->getPerRoundResources();
        if((index < 0) || (index >= resources.size()))
            return;

        const PerRoundResource& res = resources.at(index);
        if(listEntryKey == QLatin1String(RESOURCE_KEY_CURRENT))
            mb->setResourceCount(res.name, listEntryValue.toInt());
        // Other per-resource keys (name/max/recharge) are definition-side and
        // owned by the MonsterClassv2; ignore writes from the combatant widget.
        return;
    }

    if(k == QLatin1String(BattleDialogModelCombatant::DMH_KEY_CONDITIONS))
    {
        // Conditions are managed via add/remove on the model, not in-place edits.
        return;
    }

    if(TemplateObject* inner = getInner())
        inner->setListValue(k, index, listEntryKey, listEntryValue);
}

void CombatantTemplateAdapter::setListValue(const QString& key, int index, const QString& listEntryKey, const QString& listEntryValue)
{
    setListValue(key, index, listEntryKey, QVariant(listEntryValue));
}

QHash<QString, QVariant>* CombatantTemplateAdapter::valueHash()
{
    return &_dummyHash;
}

const QHash<QString, QVariant>* CombatantTemplateAdapter::valueHash() const
{
    return &_dummyHash;
}

void CombatantTemplateAdapter::declareDirty()
{
    // No-op. Underlying model objects mark themselves dirty via their own
    // setters when the adapter forwards a write.
}

void CombatantTemplateAdapter::wireModelToNotifier()
{
    if(_combatant)
    {
        connect(_combatant, &BattleDialogModelCombatant::initiativeChanged,    this, &CombatantTemplateAdapter::onCombatantInitiativeChanged);
        connect(_combatant, &BattleDialogModelCombatant::conditionsChanged,    this, &CombatantTemplateAdapter::onCombatantConditionsChanged);
        connect(_combatant, &BattleDialogModelCombatant::moveUpdated,          this, &CombatantTemplateAdapter::onCombatantMoveUpdated);
        connect(_combatant, &BattleDialogModelCombatant::visibilityChanged,    this, &CombatantTemplateAdapter::onCombatantVisibilityChanged);
        connect(_combatant, &BattleDialogModelCombatant::combatantDoneChanged, this, &CombatantTemplateAdapter::onCombatantDoneChanged);
        connect(_combatant, &BattleDialogModelCombatant::overrideChanged,      this, &CombatantTemplateAdapter::onCombatantOverrideChanged);
    }

    if(BattleDialogModelMonsterBase* mb = monsterBase())
    {
        connect(mb, &BattleDialogModelMonsterBase::dataChanged,          this, &CombatantTemplateAdapter::onMonsterDataChanged);
        connect(mb, &BattleDialogModelMonsterBase::resourceCountChanged, this, &CombatantTemplateAdapter::onMonsterResourceCountChanged);
    }

    // Forward inner template-object value changes (e.g. monster class
    // attributes edited via another widget) through this adapter's notifier
    // so widgets bound through the adapter pick them up.
    if(TemplateObject* inner = getInner())
    {
        if(TemplateObjectNotifier* innerNotifier = inner->notifier())
            connect(innerNotifier, &TemplateObjectNotifier::valueChanged, this, &CombatantTemplateAdapter::onInnerValueChanged);
    }
}

void CombatantTemplateAdapter::onCombatantInitiativeChanged()
{
    emit notifier()->valueChanged(QString::fromLatin1(BattleDialogModelCombatant::DMH_KEY_INITIATIVE));
}

void CombatantTemplateAdapter::onCombatantConditionsChanged()
{
    emit notifier()->valueChanged(QString::fromLatin1(BattleDialogModelCombatant::DMH_KEY_CONDITIONS));
}

void CombatantTemplateAdapter::onCombatantMoveUpdated()
{
    emit notifier()->valueChanged(QString::fromLatin1(BattleDialogModelCombatant::DMH_KEY_MOVED));
}

void CombatantTemplateAdapter::onCombatantVisibilityChanged()
{
    TemplateObjectNotifier* n = notifier();
    emit n->valueChanged(QString::fromLatin1(BattleDialogModelCombatant::DMH_KEY_IS_SHOWN));
    emit n->valueChanged(QString::fromLatin1(BattleDialogModelCombatant::DMH_KEY_IS_KNOWN));
}

void CombatantTemplateAdapter::onCombatantDoneChanged()
{
    emit notifier()->valueChanged(QString::fromLatin1(BattleDialogModelCombatant::DMH_KEY_IS_DONE));
}

void CombatantTemplateAdapter::onCombatantOverrideChanged(BattleDialogModelCombatant* combatant, const QString& key)
{
    Q_UNUSED(combatant);
    emit notifier()->valueChanged(key);
}

void CombatantTemplateAdapter::onMonsterDataChanged()
{
    // dataChanged is a coarse "something on the model changed" signal. Emit
    // the model-side keys most likely to be reflected in templates so any
    // bound widgets refresh. Fields that have their own dedicated signals
    // (initiative / done / shown / known / move / conditions) are NOT
    // re-emitted here to avoid double-updates.
    TemplateObjectNotifier* n = notifier();
    emit n->valueChanged(QString::fromLatin1(BattleDialogModelCombatant::DMH_KEY_HEALTH));
    emit n->valueChanged(QStringLiteral("armorClass"));
    emit n->valueChanged(QString::fromLatin1(BattleDialogModelCombatant::DMH_KEY_NAME));
}

void CombatantTemplateAdapter::onMonsterResourceCountChanged(BattleDialogModelMonsterBase* monster, const QString& resourceName, int newValue)
{
    Q_UNUSED(monster);
    Q_UNUSED(resourceName);
    Q_UNUSED(newValue);
    emit notifier()->valueChanged(QString::fromLatin1(BattleDialogModelCombatant::DMH_KEY_PER_ROUND_RESOURCES));
}

void CombatantTemplateAdapter::onInnerValueChanged(const QString& innerKey)
{
    // Pass through unchanged. Inner keys are non-model (no dmh: prefix), so
    // they cannot collide with the adapter's own model-key namespace.
    emit notifier()->valueChanged(innerKey);
}

bool CombatantTemplateAdapter::isModelKey(const QString& key) const
{
    // Recognises only canonical (dmh:-prefixed) keys plus the unprefixed
    // "armorClass" carve-out. Unprefixed legacy names should be converted at
    // XML compatibility-mode read sites via legacyAliasTable() — never here.
    return (key == QLatin1String(BattleDialogModelCombatant::DMH_KEY_NAME))
        || (key == QLatin1String(BattleDialogModelCombatant::DMH_KEY_INITIATIVE))
        || (key == QLatin1String(BattleDialogModelCombatant::DMH_KEY_MOVED))
        || (key == QLatin1String(BattleDialogModelCombatant::DMH_KEY_IS_SHOWN))
        || (key == QLatin1String(BattleDialogModelCombatant::DMH_KEY_IS_KNOWN))
        || (key == QLatin1String(BattleDialogModelCombatant::DMH_KEY_IS_DONE))
        || (key == QLatin1String(BattleDialogModelCombatant::DMH_KEY_HEALTH))
        || isResourceKey(key)
        || (key == QLatin1String(BattleDialogModelCombatant::DMH_KEY_CONDITIONS))
        || (key == QLatin1String(BattleDialogModelCombatant::DMH_KEY_PER_ROUND_RESOURCES))
        || (key == QLatin1String("armorClass"));
}

bool CombatantTemplateAdapter::isResourceKey(const QString& key) const
{
    TemplateObject* inner = getInner();
    if((!inner) || (!inner->getFactory()) || (!inner->getFactory()->hasAttribute(key)))
    {
        const QString templateKey = key.startsWith(QStringLiteral("dmh:")) ? key.mid(4) : key;
        if((templateKey.isEmpty()) || (!inner->getFactory()->hasAttribute(templateKey)))
            return false;

        return inner->getFactory()->getAttribute(templateKey)._type == TemplateFactory::TemplateType_resource;
    }

    return inner->getFactory()->getAttribute(key)._type == TemplateFactory::TemplateType_resource;
}

QString CombatantTemplateAdapter::resourceStorageString(const QString& key) const
{
    TemplateObject* inner = getInner();
    if(!inner)
        return QString();

    const QString templateKey = key.startsWith(QStringLiteral("dmh:")) ? key.mid(4) : key;
    const ResourcePair basePair = inner->getResourceValue(templateKey);

    if(_combatant && _combatant->hasOverride(key))
    {
        const QString overrideString = _combatant->getOverride(key).toString();
        int overrideCurrent = 0;
        int overrideMaximum = 0;
        if(parseResourcePair(overrideString, overrideCurrent, overrideMaximum))
            return QString::number(overrideCurrent) + QStringLiteral(",") + QString::number(overrideMaximum);

        bool ok = false;
        const int overrideCurrentOnly = overrideString.toInt(&ok);
        if(ok)
            return QString::number(overrideCurrentOnly) + QStringLiteral(",") + QString::number(basePair.second);
    }

    return QString::number(basePair.first) + QStringLiteral(",") + QString::number(basePair.second);
}

QString CombatantTemplateAdapter::normalizeResourceStorageString(const QString& key, const QString& value) const
{
    TemplateObject* inner = getInner();
    const QString templateKey = key.startsWith(QStringLiteral("dmh:")) ? key.mid(4) : key;
    const ResourcePair basePair = inner ? inner->getResourceValue(templateKey) : ResourcePair();

    int current = 0;
    int maximum = 0;
    if(parseResourcePair(value, current, maximum))
        return QString::number(current) + QStringLiteral(",") + QString::number(maximum);

    bool ok = false;
    const int singleValue = value.trimmed().toInt(&ok);
    if(ok)
        return QString::number(singleValue) + QStringLiteral(",") + QString::number(basePair.second);

    return QString::number(basePair.first) + QStringLiteral(",") + QString::number(basePair.second);
}

BattleDialogModelMonsterBase* CombatantTemplateAdapter::monsterBase() const
{
    return dynamic_cast<BattleDialogModelMonsterBase*>(_combatant);
}

BattleDialogModelCharacter* CombatantTemplateAdapter::characterModel() const
{
    return dynamic_cast<BattleDialogModelCharacter*>(_combatant);
}

MonsterClassv2* CombatantTemplateAdapter::monsterClass() const
{
    if(BattleDialogModelMonsterBase* mb = monsterBase())
        return mb->getMonsterClass();
    return nullptr;
}

Characterv2* CombatantTemplateAdapter::character() const
{
    if(BattleDialogModelCharacter* cm = characterModel())
        return cm->getCharacter();
    return nullptr;
}
