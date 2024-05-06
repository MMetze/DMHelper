#ifndef COMBATANTWIDGETINTERNALSCHARACTER_H
#define COMBATANTWIDGETINTERNALSCHARACTER_H

#include "combatantwidgetinternals.h"
#include <QUuid>

class BattleDialogModelCharacter;
class QTimer;
class CombatantWidgetCharacter;

class CombatantWidgetInternalsCharacter : public CombatantWidgetInternals
{
    Q_OBJECT
public:
    explicit CombatantWidgetInternalsCharacter(BattleDialogModelCharacter* character, CombatantWidgetCharacter *parent);

    virtual BattleDialogModelCombatant* getCombatant() override;
    virtual QFrame* getFrame() override;

    virtual int getInitiative() const override;
    virtual bool isShown() override;
    virtual bool isKnown() override;

signals:
    void clicked(QUuid characterID);

public slots:
    virtual void updateData() override;
    virtual void updateImage() override;
    virtual void setInitiative(int initiative) override;
    virtual void setHitPoints(int hp) override;
    virtual void executeDoubleClick() override;

protected:

    // Data
    CombatantWidgetCharacter* _widgetParent;
    BattleDialogModelCharacter* _character;
};

#endif // COMBATANTWIDGETINTERNALSCHARACTER_H
