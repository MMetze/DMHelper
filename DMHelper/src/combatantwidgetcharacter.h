#ifndef COMBATANTWIDGETCHARACTER_H
#define COMBATANTWIDGETCHARACTER_H

#include "combatantwidget.h"

namespace Ui {
class CombatantWidgetCharacter;
}

class CombatantWidgetInternalsCharacter;
class BattleDialogModelCombatant;

class CombatantWidgetCharacter : public CombatantWidget
{
    Q_OBJECT

public:
    explicit CombatantWidgetCharacter(bool showDone, QWidget *parent = nullptr);
    virtual ~CombatantWidgetCharacter() override;

    // From CombatantWidget
    virtual BattleDialogModelCombatant* getCombatant() override;
    
    void setInternals(CombatantWidgetInternalsCharacter* internals);
    virtual bool isShown() override;
    virtual bool isKnown() override;

public slots:
    virtual void updateData() override;
    virtual void updateMove() override;

    // From CombatantWidget
    virtual void selectCombatant() override;

protected:
    // From QWidget
    virtual void leaveEvent(QEvent * event) override;
    virtual void mousePressEvent(QMouseEvent * event) override;
    virtual void mouseReleaseEvent(QMouseEvent * event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

private slots:
    void edtInitiativeChanged();
    void edtMoveChanged();
    void edtHPChanged();

private:

    void readInternals();
    virtual void loadImage() override;

    Ui::CombatantWidgetCharacter *ui;
    
    CombatantWidgetInternalsCharacter* _internals;
};

#endif // COMBATANTWIDGETCHARACTER_H
