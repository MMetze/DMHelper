#include "combatantwidgetmonster.h"
#include "ui_combatantwidgetmonster.h"
#include "combatantwidgetinternalsmonster.h"
#include "dmconstants.h"
#include "battledialogmodelmonsterbase.h"
#include "monsterclassv2.h"
#include "campaign.h"
#include <QIntValidator>
#include <QDebug>

CombatantWidgetMonster::CombatantWidgetMonster(bool showDone, QWidget *parent) :
    CombatantWidget(parent),
    ui(new Ui::CombatantWidgetMonster),
    _internals(nullptr)
{
    ui->setupUi(this);

    connect(ui->edtName, SIGNAL(editingFinished()), this, SLOT(edtNameChanged()));
    connect(ui->edtInit, SIGNAL(editingFinished()), this, SLOT(edtInitiativeChanged()));
    connect(ui->edtMove, SIGNAL(editingFinished()), this, SLOT(edtMoveChanged()));
    connect(ui->edtHP, SIGNAL(editingFinished()), this, SLOT(edtHPChanged()));

    connect(ui->chkKnown, SIGNAL(clicked(bool)), this, SIGNAL(isKnownChanged(bool)));
    connect(ui->chkVisible, SIGNAL(clicked(bool)), this, SIGNAL(isShownChanged(bool)));

    CombatantWidgetMonster::setShowDone(showDone);

    QValidator* valInit = new QIntValidator(-99, 99, this);
    ui->edtInit->setValidator(valInit);
    QValidator* valHitPoints = new QIntValidator(-10, 9999, this);
    ui->edtHP->setValidator(valHitPoints);
}

CombatantWidgetMonster::~CombatantWidgetMonster()
{
    delete _internals;
    delete ui;
}

BattleDialogModelCombatant* CombatantWidgetMonster::getCombatant()
{
    if(_internals)
        return _internals->getCombatant();
    else
        return nullptr;
}

void CombatantWidgetMonster::setInternals(CombatantWidgetInternalsMonster* internals)
{
    if((!internals) || !internals->getCombatant())
        return;

    if(_internals)
        delete _internals;

    _internals = internals;

    BattleDialogModelMonsterBase* monsterCombatant = dynamic_cast<BattleDialogModelMonsterBase*>(_internals->getCombatant());
    if(monsterCombatant)
    {
        connect(ui->chkKnown, SIGNAL(clicked(bool)), monsterCombatant, SLOT(setKnown(bool)));
        connect(ui->chkVisible, SIGNAL(clicked(bool)), monsterCombatant, SLOT(setShown(bool)));
        connect(ui->chkDone, SIGNAL(clicked(bool)), monsterCombatant, SLOT(setDone(bool)));
        //connect(monsterCombatant, SIGNAL(combatantDoneChanged()), this, SLOT(updateData()));
        connect(monsterCombatant, &BattleDialogModelMonsterBase::combatantDoneChanged, this, &CombatantWidgetMonster::updateData);

        if(monsterCombatant->getCombatant())
            connect(monsterCombatant->getCombatant(), &Combatant::dirty, this, &CombatantWidgetMonster::updateData);
        else if (monsterCombatant->getMonsterClass())
            connect(monsterCombatant->getMonsterClass(), &MonsterClassv2::dirty, this, &CombatantWidgetMonster::updateData);
        else
            qDebug() << "[Monster Widget] neither valid combatant nor monster class found!";

        if((monsterCombatant->getMonsterClass()) && (monsterCombatant->getMonsterClass()->getIntValue("legendary") > 0))
        {
            _internals->resetLegendary();
            connect(ui->btnLegendary, SIGNAL(clicked(bool)), _internals, SLOT(decrementLegendary()));
            ui->btnLegendary->show();
        }
        else
        {
            ui->btnLegendary->hide();
        }
    }

    readInternals();
}

bool CombatantWidgetMonster::isShown()
{
    return ui->chkVisible->isChecked();
}

bool CombatantWidgetMonster::isKnown()
{
    return ui->chkKnown->isChecked();
}

void CombatantWidgetMonster::setShowDone(bool showDone)
{
    ui->lblDone->setVisible(showDone);
    ui->chkDone->setVisible(showDone);
}

void CombatantWidgetMonster::disconnectInternals()
{
    if(!_internals)
        return;

    BattleDialogModelMonsterBase* monsterCombatant = dynamic_cast<BattleDialogModelMonsterBase*>(_internals->getCombatant());
    if(!monsterCombatant)
        return;

    disconnect(monsterCombatant, &BattleDialogModelMonsterBase::combatantDoneChanged, this, &CombatantWidgetMonster::updateData);
    if(monsterCombatant->getCombatant())
        disconnect(monsterCombatant->getCombatant(), &Combatant::dirty, this, &CombatantWidgetMonster::updateData);
    else if (monsterCombatant->getMonsterClass())
        disconnect(monsterCombatant->getMonsterClass(), &MonsterClassv2::dirty, this, &CombatantWidgetMonster::updateData);
}

