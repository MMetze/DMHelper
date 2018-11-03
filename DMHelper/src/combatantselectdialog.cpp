#include "combatantselectdialog.h"
#include "combatant.h"
#include "ui_combatantselectdialog.h"

CombatantSelectDialog::CombatantSelectDialog(QList<Combatant*> combatants, bool selected, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CombatantSelectDialog)
{
    ui->setupUi(this);
    setCombatantList(combatants, selected);
}

CombatantSelectDialog::~CombatantSelectDialog()
{
    delete ui;
}

void CombatantSelectDialog::setCombatantList(QList<Combatant*> combatants, bool selected)
{
    ui->listCombatants->clear();

    for(int i = 0; i < combatants.size(); ++i)
    {
        QString itemName = combatants.at(i)->getName();
        QListWidgetItem* item = new QListWidgetItem(itemName, ui->listCombatants);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(selected ? Qt::Checked : Qt::Unchecked);
        item->setData(Qt::UserRole, QVariant::fromValue(combatants.at(i)));
    }
}

QList<Combatant*> CombatantSelectDialog::getSelectedCombatants() const
{
    QList<Combatant*> result;

    for(int i = 0; i < ui->listCombatants->count(); ++i)
    {
        if(ui->listCombatants->item(i)->checkState() == Qt::Checked)
        {
            result.append(ui->listCombatants->item(i)->data(Qt::UserRole).value<Combatant*>());
        }
    }

    return result;
}
