#include "initiativelistcombatantwidget.h"
#include "ui_initiativelistcombatantwidget.h"
#include "battledialogmodelcombatant.h"
#include <QIntValidator>

InitiativeListCombatantWidget::InitiativeListCombatantWidget(int initiative, const QPixmap& pixmap, const QString& name, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InitiativeListCombatantWidget),
    _combatant(nullptr)
{
    ui->setupUi(this);
    ui->edtInitiative->setValidator(new QIntValidator());

    connect(ui->edtInitiative, &QLineEdit::editingFinished, [=]{emit initiativeChanged(getInitiative(), getCombatant());});

    setInitiative(initiative);
    setIconPixmap(pixmap);
    setName(name);
}

InitiativeListCombatantWidget::InitiativeListCombatantWidget(BattleDialogModelCombatant* combatant, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InitiativeListCombatantWidget),
    _combatant(nullptr)
{
    ui->setupUi(this);
    ui->edtInitiative->setValidator(new QIntValidator());

    connect(ui->edtInitiative, &QLineEdit::editingFinished, [=]{emit initiativeChanged(getInitiative(), getCombatant());});

    setCombatant(combatant);
}

InitiativeListCombatantWidget::~InitiativeListCombatantWidget()
{
    delete ui;
}

void InitiativeListCombatantWidget::setCombatant(BattleDialogModelCombatant* combatant)
{
    _combatant = combatant;
    if(!_combatant)
        return;

    setInitiative(_combatant->getInitiative());
    setIconPixmap(_combatant->getIconPixmap(DMHelper::PixmapSize_Battle));
    setName(_combatant->getName());
}

void InitiativeListCombatantWidget::setInitiative(int initiative)
{
    ui->edtInitiative->setText(QString::number(initiative));
}

void InitiativeListCombatantWidget::setIconPixmap(const QPixmap& pixmap)
{
    ui->lblIcon->setPixmap(pixmap.scaled(ui->lblIcon->size(), Qt::KeepAspectRatio));
}

void InitiativeListCombatantWidget::setName(const QString& name)
{
    ui->lblName->setText(name);
}

int InitiativeListCombatantWidget::getInitiative() const
{
    return ui->edtInitiative->text().toInt();
}

BattleDialogModelCombatant* InitiativeListCombatantWidget::getCombatant() const
{
    return _combatant;
}

QString InitiativeListCombatantWidget::getName() const
{
    return ui->lblName->text();
}

void InitiativeListCombatantWidget::setInitiativeFocus()
{
    ui->edtInitiative->setFocus();
    ui->edtInitiative->selectAll();
}
