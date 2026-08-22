#ifndef INITIATIVEEVENTWIDGET_H
#define INITIATIVEEVENTWIDGET_H

#include "combatantwidget.h"

namespace Ui {
class InitiativeEventWidget;
}

class BattleDialogModelInitiativeEvent;
class QContextMenuEvent;
class QEvent;

class InitiativeEventWidget : public CombatantWidget
{
    Q_OBJECT
public:
    explicit InitiativeEventWidget(BattleDialogModelInitiativeEvent* event, QWidget* parent = nullptr);
    virtual ~InitiativeEventWidget() override;

    // From CombatantWidget
    virtual BattleDialogModelCombatant* getCombatant() override;
    virtual int getInitiative() const override;
    virtual bool isShown() override;
    virtual bool isKnown() override;
    virtual void setShowDone(bool showDone) override;
    virtual void disconnectInternals() override;

public slots:
    virtual void setInitiative(int initiative) override;
    virtual void selectCombatant() override;

protected slots:
    void handleActiveToggled(bool checked);
    void handleInitiativeEdited();
    void handleNameEdited();
    void handleCombatantInitiativeChanged();

protected:
    virtual void contextMenuEvent(QContextMenuEvent* event) override;
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

private:
    Ui::InitiativeEventWidget* ui;
    BattleDialogModelInitiativeEvent* _event;
};

#endif // INITIATIVEEVENTWIDGET_H
