#include "rulehealth.h"
#include "battledialogmodelcombatant.h"
#include "battledialogmodelmonsterbase.h"
#include "battledialogmodelmonstercombatant.h"
#include "battledialogmodelcharacter.h"
#include "monsterclassv2.h"
#include "characterv2.h"
#include "templateobject.h"
#include "campaign.h"
#include "ruleset.h"
#include "dmconstants.h"
#include "layertokens.h"
#include "layerscene.h"
#include "combatanttemplateadapter.h"
#include "templatefactory.h"

// Fallback keys used when no campaign/ruleset is available (e.g. unit-test combatants).
static const QString FALLBACK_CHARACTER_MAX_HP_KEY = QStringLiteral("maximumHp");
static const QString FALLBACK_MONSTER_HP_KEY       = QStringLiteral("hit_points");

RuleHealth::RuleHealth(QObject* parent) :
    QObject{parent}
{
}

QString RuleHealth::getHealthType() const
{
    return QString();
}

int RuleHealth::getHealth(const BattleDialogModelCombatant* combatant) const
{
    if(!combatant)
        return 0;

    const QString resourceKey = resourceHealthKey(combatant);
    if(!resourceKey.isEmpty())
        return CombatantTemplateAdapter::resourceValueFor(combatant, templateFor(combatant), resourceKey).first;

    const QString healthKey = QString::fromLatin1(BattleDialogModelCombatant::DMH_KEY_HEALTH);
    if(combatant->hasOverride(healthKey))
    {
        bool ok = false;
        const int value = combatant->getOverride(healthKey).toInt(&ok);
        if(ok)
            return value;
    }

    return combatant->getHitPoints();
}

int RuleHealth::getMaxHealth(const BattleDialogModelCombatant* combatant) const
{
    const QString resourceKey = resourceHealthKey(combatant);
    if(!resourceKey.isEmpty())
        return CombatantTemplateAdapter::resourceValueFor(combatant, templateFor(combatant), resourceKey).second;

    const BattleDialogModelMonsterBase* monsterBase = dynamic_cast<const BattleDialogModelMonsterBase*>(combatant);
    if(monsterBase && monsterBase->getMonsterMaxHP() > 0)
        return monsterBase->getMonsterMaxHP();

    const TemplateObject* tmpl = templateFor(combatant);
    if(!tmpl)
        return 0;

    const QString key = maxHpKeyFor(combatant);
    if(tmpl->hasValue(key))
        return tmpl->getIntValue(key);

    return 0;
}

void RuleHealth::setHealth(BattleDialogModelCombatant* combatant, int value)
{
    if(!combatant)
        return;

    const QString key = resourceHealthKey(combatant);
    if(key.isEmpty())
    {
        combatant->setHitPoints(value);
        return;
    }

    combatant->setOverride(key, QString::number(value) + QStringLiteral(",") + QString::number(getMaxHealth(combatant)));
}

QString RuleHealth::resourceHealthKey(const BattleDialogModelCombatant* combatant) const
{
    const TemplateObject* source = templateFor(combatant);
    const TemplateFactory* factory = source ? source->getFactory() : nullptr;
    const QString key = currentHpKeyFor(combatant);
    if(factory && factory->hasAttribute(key) && factory->getAttribute(key)._type == TemplateFactory::TemplateType_resource)
        return key;
    return QString();
}

qreal RuleHealth::getHealthFraction(const BattleDialogModelCombatant* combatant) const
{
    const int max = getMaxHealth(combatant);
    if(max <= 0)
        return 0.0;

    const qreal fraction = static_cast<qreal>(getHealth(combatant)) / static_cast<qreal>(max);
    return qBound(static_cast<qreal>(0.0), fraction, static_cast<qreal>(1.0));
}

bool RuleHealth::hasDeathSaves() const
{
    return false;
}

