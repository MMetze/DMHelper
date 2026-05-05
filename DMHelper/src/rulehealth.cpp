#include "rulehealth.h"
#include "battledialogmodelcombatant.h"
#include "battledialogmodelmonsterbase.h"
#include "battledialogmodelcharacter.h"
#include "monsterclassv2.h"
#include "characterv2.h"
#include "templateobject.h"
#include "campaign.h"
#include "ruleset.h"
#include "dmconstants.h"

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
    const TemplateObject* tmpl = templateFor(combatant);
    if(!tmpl)
        return 0;

    const QString key = maxHpKeyFor(combatant);
    if(tmpl->hasValue(key))
        return tmpl->getIntValue(key);

    return 0;
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

    CampaignObjectBase* parent = const_cast<BattleDialogModelCombatant*>(combatant)->getParentByType(DMHelper::CampaignType_Campaign);
    Campaign* campaign = dynamic_cast<Campaign*>(parent);
    if(!campaign)
        return nullptr;

    return campaign->getRuleset().getRuleHealth();
}

QString RuleHealth::maxHpKeyFor(const BattleDialogModelCombatant* combatant) const
{
    if(!combatant)
        return FALLBACK_CHARACTER_MAX_HP_KEY;

    const bool isMonster = (dynamic_cast<const BattleDialogModelMonsterBase*>(combatant) != nullptr);

    CampaignObjectBase* parent = const_cast<BattleDialogModelCombatant*>(combatant)->getParentByType(DMHelper::CampaignType_Campaign);
    Campaign* campaign = dynamic_cast<Campaign*>(parent);
    if(campaign)
    {
        const Ruleset& ruleset = campaign->getRuleset();
        return isMonster ? ruleset.getMonsterMaxHpKey() : ruleset.getCharacterMaxHpKey();
    }

    return isMonster ? FALLBACK_MONSTER_HP_KEY : FALLBACK_CHARACTER_MAX_HP_KEY;
}
