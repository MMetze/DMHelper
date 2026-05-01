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

static const QString KEY_HIT_POINTS = QStringLiteral("hit_points");

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

    if(tmpl->hasValue(KEY_HIT_POINTS))
        return tmpl->getIntValue(KEY_HIT_POINTS);

    return 0;
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
