#include "combatantwidgetcharacter.h"
#include "ui_combatantwidgetcharacter.h"
#include "combatantwidgetinternalscharacter.h"
#include "dmconstants.h"
#include "battledialogmodelcharacter.h"
#include "campaign.h"
#include <QIntValidator>
#include <QDebug>

CombatantWidgetCharacter::CombatantWidgetCharacter(bool showDone, QWidget *parent) :
    CombatantWidget(parent),
    ui(new Ui::CombatantWidgetCharacter),
    _internals(nullptr)
{
    ui->setupUi(this);

    connect(ui->edtInit, SIGNAL(editingFinished()), this, SLOT(edtInitiativeChanged()));
    connect(ui->edtMove, SIGNAL(editingFinished()), this, SLOT(edtMoveChanged()));
    connect(ui->edtHP, SIGNAL(editingFinished()), this, SLOT(edtHPChanged()));

    connect(ui->chkKnown, SIGNAL(clicked(bool)), this, SIGNAL(isKnownChanged(bool)));
    connect(ui->chkVisible, SIGNAL(clicked(bool)), this, SIGNAL(isShownChanged(bool)));

    CombatantWidgetCharacter::setShowDone(showDone);

    QValidator* valInit = new QIntValidator(-99, 99, this);
    ui->edtInit->setValidator(valInit);
    QValidator* valHitPoints = new QIntValidator(-10, 9999, this);
    ui->edtHP->setValidator(valHitPoints);
}

CombatantWidgetCharacter::~CombatantWidgetCharacter()
{
    delete _internals;
    delete ui;
}

BattleDialogModelCombatant* CombatantWidgetCharacter::getCombatant()
{
    if(_internals)
        return _internals->getCombatant();
    else
        return nullptr;
}

void CombatantWidgetCharacter::setInternals(CombatantWidgetInternalsCharacter* internals)
{
    if(!internals)
        return;

    if(_internals)
        delete _internals;

    _internals = internals;

    BattleDialogModelCharacter* characterCombatant = dynamic_cast<BattleDialogModelCharacter*>(_internals->getCombatant());
    if(characterCombatant)
    {
        connect(ui->chkKnown, SIGNAL(clicked(bool)), characterCombatant, SLOT(setKnown(bool)));
        connect(ui->chkVisible, SIGNAL(clicked(bool)), characterCombatant, SLOT(setShown(bool)));
        connect(ui->chkDone, SIGNAL(clicked(bool)), characterCombatant, SLOT(setDone(bool)));
        //connect(characterCombatant, SIGNAL(combatantDoneChanged()), this, SLOT(updateData()));
        connect(characterCombatant, &BattleDialogModelCharacter::combatantDoneChanged, this, &CombatantWidgetCharacter::updateData);

        if(characterCombatant->getCombatant())
            connect(characterCombatant->getCombatant(), &Combatant::dirty, this, &CombatantWidgetCharacter::updateData);
        else
            qDebug() << "[Character Widget] a valid combatant could not be found!";
    }

    readInternals();
}

bool CombatantWidgetCharacter::isShown()
{
    return ui->chkVisible->isChecked();
}

bool CombatantWidgetCharacter::isKnown()
{
    return ui->chkKnown->isChecked();
}

void CombatantWidgetCharacter::setShowDone(bool showDone)
{
    ui->lblDone->setVisible(showDone);
    ui->chkDone->setVisible(showDone);
}

void CombatantWidgetCharacter::disconnectInternals()
{
    if(!_internals)
        return;

    BattleDialogModelCharacter* characterCombatant = dynamic_cast<BattleDialogModelCharacter*>(_internals->getCombatant());
    if(!characterCombatant)
        return;

    disconnect(characterCombatant, &BattleDialogModelCharacter::combatantDoneChanged, this, &CombatantWidgetCharacter::updateData);
    if(characterCombatant->getCombatant())
        disconnect(characterCombatant->getCombatant(), &Combatant::dirty, this, &CombatantWidgetCharacter::updateData);
}

void CombatantWidgetCharacter::updateData()
{
    if((!_internals) || (!_internals->getCombatant()))
        return;

    readInternals();
    update();
}

void CombatantWidgetCharacter::updateMove()
{
    if((!_internals) || (!_internals->getCombatant()))
        return;

    ui->edtMove->setText(QString::number(static_cast<int>(_internals->getCombatant()->getMoved())));
}

void CombatantWidgetCharacter::selectCombatant()
{
    if(_internals)
        _internals->executeDoubleClick();
}

void CombatantWidgetCharacter::leaveEvent(QEvent * event)
{
    if(_internals)
        _internals->leaveEvent(event);
    
    CombatantWidget::leaveEvent(event);
}

void CombatantWidgetCharacter::mousePressEvent(QMouseEvent * event)
{
    if(_internals)
        _internals->mousePressEvent(event);
    
    CombatantWidget::mousePressEvent(event);
}

void CombatantWidgetCharacter::mouseReleaseEvent(QMouseEvent * event)
{
    if(_internals)
        _internals->mouseReleaseEvent(event);
    
    CombatantWidget::mouseReleaseEvent(event);
}

void CombatantWidgetCharacter::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(_internals)
        _internals->mouseDoubleClickEvent(event);
    
    CombatantWidget::mouseDoubleClickEvent(event);
}

void CombatantWidgetCharacter::edtInitiativeChanged()
{
    if(_internals)
        _internals->initiativeChanged(ui->edtInit->text().toInt());
}

void CombatantWidgetCharacter::edtMoveChanged()
{
    if(_internals)
        _internals->moveChanged(ui->edtMove->text().toInt());
}

void CombatantWidgetCharacter::edtHPChanged()
{
    if(_internals)
        _internals->handleHitPointsChanged(ui->edtHP->text().toInt());
}

void CombatantWidgetCharacter::readInternals()
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
}

void CombatantWidgetCharacter::loadImage()
{
    if((_internals) && (_internals->getCombatant()))
    {
        ui->lblIcon->resize(DMHelper::CHARACTER_ICON_WIDTH, DMHelper::CHARACTER_ICON_HEIGHT);
        //ui->lblIcon->setPixmap(_internals->getCombatant()->getIconPixmap(DMHelper::PixmapSize_Thumb));
        QPixmap iconPixmap = _internals->getCombatant()->getIconPixmap(DMHelper::PixmapSize_Thumb);
        //if(_internals->getCombatant()->getHitPoints() <= 0)
        if(_internals->getCombatant()->getConditions() & Combatant::Condition_Unconscious)
        {
            QImage originalImage = iconPixmap.toImage();
            QImage grayscaleImage = originalImage.convertToFormat(QImage::Format_Grayscale8);
            iconPixmap = QPixmap::fromImage(grayscaleImage);
        }
        ui->lblIcon->setPixmap(iconPixmap);
    }
}
