#ifndef WIDGETCHARACTERINTERNAL_H
#define WIDGETCHARACTERINTERNAL_H

#include "widgetcombatantbase.h"
#include <QUuid>

class BattleDialogModelCharacter;
class QTimer;
class WidgetCharacter;

class WidgetCharacterInternal : public WidgetCombatantBase
{
    Q_OBJECT
public:
    explicit WidgetCharacterInternal(BattleDialogModelCharacter* character, WidgetCharacter *parent);

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
    WidgetCharacter* _widgetParent;
    BattleDialogModelCharacter* _character;
};

#endif // WIDGETCHARACTERINTERNAL_H
