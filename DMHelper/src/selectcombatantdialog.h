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
    explicit SelectCombatantDialog(BattleDialogModel& model, BattleDialogModelObject* thisItem, bool includeParents = true, bool includeChildren = false, QWidget *parent = nullptr);
    ~SelectCombatantDialog();

    void addObject(BattleDialogModelObject* itemObject);
    BattleDialogModelObject* getSelectedObject() const;
    bool isCentered() const;

protected:
    virtual void showEvent(QShowEvent *event) override;

    void readModel();
    void addCombatant(BattleDialogModelCombatant* combatant);
    void addEffect(BattleDialogModelEffect* effect, BattleDialogModelEffect* childEffect, QGraphicsItem* effectItem);
    bool isChild(BattleDialogModelObject* item);
    void removeParents();

private:
    Ui::SelectCombatantDialog *ui;
    BattleDialogModel& _model;
    QList<BattleDialogModelObject*> _thisItemList;
    bool _includeParents;
    bool _includeChildren;
};

#endif // SELECTCOMBATANTDIALOG_H
