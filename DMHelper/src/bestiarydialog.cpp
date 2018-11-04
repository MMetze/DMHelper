#include "bestiarydialog.h"
#include "bestiary.h"
#include "monsterclass.h"
#include "combatant.h"
#include "dmconstants.h"
#include <QIntValidator>
#include <QDoubleValidator>
#include <QInputDialog>
#include <QMouseEvent>
#include <QFileDialog>
#include <QDebug>
#include "ui_bestiarydialog.h"

BestiaryDialog::BestiaryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BestiaryDialog),
    _monster(nullptr),
    _edit(false),
    _mouseDown(false)
{
    ui->setupUi(this);

    connect(ui->btnLeft,SIGNAL(clicked()),this,SLOT(previousMonster()));
    connect(ui->btnRight,SIGNAL(clicked()),this,SLOT(nextMonster()));
    connect(ui->btnNewMonster,SIGNAL(clicked()),this,SLOT(createNewMonster()));
    connect(ui->btnDeleteMonster,SIGNAL(clicked()),this,SLOT(deleteCurrentMonster()));
    connect(ui->cmbSearch,SIGNAL(activated(QString)),this,SLOT(setMonster(QString)));
    connect(ui->btnPublish,SIGNAL(clicked()),this,SLOT(handlePublishButton()));

    connect(ui->edtStrength,SIGNAL(editingFinished()),this,SLOT(abilityChanged()));
    connect(ui->edtDexterity,SIGNAL(editingFinished()),this,SLOT(abilityChanged()));
    connect(ui->edtConstitution,SIGNAL(editingFinished()),this,SLOT(abilityChanged()));
    connect(ui->edtIntelligence,SIGNAL(editingFinished()),this,SLOT(abilityChanged()));
    connect(ui->edtWisdom,SIGNAL(editingFinished()),this,SLOT(abilityChanged()));
    connect(ui->edtCharisma,SIGNAL(editingFinished()),this,SLOT(abilityChanged()));

    connect(ui->edtName,SIGNAL(editingFinished()),this,SLOT(monsterRenamed()));

    ui->edtArmorClass->setValidator(new QIntValidator(0,100));
    ui->edtAverageHitPoints->setValidator(new QIntValidator(0,10000));
    ui->edtChallenge->setValidator(new QDoubleValidator(0.0, 100.0, 2));
    ui->edtXP->setValidator(new QIntValidator(0,1000000));
}

BestiaryDialog::~BestiaryDialog()
{
    delete ui;
}

MonsterClass* BestiaryDialog::getMonster() const
{
    return _monster;
}

void BestiaryDialog::setMonster(MonsterClass* monster, bool edit)
{
    if(!monster)
        return;

    qDebug() << "[Bestiary Dialog] Set Monster to " << monster->getName();

    if(_monster && _edit)
        storeMonsterData();

    _monster = monster;
    _edit = edit;

    if(ui->cmbSearch->currentText() != _monster->getName())
        ui->cmbSearch->setCurrentText(_monster->getName());

    loadMonsterImage();

    ui->chkPrivate->setChecked(_monster->getPrivate());
    ui->chkLegendary->setChecked(_monster->getLegendary());
    ui->edtName->setText(_monster->getName());
    ui->edtMonsterType->setText(_monster->getMonsterType());
    ui->edtAlignment->setText(_monster->getAlignment());
    ui->edtArmorClass->setText(QString::number(_monster->getArmorClass()));
    ui->edtArmorClassDescription->setText(_monster->getArmorClassDescription());
    ui->edtHitDice->setText(_monster->getHitDice().toString());
    ui->edtAverageHitPoints->setText(QString::number(_monster->getAverageHitPoints()));
    ui->edtSpeed->setText(_monster->getSpeed());
    ui->edtStrength->setText(QString::number(_monster->getStrength()));
    ui->edtDexterity->setText(QString::number(_monster->getDexterity()));
    ui->edtConstitution->setText(QString::number(_monster->getConstitution()));
    ui->edtIntelligence->setText(QString::number(_monster->getIntelligence()));
    ui->edtWisdom->setText(QString::number(_monster->getWisdom()));
    ui->edtCharisma->setText(QString::number(_monster->getCharisma()));
    ui->edtSkills->setText(_monster->getSkills());
    ui->edtResistances->setText(_monster->getResistances());
    ui->edtSenses->setText(_monster->getSenses());
    ui->edtLanguages->setText(_monster->getLanguages());
    if(_monster->getLanguages().isEmpty())
    {
        ui->edtLanguages->setText("---");
    }
    ui->edtChallenge->setText(QString::number(_monster->getChallenge()));
    ui->edtXP->setText(QString::number(_monster->getXP()));
    ui->edtTraits->setHtml(_monster->getTraits());
    ui->edtActions->setHtml(_monster->getActions());

    updateAbilityMods();

    if(!_edit)
    {
        ui->btnLeft->hide();
        ui->btnRight->hide();

        if(_monster->getSkills().isEmpty())
        {
            ui->edtSkills->hide();
            ui->lblSkills->hide();
        }

        if(_monster->getResistances().isEmpty())
        {
            ui->edtResistances->hide();
            ui->lblResistances->hide();
        }

        if(_monster->getSenses().isEmpty())
        {
            ui->edtSenses->hide();
            ui->lblSenses->hide();
        }

        if(_monster->getTraits().isEmpty())
            ui->edtTraits->hide();
    }

    ui->btnLeft->setEnabled(_monster != Bestiary::Instance()->getFirstMonsterClass());
    ui->btnRight->setEnabled(_monster != Bestiary::Instance()->getLastMonsterClass());

    ui->edtName->setReadOnly(!_edit);
    ui->edtMonsterType->setReadOnly(!_edit);
    ui->edtAlignment->setReadOnly(!_edit);
    ui->edtArmorClass->setReadOnly(!_edit);
    ui->edtArmorClassDescription->setReadOnly(!_edit);
    ui->edtHitDice->setReadOnly(!_edit);
    ui->edtAverageHitPoints->setReadOnly(!_edit);
    ui->edtSpeed->setReadOnly(!_edit);
    ui->edtStrength->setReadOnly(!_edit);
    ui->edtDexterity->setReadOnly(!_edit);
    ui->edtConstitution->setReadOnly(!_edit);
    ui->edtIntelligence->setReadOnly(!_edit);
    ui->edtWisdom->setReadOnly(!_edit);
    ui->edtCharisma->setReadOnly(!_edit);
    ui->edtSkills->setReadOnly(!_edit);
    ui->edtResistances->setReadOnly(!_edit);
    ui->edtSenses->setReadOnly(!_edit);
    ui->edtLanguages->setReadOnly(!_edit);
    ui->edtChallenge->setReadOnly(!_edit);
    ui->edtXP->setReadOnly(!_edit);
    ui->edtTraits->setReadOnly(!_edit);
    ui->edtActions->setReadOnly(!_edit);
}

