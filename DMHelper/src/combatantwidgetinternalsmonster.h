#ifndef COMBATANTWIDGETINTERNALSMONSTER_H
#define COMBATANTWIDGETINTERNALSMONSTER_H

#include "combatantwidgetinternals.h"

class BattleDialogModelMonsterBase;
class QTimer;
class CombatantWidgetMonster;

class CombatantWidgetInternalsMonster : public CombatantWidgetInternals
{
    Q_OBJECT
public:
    explicit CombatantWidgetInternalsMonster(BattleDialogModelMonsterBase* monster, CombatantWidgetMonster* parent);

    virtual BattleDialogModelCombatant* getCombatant() override;
    virtual QFrame* getFrame() override;

    virtual int getInitiative() const override;
    virtual bool isShown() override;
    virtual bool isKnown() override;

signals:
    void clicked(const QString& monsterClass);

public slots:
    virtual void updateData() override;
    virtual void updateImage() override;
    virtual void setInitiative(int initiative) override;
    virtual void setHitPoints(int hp) override;
    virtual void executeDoubleClick() override;

    void decrementLegendary();
    void resetLegendary();

    void setMonsterName(const QString& monsterName);

protected:

    // Data
    CombatantWidgetMonster* _widgetParent;
    BattleDialogModelMonsterBase* _monster;
};

#endif // COMBATANTWIDGETINTERNALSMONSTER_H
