#ifndef SELECTCOMBATANTDIALOG_H
#define SELECTCOMBATANTDIALOG_H

#include <QDialog>
#include "battledialogmodel.h"

namespace Ui {
class SelectCombatantDialog;
}

class SelectCombatantDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectCombatantDialog(BattleDialogModel& model, BattleDialogModelObject* thisItem, QWidget *parent = nullptr);
    ~SelectCombatantDialog();

    BattleDialogModelObject* getSelectedObject() const;
    bool isCentered() const;

protected:
    void readModel();
    void addCombatant(BattleDialogModelCombatant* combatant);
    void addEffect(BattleDialogModelEffect* effect, BattleDialogModelEffect* childEffect, QGraphicsItem* effectItem);

private:
    Ui::SelectCombatantDialog *ui;
    BattleDialogModel& _model;
    BattleDialogModelObject* _thisItem;
};

#endif // SELECTCOMBATANTDIALOG_H
