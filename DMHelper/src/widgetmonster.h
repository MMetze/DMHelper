#ifndef WIDGETMONSTER_H
#define WIDGETMONSTER_H

#include "combatantwidget.h"

namespace Ui {
class WidgetMonster;
}

class WidgetMonsterInternal;
class BattleDialogModelCombatant;

class WidgetMonster : public CombatantWidget
{
    Q_OBJECT

public:
    explicit WidgetMonster(QWidget *parent = nullptr);
    virtual ~WidgetMonster() override;

    // From CombatantWidget
    virtual BattleDialogModelCombatant* getCombatant() override;

    void setInternals(WidgetMonsterInternal* internals);
    virtual bool isShown() override;
    virtual bool isKnown() override;

public slots:
    virtual void updateData() override;

    // From CombatantWidget
    virtual void setActive(bool active) override;

protected:
    // From QWidget
    virtual void leaveEvent(QEvent * event) override;
    virtual void mousePressEvent(QMouseEvent * event) override;
    virtual void mouseReleaseEvent(QMouseEvent * event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

private slots:
    void edtInitiativeChanged();
    void edtHPChanged();

private:

    void readInternals();
    virtual void loadImage() override;

    Ui::WidgetMonster *ui;

    WidgetMonsterInternal* _internals;
};

#endif // WIDGETMONSTER_H
