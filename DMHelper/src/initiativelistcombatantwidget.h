#ifndef INITIATIVELISTCOMBATANTWIDGET_H
#define INITIATIVELISTCOMBATANTWIDGET_H

#include <QWidget>

namespace Ui {
class InitiativeListCombatantWidget;
}

class BattleDialogModelCombatant;

class InitiativeListCombatantWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InitiativeListCombatantWidget(int initiative = 0, const QPixmap& pixmap = QPixmap(), const QString& name = QString(), QWidget *parent = nullptr);
    explicit InitiativeListCombatantWidget(BattleDialogModelCombatant* combatant, QWidget *parent = nullptr);
    ~InitiativeListCombatantWidget();

    void setCombatant(BattleDialogModelCombatant* combatant);
    void setInitiative(int initiative);
    void setIconPixmap(const QPixmap& pixmap);
    void setName(const QString& name);

    int getInitiative() const;
    BattleDialogModelCombatant* getCombatant() const;
    QString getName() const;

signals:
    void initiativeChanged(int initiative, BattleDialogModelCombatant* combatant);

private:
    Ui::InitiativeListCombatantWidget *ui;
    BattleDialogModelCombatant* _combatant;
};

#endif // INITIATIVELISTCOMBATANTWIDGET_H
