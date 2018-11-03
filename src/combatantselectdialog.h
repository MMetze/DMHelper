#ifndef COMBATANTSELECTDIALOG_H
#define COMBATANTSELECTDIALOG_H

#include <QDialog>
#include <QList>

class Combatant;

namespace Ui {
class CombatantSelectDialog;
}

class CombatantSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CombatantSelectDialog(QList<Combatant*> combatants, bool selected = true, QWidget *parent = 0);
    ~CombatantSelectDialog();

    void setCombatantList(QList<Combatant*> combatants, bool selected = true);
    QList<Combatant*> getSelectedCombatants() const;

private:
    Ui::CombatantSelectDialog *ui;
};

#endif // COMBATANTSELECTDIALOG_H
