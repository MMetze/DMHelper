#ifndef COMBATANTWIDGETMONSTER_H
#define COMBATANTWIDGETMONSTER_H

#include "combatantwidget.h"

namespace Ui {
class CombatantWidgetMonster;
}

class CombatantWidgetInternalsMonster;
class BattleDialogModelCombatant;

class CombatantWidgetMonster : public CombatantWidget
{
    Q_OBJECT

public:
    explicit CombatantWidgetMonster(bool showDone, QWidget *parent = nullptr);
    virtual ~CombatantWidgetMonster() override;

    // From CombatantWidget
    virtual BattleDialogModelCombatant* getCombatant() override;

    void setInternals(CombatantWidgetInternalsMonster* internals);
    virtual bool isShown() override;
    virtual bool isKnown() override;

public slots:
    void clearImage();
    virtual void updateData() override;
    virtual void updateMove() override;

    // From CombatantWidget
    virtual void setActive(bool active) override;
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

    Ui::CombatantWidgetMonster *ui;

    CombatantWidgetInternalsMonster* _internals;
};

#endif // COMBATANTWIDGETMONSTER_H
