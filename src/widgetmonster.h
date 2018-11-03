#ifndef WIDGETMONSTER_H
#define WIDGETMONSTER_H

//#include <QFrame>
#include "combatantwidget.h"

namespace Ui {
class WidgetMonster;
}

class WidgetMonsterInternal;
class BattleDialogModelCombatant;

class WidgetMonster : public CombatantWidget //QFrame
{
    Q_OBJECT

public:
    explicit WidgetMonster(QWidget *parent = nullptr);
    ~WidgetMonster();

    // From CombatantWidget
    virtual BattleDialogModelCombatant* getCombatant();

    void setInternals(WidgetMonsterInternal* internals);
    virtual bool isShown();
    virtual bool isKnown();

public slots:
    virtual void updateData();

    // From CombatantWidget
    virtual void setActive(bool active);

protected:
    // From QWidget
    virtual void leaveEvent(QEvent * event);
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);

private slots:
    void edtInitiativeChanged();
    void edtHPChanged();

private:

    void readInternals();
    virtual void loadImage();

    Ui::WidgetMonster *ui;

    WidgetMonsterInternal* _internals;
};

#endif // WIDGETMONSTER_H
