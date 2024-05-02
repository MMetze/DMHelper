#include "widgetinitiativecombatant.h"
#include "ui_widgetinitiativecombatant.h"
#include "battledialogmodelcombatant.h"
#include <QIntValidator>

WidgetInitiativeCombatant::WidgetInitiativeCombatant(int initiative, const QPixmap& pixmap, const QString& name, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetInitiativeCombatant),
    _combatant(nullptr)
{
    ui->setupUi(this);
    ui->edtInitiative->setValidator(new QIntValidator());

    connect(ui->edtInitiative, &QLineEdit::editingFinished, [=]{emit initiativeChanged(getInitiative(), getCombatant());});

    setInitiative(initiative);
    setIconPixmap(pixmap);
    setName(name);
}

WidgetInitiativeCombatant::WidgetInitiativeCombatant(BattleDialogModelCombatant* combatant, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetInitiativeCombatant),
    _combatant(nullptr)
{
    ui->setupUi(this);
    ui->edtInitiative->setValidator(new QIntValidator());

    connect(ui->edtInitiative, &QLineEdit::editingFinished, [=]{emit initiativeChanged(getInitiative(), getCombatant());});

    setCombatant(combatant);
}

WidgetInitiativeCombatant::~WidgetInitiativeCombatant()
{
    delete ui;
}

void WidgetInitiativeCombatant::setCombatant(BattleDialogModelCombatant* combatant)
{
    _combatant = combatant;
    if(!_combatant)
        return;

    setInitiative(_combatant->getInitiative());
    setIconPixmap(_combatant->getIconPixmap(DMHelper::PixmapSize_Battle));
    setName(_combatant->getName());
}

void WidgetInitiativeCombatant::setInitiative(int initiative)
{
    ui->edtInitiative->setText(QString::number(initiative));
}

void WidgetInitiativeCombatant::setIconPixmap(const QPixmap& pixmap)
{
    ui->lblIcon->setPixmap(pixmap.scaled(ui->lblIcon->size(), Qt::KeepAspectRatio));
}

void WidgetInitiativeCombatant::setName(const QString& name)
{
    ui->lblName->setText(name);
}

int WidgetInitiativeCombatant::getInitiative() const
{
    return ui->edtInitiative->text().toInt();
}

BattleDialogModelCombatant* WidgetInitiativeCombatant::getCombatant() const
{
    return _combatant;
}

QString WidgetInitiativeCombatant::getName() const
{
    return ui->lblName->text();
}
