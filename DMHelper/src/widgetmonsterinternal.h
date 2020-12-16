#ifndef WIDGETMONSTERINTERNAL_H
#define WIDGETMONSTERINTERNAL_H

#include "widgetcombatantbase.h"

class BattleDialogModelMonsterBase;
class QTimer;
class WidgetMonster;

class WidgetMonsterInternal : public WidgetCombatantBase
{
    Q_OBJECT
public:
    explicit WidgetMonsterInternal(BattleDialogModelMonsterBase* monster, WidgetMonster* parent);

    virtual BattleDialogModelCombatant* getCombatant() override;
    virtual QFrame* getFrame() override;

    virtual int getInitiative() const override;
    virtual bool isShown() override;
    virtual bool isKnown() override;

    void setLegendaryMaximum(int legendaryMaximum);
    int getLegendaryMaximum() const;

signals:

    void clicked(const QString& monsterClass);

public slots:

    virtual void updateData() override;
    virtual void setInitiative(int initiative) override;
    virtual void setHitPoints(int hp) override;

    void decrementLegendary();
    void resetLegendary();

protected:

    // local
    virtual void executeDoubleClick() override;

    // Data
    WidgetMonster* _widgetParent;
    BattleDialogModelMonsterBase* _monster;

    int _legendaryMaximum;
};

#endif // WIDGETMONSTERINTERNAL_H
