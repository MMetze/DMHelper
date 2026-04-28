#include "combatanttemplateadapter.h"
#include "battledialogmodelcombatant.h"
#include "battledialogmodelmonsterbase.h"
#include "battledialogmodelcharacter.h"
#include "monsterclassv2.h"
#include "characterv2.h"
#include "perroundresource.h"
#include "dmconstants.h"
#include <QStringList>

const char* CombatantTemplateAdapter::KEY_NAME                = "name";
const char* CombatantTemplateAdapter::KEY_INITIATIVE          = "initiative";
const char* CombatantTemplateAdapter::KEY_MOVED               = "moved";
const char* CombatantTemplateAdapter::KEY_IS_SHOWN            = "isShown";
const char* CombatantTemplateAdapter::KEY_IS_KNOWN            = "isKnown";
const char* CombatantTemplateAdapter::KEY_IS_DONE             = "isDone";
const char* CombatantTemplateAdapter::KEY_HP                  = "hp";
const char* CombatantTemplateAdapter::KEY_AC                  = "ac";
const char* CombatantTemplateAdapter::KEY_CONDITIONS          = "conditions";
const char* CombatantTemplateAdapter::KEY_PER_ROUND_RESOURCES = "perRoundResources";

const char* CombatantTemplateAdapter::CONDITION_KEY_ID        = "conditionId";
const char* CombatantTemplateAdapter::RESOURCE_KEY_NAME       = "name";
const char* CombatantTemplateAdapter::RESOURCE_KEY_MAX        = "max";
const char* CombatantTemplateAdapter::RESOURCE_KEY_CURRENT    = "current";
const char* CombatantTemplateAdapter::RESOURCE_KEY_RECHARGE   = "recharge";

CombatantTemplateAdapter::CombatantTemplateAdapter(BattleDialogModelCombatant* combatant, QObject* parent) :
    QObject(parent),
    TemplateObject(nullptr),
    _combatant(combatant),
    _dummyHash()
{
    if(TemplateObject* inner = getInner())
        setFactory(inner->getFactory());
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
    if(isModelKey(key))
        return _combatant != nullptr;
    if(TemplateObject* inner = getInner())
        return inner->hasValue(key);
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

    if(key == QLatin1String(KEY_NAME))
        return _combatant->getName();
    if(key == QLatin1String(KEY_INITIATIVE))
        return QString::number(_combatant->getInitiative());
    if(key == QLatin1String(KEY_MOVED))
        return QString::number(_combatant->getMoved(), 'g', -1);
    if(key == QLatin1String(KEY_IS_SHOWN))
        return _combatant->getShown() ? QStringLiteral("1") : QStringLiteral("0");
    if(key == QLatin1String(KEY_IS_KNOWN))
        return _combatant->getKnown() ? QStringLiteral("1") : QStringLiteral("0");
    if(key == QLatin1String(KEY_IS_DONE))
        return _combatant->getDone() ? QStringLiteral("1") : QStringLiteral("0");
    if(key == QLatin1String(KEY_HP))
        return QString::number(_combatant->getHitPoints());
    if(key == QLatin1String(KEY_AC))
        return QString::number(_combatant->getArmorClass());

    if(TemplateObject* inner = getInner())
        return inner->getStringValue(key);
    return QString();
}

int CombatantTemplateAdapter::getIntValue(const QString& key) const
{
    if(!_combatant)
        return 0;

    if(key == QLatin1String(KEY_INITIATIVE))
        return _combatant->getInitiative();
    if(key == QLatin1String(KEY_MOVED))
        return static_cast<int>(_combatant->getMoved());
    if(key == QLatin1String(KEY_HP))
        return _combatant->getHitPoints();
    if(key == QLatin1String(KEY_AC))
        return _combatant->getArmorClass();
    if(isModelKey(key))
        return getStringValue(key).toInt();

    if(TemplateObject* inner = getInner())
        return inner->getIntValue(key);
    return 0;
}

bool CombatantTemplateAdapter::getBoolValue(const QString& key) const
{
    if(!_combatant)
        return false;

    if(key == QLatin1String(KEY_IS_SHOWN))
        return _combatant->getShown();
    if(key == QLatin1String(KEY_IS_KNOWN))
        return _combatant->getKnown();
    if(key == QLatin1String(KEY_IS_DONE))
        return _combatant->getDone();

    if(TemplateObject* inner = getInner())
        return inner->getBoolValue(key);
    return false;
}

QList<QVariant> CombatantTemplateAdapter::getListValue(const QString& key) const
{
    if(!_combatant)
        return QList<QVariant>();

    if(key == QLatin1String(KEY_CONDITIONS))
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

    if(key == QLatin1String(KEY_PER_ROUND_RESOURCES))
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
        return inner->getListValue(key);
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

    if(key == QLatin1String(KEY_NAME))
    {
        _combatant->setName(value);
        return;
    }
    if(key == QLatin1String(KEY_INITIATIVE))
    {
        _combatant->setInitiative(value.toInt());
        return;
    }
    if(key == QLatin1String(KEY_MOVED))
    {
        _combatant->setMoved(value.toDouble());
        return;
    }
    if(key == QLatin1String(KEY_IS_SHOWN))
    {
        _combatant->setShown((value == QLatin1String("1")) || (value.toLower() == QLatin1String("true")));
        return;
    }
    if(key == QLatin1String(KEY_IS_KNOWN))
    {
        _combatant->setKnown((value == QLatin1String("1")) || (value.toLower() == QLatin1String("true")));
        return;
    }
    if(key == QLatin1String(KEY_IS_DONE))
    {
        _combatant->setDone((value == QLatin1String("1")) || (value.toLower() == QLatin1String("true")));
        return;
    }
    if(key == QLatin1String(KEY_HP))
    {
        _combatant->setHitPoints(value.toInt());
        return;
    }

    if(TemplateObject* inner = getInner())
        inner->setValue(key, value);
}

void CombatantTemplateAdapter::setListValue(const QString& key, int index, const QString& listEntryKey, const QVariant& listEntryValue)
{
    if(!_combatant)
        return;

    if(key == QLatin1String(KEY_PER_ROUND_RESOURCES))
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

    if(key == QLatin1String(KEY_CONDITIONS))
    {
        // Conditions are managed via add/remove on the model, not in-place edits.
        return;
    }

    if(TemplateObject* inner = getInner())
        inner->setListValue(key, index, listEntryKey, listEntryValue);
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

bool CombatantTemplateAdapter::isModelKey(const QString& key) const
{
    return (key == QLatin1String(KEY_NAME))
        || (key == QLatin1String(KEY_INITIATIVE))
        || (key == QLatin1String(KEY_MOVED))
        || (key == QLatin1String(KEY_IS_SHOWN))
        || (key == QLatin1String(KEY_IS_KNOWN))
        || (key == QLatin1String(KEY_IS_DONE))
        || (key == QLatin1String(KEY_HP))
        || (key == QLatin1String(KEY_AC))
        || (key == QLatin1String(KEY_CONDITIONS))
        || (key == QLatin1String(KEY_PER_ROUND_RESOURCES));
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
