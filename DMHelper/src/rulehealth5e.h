#ifndef RULEHEALTH5E_H
#define RULEHEALTH5E_H

#include "rulehealth.h"

class RuleHealth5e : public RuleHealth
{
    Q_OBJECT
public:
    explicit RuleHealth5e(QObject* parent = nullptr);

    static QString HealthType;
    static QString HealthDescription;

    virtual QString getHealthType() const override;

    virtual int  applyDamage(BattleDialogModelCombatant* combatant, int amount) override;
    virtual bool isDead(const BattleDialogModelCombatant* combatant) const override;
    virtual bool isIncapacitated(const BattleDialogModelCombatant* combatant) const override;
    virtual bool hasDeathSaves() const override;
    virtual bool healthCountsUp() const override;
    virtual void rollInitial(BattleDialogModelCombatant* combatant) override;
};

#endif // RULEHEALTH5E_H
