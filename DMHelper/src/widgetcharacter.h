#ifndef WIDGETCHARACTER_H
#define WIDGETCHARACTER_H

#include "combatantwidget.h"

namespace Ui {
class WidgetCharacter;
}

class WidgetCharacterInternal;
class BattleDialogModelCombatant;

class WidgetCharacter : public CombatantWidget
{
    Q_OBJECT

public:
    explicit WidgetCharacter(QWidget *parent = nullptr);
    virtual ~WidgetCharacter() override;

    // From CombatantWidget
    virtual BattleDialogModelCombatant* getCombatant() override;

    void setInternals(WidgetCharacterInternal* internals);
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

    Ui::WidgetCharacter *ui;

    WidgetCharacterInternal* _internals;
};

#endif // WIDGETCHARACTER_H
