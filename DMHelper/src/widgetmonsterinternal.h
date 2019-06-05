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

    virtual BattleDialogModelCombatant* getCombatant();
    virtual QFrame* getFrame();

    virtual int getInitiative() const;
    virtual bool isShown();
    virtual bool isKnown();

    void setLegendaryMaximum(int legendaryMaximum);
    int getLegendaryMaximum() const;

signals:

    void clicked(const QString& monsterClass);

public slots:

    virtual void updateData();
    virtual void setInitiative(int initiative);
    virtual void setHitPoints(int hp);

    void decrementLegendary();
    void resetLegendary();

protected slots:
    virtual void setHighlighted(bool highlighted);

protected:

    // local
    virtual void executeDoubleClick();

    // Data
    WidgetMonster* _widgetParent;
    BattleDialogModelMonsterBase* _monster;

    int _legendaryMaximum;
};

#endif // WIDGETMONSTERINTERNAL_H