void BestiaryDialog::setMonster(const QString& monsterName, bool edit)
{
    MonsterClass* monsterClass = Bestiary::Instance()->getMonsterClass(monsterName);
    if(monsterClass)
        setMonster(monsterClass, edit);
}

void BestiaryDialog::createNewMonster()
{
    bool ok;
    QString monsterName = QInputDialog::getText(this, QString("Enter New Monster Name"),QString("New Monster"),QLineEdit::Normal,QString(),&ok);
    if((!ok)||(monsterName.isEmpty()))
        return;

    MonsterClass* monsterClass;
    if(Bestiary::Instance()->exists(monsterName))
    {
        monsterClass = Bestiary::Instance()->getMonsterClass(monsterName);
    }
    else
    {
        monsterClass = new MonsterClass(monsterName);
        Bestiary::Instance()->insertMonsterClass(monsterClass);
    }

    qDebug() << "[Bestiary Dialog] New Monster created " << monsterClass;

    setMonster(monsterClass);
    show();
    activateWindow();
}

void BestiaryDialog::deleteCurrentMonster()
{
    if(!_monster)
        return;

    qDebug() << "[Bestiary Dialog] Removing Monster created " << _monster->getName();
    Bestiary::Instance()->removeMonsterClass(_monster);
    if(Bestiary::Instance()->count() > 0)
    {
        setMonster(Bestiary::Instance()->getFirstMonsterClass());
    }
    else
    {
        _monster = nullptr;
        hide();
    }
}

void BestiaryDialog::dataChanged()
{
    _monster = nullptr;
    ui->cmbSearch->clear();
    ui->cmbSearch->addItems(Bestiary::Instance()->getMonsterList());
}

void BestiaryDialog::abilityChanged()
{
    if(!_monster)
        return;

    _monster->setStrength(ui->edtStrength->text().toInt());
    _monster->setDexterity(ui->edtDexterity->text().toInt());
    _monster->setConstitution(ui->edtConstitution->text().toInt());
    _monster->setIntelligence(ui->edtIntelligence->text().toInt());
    _monster->setWisdom(ui->edtWisdom->text().toInt());
    _monster->setCharisma(ui->edtCharisma->text().toInt());

    updateAbilityMods();
}

void BestiaryDialog::updateAbilityMods()
{
    if(!_monster)
        return;

    ui->lblStrengthBonus->setText(QString("(") + Combatant::getAbilityModStr(_monster->getStrength()) + QString(")"));
    ui->lblDexterityBonus->setText(QString("(") + Combatant::getAbilityModStr(_monster->getDexterity()) + QString(")"));
    ui->lblConstitutionBonus->setText(QString("(") + Combatant::getAbilityModStr(_monster->getConstitution()) + QString(")"));
    ui->lblIntelligenceBonus->setText(QString("(") + Combatant::getAbilityModStr(_monster->getIntelligence()) + QString(")"));
    ui->lblWisdomBonus->setText(QString("(") + Combatant::getAbilityModStr(_monster->getWisdom()) + QString(")"));
    ui->lblCharismaBonus->setText(QString("(") + Combatant::getAbilityModStr(_monster->getCharisma()) + QString(")"));
}

