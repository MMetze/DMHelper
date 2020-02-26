#include "combatantdialog.h"
#include "monster.h"
#include "monsterclass.h"
#include "bestiary.h"
#include "dmconstants.h"
#include "ui_combatantdialog.h"
#include <QInputDialog>
#include <QLineEdit>
#include <QDebug>

// TODO: Add option to use average HPs instead of rolling HitDice

CombatantDialog::CombatantDialog(QDialogButtonBox::StandardButtons buttons, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CombatantDialog),
    _combatant(nullptr),
    _count(1)
{
    ui->setupUi(this);

    ui->edtCount->setValidator(new QIntValidator(1,100,this));
    ui->edtHitPointsLocal->setValidator(new QIntValidator(-10,1000,this));
    ui->buttonBox->setStandardButtons(buttons);

    connect(ui->cmbMonsterClass, SIGNAL(currentIndexChanged(QString)), this, SLOT(monsterClassChanged(QString)));
    connect(ui->chkUseAverage, SIGNAL(clicked(bool)), ui->edtHitPointsLocal, SLOT(setDisabled(bool)));
    connect(ui->chkUseAverage, SIGNAL(clicked(bool)), this, SLOT(useHitPointAverageChanged(bool)));
    connect(ui->btnOpenMonster, SIGNAL(clicked(bool)), this, SLOT(openMonsterClicked()));

    ui->cmbMonsterClass->addItems(Bestiary::Instance()->getMonsterList());
}

CombatantDialog::~CombatantDialog()
{
    delete ui;
}

void CombatantDialog::setCombatant(int combatantCount, Combatant* combatant)
{
    if((!combatant)||(combatant->getType() != DMHelper::CombatantType_Monster))
        return;

    Monster* monster = dynamic_cast<Monster*>(combatant);
    if(!monster)
        return;

    _combatant = combatant;
    _count = combatantCount;

    ui->edtCount->setText(QString::number(_count));
    ui->edtNameLocal->setText(_combatant->getName());
    ui->edtHitPointsLocal->setText(_combatant->getHitPoints() == 0 ? "" : QString::number(_combatant->getHitPoints()));

    ui->cmbMonsterClass->setCurrentText(monster->getMonsterClassName());
    ui->lblIcon->setPixmap(monster->getIconPixmap(DMHelper::PixmapSize_Animate));
}

int CombatantDialog::getCount() const
{
    //return _count;
    return ui->edtCount->text().toInt();
}

QString CombatantDialog::getName() const
{
    if(!ui->edtNameLocal->text().isEmpty())
        return ui->edtNameLocal->text();
    else
        return ui->edtName->text();
}

QString CombatantDialog::getLocalHitPoints() const
{
    return ui->edtHitPointsLocal->text();
}

MonsterClass* CombatantDialog::getMonsterClass() const
{
    MonsterClass* monsterClass = Bestiary::Instance()->getMonsterClass(ui->cmbMonsterClass->currentText());
    if(!monsterClass)
        qDebug() << "[Combatant Dialog] Unable to find monster class: " << ui->cmbMonsterClass->currentText();

    return monsterClass;
}

void CombatantDialog::writeCombatant(Combatant* combatant)
{
// TODO: Change to a reference

    if((!combatant)||(combatant->getType() != DMHelper::CombatantType_Monster))
        return;

    Monster* monster = dynamic_cast<Monster*>(combatant);
    if(!monster)
        return;

    MonsterClass* monsterClass = Bestiary::Instance()->getMonsterClass(ui->cmbMonsterClass->currentText());
    if(monsterClass == nullptr)
        return;

    _count = ui->edtCount->text().toInt();

    monster->setMonsterClass(monsterClass);

    combatant->setName(ui->edtNameLocal->text());
    combatant->setHitPoints(ui->edtHitPointsLocal->text().toInt());
}

void CombatantDialog::accept()
{
    if(_combatant)
        writeCombatant(_combatant);
    QDialog::accept();
}

void CombatantDialog::monsterClassChanged(const QString &text)
{
    MonsterClass* monsterClass = Bestiary::Instance()->getMonsterClass(text);
    if(monsterClass == nullptr)
    {
        qDebug() << "[Combatant Dialog] invalid monster class change detected, monster class not found: " << text;
        return;
    }

    QPixmap pmp = monsterClass->getIconPixmap(DMHelper::PixmapSize_Animate);
    if(pmp.isNull())
        pmp = ScaledPixmap::defaultPixmap()->getPixmap(DMHelper::PixmapSize_Animate);

    ui->lblIcon->setPixmap(pmp);

    ui->edtName->setText(text);
    ui->edtHitDice->setText(monsterClass->getHitDice().toString());
    if(ui->chkUseAverage->isChecked())
        useHitPointAverageChanged(true);
}

void CombatantDialog::useHitPointAverageChanged(bool useAverage)
{
    QString localHitPoints;

    if(useAverage)
    {
        MonsterClass* monsterClass = Bestiary::Instance()->getMonsterClass(ui->cmbMonsterClass->currentText());
        if(monsterClass)
            localHitPoints = QString::number(monsterClass->getHitDice().average());
    }

    ui->edtHitPointsLocal->setText(localHitPoints);
}

void CombatantDialog::openMonsterClicked()
{
    emit openMonster(ui->cmbMonsterClass->currentText());
}
