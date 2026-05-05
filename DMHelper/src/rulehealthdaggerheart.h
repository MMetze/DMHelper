#ifndef RULEHEALTHDAGGERHEART_H
#define RULEHEALTHDAGGERHEART_H

#include "rulehealth.h"

class RuleHealthDaggerheart : public RuleHealth
{
    Q_OBJECT
public:
    explicit RuleHealthDaggerheart(QObject* parent = nullptr);

    static QString HealthType;
    static QString HealthDescription;

    virtual QString getHealthType() const override;

    virtual qreal getHealthFraction(const BattleDialogModelCombatant* combatant) const override;

    virtual int  applyDamage(BattleDialogModelCombatant* combatant, int amount) override;
    virtual bool isDead(const BattleDialogModelCombatant* combatant) const override;
    virtual bool isIncapacitated(const BattleDialogModelCombatant* combatant) const override;
    virtual bool healthCountsUp() const override;
    virtual void rollInitial(BattleDialogModelCombatant* combatant) override;
};

#endif // RULEHEALTHDAGGERHEART_H
