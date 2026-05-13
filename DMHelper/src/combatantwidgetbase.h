#ifndef COMBATANTWIDGETBASE_H
#define COMBATANTWIDGETBASE_H

#include <QFrame>

class QLabel;
class QCheckBox;
class QFrame;

namespace Ui {
class CombatantWidgetBase;
}

// Shared shell for in-battle combatant rows. Provides the common chrome
// (icon, visible/known/done checkboxes) plus a host frame into which the
// per-combatant template content is loaded. Pure container - no behaviour.
class CombatantWidgetBase : public QFrame
{
    Q_OBJECT
public:
    explicit CombatantWidgetBase(QWidget* parent = nullptr);
    ~CombatantWidgetBase() override;

    QFrame* contentFrame() const;
    QLabel* iconLabel() const;
    QCheckBox* knownCheckbox() const;
    QCheckBox* visibleCheckbox() const;
    QCheckBox* doneCheckbox() const;
    QLabel* doneIconLabel() const;

private:
    Ui::CombatantWidgetBase* ui;
};

#endif // COMBATANTWIDGETBASE_H