void BestiaryDialog::monsterRenamed()
{
    if( (!_monster) || (ui->edtName->text() == _monster->getName()) )
        return;

    Bestiary::Instance()->renameMonster(_monster, ui->edtName->text());
}

void BestiaryDialog::handlePublishButton()
{
    if(!_monster)
        return;

    QImage iconImg;
    QString iconFile = _monster->getIcon();
    QString iconPath = Bestiary::Instance()->getDirectory().filePath(iconFile);
    if((!iconPath.isEmpty()) && (iconImg.load(iconPath) == true))
    {
        emit publishMonsterImage(iconImg);
    }
}

void BestiaryDialog::closeEvent(QCloseEvent * event)
{
    Q_UNUSED(event);
    qDebug() << "[Bestiary Dialog] Bestiary Dialog closing... storing data";
    storeMonsterData();
    QDialog::closeEvent(event);
}

void BestiaryDialog::mousePressEvent(QMouseEvent * event)
{
    Q_UNUSED(event);
    if(_edit && _monster)
    {
        ui->lblIcon->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        _mouseDown = true;
    }
}

void BestiaryDialog::mouseReleaseEvent(QMouseEvent * event)
{
    if(_edit && _mouseDown && _monster)
    {
        ui->lblIcon->setFrameStyle(QFrame::Panel | QFrame::Raised);
        _mouseDown = false;
        if(ui->lblIcon->frameGeometry().contains(event->pos()))
        {
            QString filename = QFileDialog::getOpenFileName(this,QString("Select New Image..."));
            if(!filename.isEmpty())
            {
                _monster->setIcon(filename);
                loadMonsterImage();
            }
        }
    }
}

void BestiaryDialog::showEvent(QShowEvent * event)
{
    Q_UNUSED(event);
    qDebug() << "[Bestiary Dialog] Bestiary Dialog shown";
    connect(Bestiary::Instance(),SIGNAL(changed()),this,SLOT(dataChanged()));
    QDialog::showEvent(event);
}

void BestiaryDialog::hideEvent(QHideEvent * event)
{
    Q_UNUSED(event);
    qDebug() << "[Bestiary Dialog] Bestiary Dialog hidden... storing data";
    storeMonsterData();
    QDialog::hideEvent(event);
}

void BestiaryDialog::focusOutEvent(QFocusEvent * event)
{
    Q_UNUSED(event);
    qDebug() << "[Bestiary Dialog] Bestiary Dialog lost focus... storing data";
    storeMonsterData();
    QDialog::focusOutEvent(event);
}

void BestiaryDialog::previousMonster()
{
    MonsterClass* previousClass = Bestiary::Instance()->getPreviousMonsterClass(_monster);
    if(previousClass)
        setMonster(previousClass);
}

void BestiaryDialog::nextMonster()
{
    MonsterClass* nextClass = Bestiary::Instance()->getNextMonsterClass(_monster);
    if(nextClass)
        setMonster(nextClass);
}

void BestiaryDialog::loadMonsterImage()
{
    ui->lblIcon->setPixmap(_monster->getIconPixmap(DMHelper::PixmapSize_Showcase));
}

void BestiaryDialog::storeMonsterData()
{
    if((!_monster)||(!_edit))
        return;

    qDebug() << "[Bestiary Dialog] Storing monster data for " << _monster->getName();

    _monster->beginBatchChanges();

    _monster->setPrivate(ui->chkPrivate->isChecked());
    _monster->setLegendary(ui->chkLegendary->isChecked());
    _monster->setName(ui->edtName->text());
    _monster->setMonsterType(ui->edtMonsterType->text());
    _monster->setAlignment(ui->edtAlignment->text());
    _monster->setArmorClass(ui->edtArmorClass->text().toInt());
    _monster->setArmorClassDescription(ui->edtArmorClassDescription->text());
    _monster->setHitDice(Dice(ui->edtHitDice->text()));
    _monster->setAverageHitPoints(ui->edtAverageHitPoints->text().toInt());
    _monster->setSpeed(ui->edtSpeed->text());
    _monster->setSkills(ui->edtSkills->text());
    _monster->setResistances(ui->edtResistances->text());
    _monster->setSenses(ui->edtSenses->text());
    _monster->setLanguages(ui->edtLanguages->text());
    _monster->setChallenge(ui->edtChallenge->text().toFloat());
    _monster->setXP(ui->edtXP->text().toInt());
    _monster->setTraits(ui->edtTraits->toHtml());
    _monster->setActions(ui->edtActions->toHtml());

    _monster->endBatchChanges();
}
