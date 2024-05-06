#include "initiativelistdialog.h"
#include "ui_initiativelistdialog.h"
#include "initiativelistcombatantwidget.h"

InitiativeListDialog::InitiativeListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InitiativeListDialog),
    _combatantLayout(nullptr)
{
    ui->setupUi(this);

    _combatantLayout = new QVBoxLayout;
    _combatantLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    _combatantLayout->setSpacing(1);
    ui->scrollAreaWidgetContents->setLayout(_combatantLayout);
}

InitiativeListDialog::~InitiativeListDialog()
{
    QLayoutItem *child;
    while ((child = _combatantLayout->takeAt(0)) != nullptr) {
        delete child;
    }

    delete ui;
}

void InitiativeListDialog::addCombatantWidget(InitiativeListCombatantWidget* widget)
{
    if(!widget)
        return;

    _combatantLayout->addWidget(widget);
}

int InitiativeListDialog::getCombatantCount() const
{
    return _combatantLayout->count();
}

InitiativeListCombatantWidget* InitiativeListDialog::getCombatantWidget(int index)
{
    if((index < 0) || (index >= _combatantLayout->count()))
        return nullptr;

    QLayoutItem* layoutItem = _combatantLayout->itemAt(index);
    if(!layoutItem)
        return nullptr;
    
    return qobject_cast<InitiativeListCombatantWidget*>(layoutItem->widget());
}
