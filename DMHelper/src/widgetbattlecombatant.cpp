#include "widgetbattlecombatant.h"
#include "ui_widgetbattlecombatant.h"
#include "battledialogmodel.h"
#include <QIntValidator>
#include <QDebug>

WidgetBattleCombatant::WidgetBattleCombatant(BattleDialogModelCombatant* combatant, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetBattleCombatant),
    _combatant(combatant),
    _mouseDown(false),
    _mouseDownPos(),
    _result(0)
{
    ui->setupUi(this);
    QValidator* valHitPoints = new QIntValidator(-10, 9999, this);
    ui->edtHP->setValidator(valHitPoints);
    setCombatantValues();

    connect(ui->edtHP,SIGNAL(textEdited(QString)),this,SLOT(handleHitPointsChanged(QString)));
    connect(ui->btnAdvantage,SIGNAL(clicked(bool)),this,SLOT(handleAdvantageClicked(bool)));
    connect(ui->btnDisadvantage,SIGNAL(clicked(bool)),this,SLOT(handleDisadvantageClicked(bool)));
}

WidgetBattleCombatant::~WidgetBattleCombatant()
{
    delete ui;
}

bool WidgetBattleCombatant::hasAdvantage() const
{
    return ui->btnAdvantage->isChecked();
}

bool WidgetBattleCombatant::hasDisadvantage() const
{
    return ui->btnDisadvantage->isChecked();
}

void WidgetBattleCombatant::setResult(const QString &text)
{
    ui->edtResult->setText(text);
}

void WidgetBattleCombatant::setResult(int result)
{
    _result = result;
}

int WidgetBattleCombatant::getResult() const
{
    return _result;
}

void WidgetBattleCombatant::applyDamage(int damage)
{
    if(!_combatant)
        return;

    if(_combatant->getHitPoints() > damage)
        _combatant->setHitPoints(_combatant->getHitPoints() - damage);
    else
        _combatant->setHitPoints(0);

    emit combatantChanged(_combatant);
    setCombatantValues();
}

void WidgetBattleCombatant::mousePressEvent(QMouseEvent *event)
{
    if(!event)
        return;

    _mouseDown = true;
    _mouseDownPos = event->pos();
}

void WidgetBattleCombatant::mouseReleaseEvent(QMouseEvent *event)
{
    if(!event)
        return;

    if((_mouseDown) && (_mouseDownPos == event->pos()))
    {
        emit selectCombatant(_combatant);
    }

    _mouseDown = false;
}

void WidgetBattleCombatant::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if(!_combatant)
        return;

    QPixmap combatantImage = _combatant->getIconPixmap(DMHelper::PixmapSize_Full);
    ui->lblIcon->setPixmap(combatantImage.scaled(ui->lblIcon->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void WidgetBattleCombatant::handleHitPointsChanged(const QString& text)
{
    int newHP = text.toInt();

    if((!_combatant) || (newHP == _combatant->getHitPoints()))
        return;

    _combatant->setHitPoints(newHP);
    emit combatantChanged(_combatant);
}

void WidgetBattleCombatant::handleRerollRequest()
{
    emit rerollNeeded(this);
}

void WidgetBattleCombatant::handleAdvantageClicked(bool checked)
{
    if(checked && ui->btnDisadvantage->isChecked())
        ui->btnDisadvantage->setChecked(false);

    handleRerollRequest();
}

void WidgetBattleCombatant::handleDisadvantageClicked(bool checked)
{
    if(checked && ui->btnAdvantage->isChecked())
        ui->btnAdvantage->setChecked(false);

    handleRerollRequest();
}

void WidgetBattleCombatant::setCombatantValues()
{
    if(!_combatant)
        return;

    ui->edtName->setText(_combatant->getName());
    ui->edtHP->setText(QString::number(_combatant->getHitPoints()));
}
