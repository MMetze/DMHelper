#ifndef RULEINITIATIVE_H
#define RULEINITIATIVE_H

#include <QObject>

class BattleDialogModelCombatant;

class RuleInitiative : public QObject
{
    Q_OBJECT
public:
    explicit RuleInitiative(QObject *parent = nullptr);

    virtual QString getInitiativeType();

    virtual bool rollInitiative(QList<BattleDialogModelCombatant*>& combatants);
    virtual void sortInitiative(QList<BattleDialogModelCombatant*>& combatants);

protected:
    void resetCombatantSortValues(QList<BattleDialogModelCombatant*> combatants);

signals:
};

#endif // RULEINITIATIVE_H
