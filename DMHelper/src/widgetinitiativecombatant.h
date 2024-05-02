#ifndef WIDGETINITIATIVECOMBATANT_H
#define WIDGETINITIATIVECOMBATANT_H

#include <QWidget>

namespace Ui {
class WidgetInitiativeCombatant;
}

class BattleDialogModelCombatant;

class WidgetInitiativeCombatant : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetInitiativeCombatant(int initiative = 0, const QPixmap& pixmap = QPixmap(), const QString& name = QString(), QWidget *parent = nullptr);
    explicit WidgetInitiativeCombatant(BattleDialogModelCombatant* combatant, QWidget *parent = nullptr);
    ~WidgetInitiativeCombatant();

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
    Ui::WidgetInitiativeCombatant *ui;
    BattleDialogModelCombatant* _combatant;
};

#endif // WIDGETINITIATIVECOMBATANT_H
