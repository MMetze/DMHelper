#include "combatantwidgetbase.h"
#include "ui_combatantwidgetbase.h"

CombatantWidgetBase::CombatantWidgetBase(QWidget* parent) :
    QFrame(parent),
    ui(new Ui::CombatantWidgetBase)
{
    ui->setupUi(this);
}

CombatantWidgetBase::~CombatantWidgetBase()
{
    delete ui;
}

QFrame* CombatantWidgetBase::contentFrame() const
{
    return ui->frameCombatant;
}

QLabel* CombatantWidgetBase::iconLabel() const
{
    return ui->lblIcon;
}

QCheckBox* CombatantWidgetBase::knownCheckbox() const
{
    return ui->chkKnown;
}

QCheckBox* CombatantWidgetBase::visibleCheckbox() const
{
    return ui->chkVisible;
}

QCheckBox* CombatantWidgetBase::doneCheckbox() const
{
    return ui->chkDone;
}

QLabel* CombatantWidgetBase::doneIconLabel() const
{
    return ui->lblDone;
}
