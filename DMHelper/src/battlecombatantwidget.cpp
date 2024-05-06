#include "battlecombatantwidget.h"
#include "ui_battlecombatantwidget.h"
#include "battledialogmodel.h"
#include <QIntValidator>
#include <QMouseEvent>

BattleCombatantWidget::BattleCombatantWidget(BattleDialogModelCombatant* combatant, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BattleCombatantWidget),
    _combatant(combatant),
    _mouseDown(false),
    _mouseDownPos(),
    _result(0)
{
    ui->setupUi(this);
    QValidator* valHitPoints = new QIntValidator(-10, 9999, this);
    ui->edtHP->setValidator(valHitPoints);
    setCombatantValues();

    connect(ui->edtHP, SIGNAL(textEdited(QString)), this, SLOT(handleHitPointsChanged(QString)));
    connect(ui->btnAdvantage, SIGNAL(clicked(bool)), this, SLOT(handleAdvantageClicked(bool)));
    connect(ui->btnDisadvantage, SIGNAL(clicked(bool)), this, SLOT(handleDisadvantageClicked(bool)));
    connect(ui->chkActive, SIGNAL(toggled(bool)), this, SLOT(handleCombatantActive(bool)));
}

BattleCombatantWidget::~BattleCombatantWidget()
{
    delete ui;
}

bool BattleCombatantWidget::hasAdvantage() const
{
    return ui->btnAdvantage->isChecked();
}

bool BattleCombatantWidget::hasDisadvantage() const
{
    return ui->btnDisadvantage->isChecked();
}

void BattleCombatantWidget::setResult(const QString &text)
{
    ui->edtResult->setText(text);
}

void BattleCombatantWidget::setResult(int result)
{
    _result = result;
}

int BattleCombatantWidget::getResult() const
{
    return _result;
}

void BattleCombatantWidget::applyDamage(int damage)
{
    if(!_combatant)
        return;

    if(_combatant->getHitPoints() > damage)
        _combatant->setHitPoints(_combatant->getHitPoints() - damage);
    else
        _combatant->setHitPoints(0);

    emit combatantChanged(_combatant);
    setCombatantValues();
    emit hitPointsChanged(_combatant, damage);
}

void BattleCombatantWidget::applyConditions(int conditions)
{
    if((!_combatant) || (conditions == 0))
        return;

    _combatant->applyConditions(conditions);
    emit combatantChanged(_combatant);
}

bool BattleCombatantWidget::isActive()
{
    return ui->chkActive->isChecked();
}

void BattleCombatantWidget::mousePressEvent(QMouseEvent *event)
{
    if(!event)
        return;

    _mouseDown = true;
    _mouseDownPos = event->pos();
}

void BattleCombatantWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(!event)
        return;

    if((_mouseDown) && (_mouseDownPos == event->pos()))
    {
        emit selectCombatant(_combatant);
    }

    _mouseDown = false;
}

void BattleCombatantWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if(!_combatant)
        return;

    QPixmap combatantImage = _combatant->getIconPixmap(DMHelper::PixmapSize_Full);
    ui->lblIcon->setPixmap(combatantImage.scaled(ui->lblIcon->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void BattleCombatantWidget::handleHitPointsChanged(const QString& text)
{
    int newHP = text.toInt();

    if((!_combatant) || (newHP == _combatant->getHitPoints()))
        return;

    _combatant->setHitPoints(newHP);
    emit combatantChanged(_combatant);
}

void BattleCombatantWidget::handleRerollRequest()
{
    emit rerollNeeded(this);
}

void BattleCombatantWidget::handleAdvantageClicked(bool checked)
{
    if(checked && ui->btnDisadvantage->isChecked())
        ui->btnDisadvantage->setChecked(false);

    handleRerollRequest();
}

void BattleCombatantWidget::handleDisadvantageClicked(bool checked)
{
    if(checked && ui->btnAdvantage->isChecked())
        ui->btnAdvantage->setChecked(false);

    handleRerollRequest();
}

void BattleCombatantWidget::handleCombatantActive(bool active)
{
    ui->lblIcon->setEnabled(active);
    ui->edtName->setEnabled(active);
    ui->edtHP->setEnabled(active);
    ui->label->setEnabled(active);
    ui->edtResult->setEnabled(active);
    ui->btnAdvantage->setEnabled(active);
    ui->btnDisadvantage->setEnabled(active);
}

void BattleCombatantWidget::setCombatantValues()
{
    if(!_combatant)
        return;

    ui->edtName->setText(_combatant->getName());
    ui->edtHP->setText(QString::number(_combatant->getHitPoints()));
}