void CombatantWidgetMonster::clearImage()
{
    ui->lblIcon->clear();
}

void CombatantWidgetMonster::updateData()
{
    if((!_internals) || (!_internals->getCombatant()))
        return;

    readInternals();
    update();
}

void CombatantWidgetMonster::updateMove()
{
    if((!_internals) || (!_internals->getCombatant()))
        return;

    ui->edtMove->setText(QString::number(static_cast<int>(_internals->getCombatant()->getMoved())));
}

void CombatantWidgetMonster::setActive(bool active)
{
    if(_internals)
        _internals->resetLegendary();
    
    CombatantWidget::setActive(active);
}

void CombatantWidgetMonster::selectCombatant()
{
    if(_internals)
        _internals->executeDoubleClick();
}

void CombatantWidgetMonster::leaveEvent(QEvent * event)
{
    if(_internals)
        _internals->leaveEvent(event);
    
    CombatantWidget::leaveEvent(event);
}

void CombatantWidgetMonster::mousePressEvent(QMouseEvent * event)
{
    if(_internals)
        _internals->mousePressEvent(event);
    
    CombatantWidget::mousePressEvent(event);
}

void CombatantWidgetMonster::mouseReleaseEvent(QMouseEvent * event)
{
    if(_internals)
        _internals->mouseReleaseEvent(event);
    
    CombatantWidget::mouseReleaseEvent(event);
}

void CombatantWidgetMonster::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(_internals)
        _internals->mouseDoubleClickEvent(event);
    
    CombatantWidget::mouseDoubleClickEvent(event);
}

void CombatantWidgetMonster::edtNameChanged()
{
    if(_internals)
        _internals->setMonsterName(ui->edtName->text());
}

void CombatantWidgetMonster::edtInitiativeChanged()
{
    if(_internals)
        _internals->initiativeChanged(ui->edtInit->text().toInt());
}

void CombatantWidgetMonster::edtMoveChanged()
{
    if(_internals)
        _internals->moveChanged(ui->edtMove->text().toInt());
}

void CombatantWidgetMonster::edtHPChanged()
{
    if(_internals)
        _internals->handleHitPointsChanged(ui->edtHP->text().toInt());
}

void CombatantWidgetMonster::readInternals()
{
    if((!_internals) || (!_internals->getCombatant()))
        return;

    loadImage();
    updateMove();

    ui->edtName->setText(_internals->getCombatant()->getName());
    ui->edtName->home(false);
    ui->edtAC->setText(QString::number(_internals->getCombatant()->getArmorClass()));
    ui->edtHP->setText(QString::number(_internals->getCombatant()->getHitPoints()));
    ui->edtMove->setText(QString::number(static_cast<int>(_internals->getCombatant()->getMoved())));
    ui->edtInit->setText(QString::number(_internals->getCombatant()->getInitiative()));
    ui->chkKnown->setChecked(_internals->getCombatant()->getKnown());
    ui->chkVisible->setChecked(_internals->getCombatant()->getShown());
    ui->chkDone->setChecked(_internals->getCombatant()->getDone());

    BattleDialogModelMonsterBase* monsterCombatant = dynamic_cast<BattleDialogModelMonsterBase*>(_internals->getCombatant());
    if((monsterCombatant) && (monsterCombatant->getMonsterClass()) && (monsterCombatant->getMonsterClass()->getIntValue("legendary") > 0))
        ui->btnLegendary->setText(QString("L: ") + QString::number(monsterCombatant->getLegendaryCount()));
}

void CombatantWidgetMonster::loadImage()
{
    if(!ui->lblIcon->pixmap(Qt::ReturnByValue).isNull())
        return;

    if((_internals) && (_internals->getCombatant()))
    {
        ui->lblIcon->resize(DMHelper::CHARACTER_ICON_WIDTH, DMHelper::CHARACTER_ICON_HEIGHT);
        QPixmap iconPixmap = _internals->getCombatant()->getIconPixmap(DMHelper::PixmapSize_Thumb);
        if(_internals->getCombatant()->hasCondition(Combatant::Condition_Unconscious))
        {
            QImage originalImage = iconPixmap.toImage();
            QImage grayscaleImage = originalImage.convertToFormat(QImage::Format_Grayscale8);
            iconPixmap = QPixmap::fromImage(grayscaleImage);
        }
        ui->lblIcon->setPixmap(iconPixmap);
        emit imageChanged(_internals->getCombatant());
    }
}