TemplateObject* RuleHealth::templateFor(const BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return nullptr;

    if(const BattleDialogModelMonsterBase* mb = dynamic_cast<const BattleDialogModelMonsterBase*>(combatant))
        return mb->getMonsterClass();

    if(const BattleDialogModelCharacter* cm = dynamic_cast<const BattleDialogModelCharacter*>(combatant))
        return cm->getCharacter();

    return nullptr;
}

RuleHealth* RuleHealth::forCombatant(const BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return nullptr;

    LayerTokens* tokenLayer = combatant->getLayer();
    if(!tokenLayer)
        return nullptr;

    LayerScene* scene = tokenLayer->getLayerScene();
    if(!scene)
        return nullptr;

    const Campaign* campaign = dynamic_cast<const Campaign*>(scene->getParentByType(DMHelper::CampaignType_Campaign));
    if(!campaign)
        return nullptr;

    return const_cast<Campaign*>(campaign)->getRuleset().getRuleHealth();
}

QString RuleHealth::maxHpKeyFor(const BattleDialogModelCombatant* combatant) const
{
    if(!combatant)
        return FALLBACK_CHARACTER_MAX_HP_KEY;

    const bool isMonster = (dynamic_cast<const BattleDialogModelMonsterBase*>(combatant) != nullptr);

    if(RuleHealth::forCombatant(combatant))
    {
        const Campaign* campaign = dynamic_cast<const Campaign*>(combatant->getLayer()->getLayerScene()->getParentByType(DMHelper::CampaignType_Campaign));
        const Ruleset& ruleset = const_cast<Campaign*>(campaign)->getRuleset();
        return isMonster ? ruleset.getMonsterMaxHpKey() : ruleset.getCharacterMaxHpKey();
    }

    const Combatant* baseCombatant = combatant->getCombatant();
    if(!baseCombatant)
        return isMonster ? FALLBACK_MONSTER_HP_KEY : FALLBACK_CHARACTER_MAX_HP_KEY;

    const Campaign* campaign = dynamic_cast<const Campaign*>(baseCombatant->getParentByType(DMHelper::CampaignType_Campaign));
    if(campaign)
    {
        const Ruleset& ruleset = const_cast<Campaign*>(campaign)->getRuleset();
        return isMonster ? ruleset.getMonsterMaxHpKey() : ruleset.getCharacterMaxHpKey();
    }

    return isMonster ? FALLBACK_MONSTER_HP_KEY : FALLBACK_CHARACTER_MAX_HP_KEY;
}

QString RuleHealth::currentHpKeyFor(const BattleDialogModelCombatant* combatant) const
{
    const bool isMonster = dynamic_cast<const BattleDialogModelMonsterBase*>(combatant) != nullptr;
    const Campaign* campaign = nullptr;
    if(combatant && combatant->getLayer() && combatant->getLayer()->getLayerScene())
        campaign = dynamic_cast<const Campaign*>(combatant->getLayer()->getLayerScene()->getParentByType(DMHelper::CampaignType_Campaign));
    if(!campaign && combatant && combatant->getCombatant())
        campaign = dynamic_cast<const Campaign*>(combatant->getCombatant()->getParentByType(DMHelper::CampaignType_Campaign));
    if(campaign)
    {
        const Ruleset& ruleset = const_cast<Campaign*>(campaign)->getRuleset();
        return isMonster ? ruleset.getMonsterCurrentHpKey() : ruleset.getCharacterCurrentHpKey();
    }
    return FALLBACK_MONSTER_HP_KEY;
}

bool RuleHealth::combatantIsDead(const BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return false;
    if(RuleHealth* health = forCombatant(combatant))
        return health->isDead(combatant);
    return combatant->getHitPoints() <= 0;
}

bool RuleHealth::combatantIsUnconscious(const BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return false;
    if(combatant->hasConditionId(QStringLiteral("unconscious")))
        return true;
    if(combatant->getCombatantType() != DMHelper::CombatantType_Character)
        return false;
    if(RuleHealth* health = forCombatant(combatant))
        return health->isIncapacitated(combatant);
    return combatant->getHitPoints() <= 0;
}
