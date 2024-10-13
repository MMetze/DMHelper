#ifndef INITIATIVELISTDIALOG_H
#define INITIATIVELISTDIALOG_H

#include <QDialog>

namespace Ui {
class InitiativeListDialog;
}

class QVBoxLayout;
class InitiativeListCombatantWidget;

class InitiativeListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InitiativeListDialog(QWidget *parent = nullptr);
    ~InitiativeListDialog();
    
    void addCombatantWidget(InitiativeListCombatantWidget* widget);

    int getCombatantCount() const;
    InitiativeListCombatantWidget* getCombatantWidget(int index);

private:
    Ui::InitiativeListDialog *ui;
    QVBoxLayout* _combatantLayout;
};

#endif // INITIATIVELISTDIALOG_H
