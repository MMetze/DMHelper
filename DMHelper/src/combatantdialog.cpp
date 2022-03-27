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

    ui->edtCount->setValidator(new QIntValidator(1, 100, this));
    ui->edtHitPointsLocal->setValidator(new QIntValidator(-10, 1000, this));
    ui->buttonBox->setStandardButtons(buttons);

    connect(ui->cmbMonsterClass, SIGNAL(currentIndexChanged(QString)), this, SLOT(monsterClassChanged(QString)));
    connect(ui->chkUseAverage, SIGNAL(clicked(bool)), ui->edtHitPointsLocal, SLOT(setDisabled(bool)));
    connect(ui->btnOpenMonster, SIGNAL(clicked(bool)), this, SLOT(openMonsterClicked()));

    connect(ui->chkRandomInitiative, &QAbstractButton::clicked, ui->edtInitiative, &QWidget::setDisabled);
    ui->edtInitiative->setValidator(new QIntValidator(-100, 1000, this));

    ui->edtSize->setValidator(new QDoubleValidator(0.25, 1000.0, 2, this));
    connect(ui->cmbSize, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CombatantDialog::sizeSelected);
    fillSizeCombo();

    ui->cmbMonsterClass->addItems(Bestiary::Instance()->getMonsterList());
}

CombatantDialog::~CombatantDialog()
{
    delete ui;
}

void CombatantDialog::setCombatant(int combatantCount, Combatant* combatant)
{
    if((!combatant)||(combatant->getCombatantType() != DMHelper::CombatantType_Monster))
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

bool CombatantDialog::isRandomInitiative() const
{
    return ui->chkRandomInitiative->isChecked();
}

QString CombatantDialog::getInitiative() const
{
    return ui->edtInitiative->text();
}

bool CombatantDialog::isKnown() const
{
    return ui->chkKnown->isChecked();
}

bool CombatantDialog::isShown() const
{
    return ui->chkVisible->isChecked();
}

bool CombatantDialog::isCustomSize() const
{
    return ui->cmbSize->currentData().toInt() == DMHelper::CombatantSize_Unknown;
}

QString CombatantDialog::getSizeFactor() const
{
    return ui->edtSize->text();
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

    if((!combatant)||(combatant->getCombatantType() != DMHelper::CombatantType_Monster))
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

void CombatantDialog::showEvent(QShowEvent *event)
{
    updateIcon();
    QDialog::showEvent(event);
}

void CombatantDialog::resizeEvent(QResizeEvent *event)
{
    updateIcon();
    QDialog::resizeEvent(event);
}

void CombatantDialog::monsterClassChanged(const QString &text)
{
    MonsterClass* monsterClass = Bestiary::Instance()->getMonsterClass(text);
    if(monsterClass == nullptr)
    {
        qDebug() << "[Combatant Dialog] invalid monster class change detected, monster class not found: " << text;
        return;
    }

    updateIcon();

    ui->edtName->setText(text);
    ui->edtHitDice->setText(monsterClass->getHitDice().toString());

    setHitPointAverageChanged();

    if(ui->cmbSize->currentData().toInt() != DMHelper::CombatantSize_Unknown)
        ui->cmbSize->setCurrentIndex(monsterClass->getMonsterSizeCategory() - 1);
}

void CombatantDialog::updateIcon()
{
    if(!ui->lblIcon->size().isValid())
        return;

    MonsterClass* monsterClass = getMonsterClass();
    if(!monsterClass)
        return;

    QPixmap pmp = monsterClass->getIconPixmap(DMHelper::PixmapSize_Full);
    if(pmp.isNull())
    {
        pmp = ScaledPixmap::defaultPixmap()->getPixmap(DMHelper::PixmapSize_Full);
        if(pmp.isNull())
            return;
    }

    ui->lblIcon->setPixmap(pmp.scaled(ui->lblIcon->size(), Qt::KeepAspectRatio));
}

void CombatantDialog::setHitPointAverageChanged()
{
    QString localHitPoints;

    MonsterClass* monsterClass = Bestiary::Instance()->getMonsterClass(ui->cmbMonsterClass->currentText());
    if(monsterClass)
        localHitPoints = QString::number(monsterClass->getHitDice().average());

    ui->edtHitPointsLocal->setText(localHitPoints);
}

void CombatantDialog::openMonsterClicked()
{
    emit openMonster(ui->cmbMonsterClass->currentText());
}

void CombatantDialog::sizeSelected(int index)
{
    Q_UNUSED(index);
    int sizeCategory = ui->cmbSize->currentData().toInt();

    ui->edtSize->setEnabled(sizeCategory == DMHelper::CombatantSize_Unknown);
    if(sizeCategory != DMHelper::CombatantSize_Unknown)
        ui->edtSize->setText(QString::number(MonsterClass::convertSizeCategoryToScaleFactor(sizeCategory)));
}

void CombatantDialog::fillSizeCombo()
{
    ui->cmbSize->clear();

    ui->cmbSize->addItem(MonsterClass::convertCategoryToSize(DMHelper::CombatantSize_Tiny), DMHelper::CombatantSize_Tiny);
    ui->cmbSize->addItem(MonsterClass::convertCategoryToSize(DMHelper::CombatantSize_Small), DMHelper::CombatantSize_Small);
    ui->cmbSize->addItem(MonsterClass::convertCategoryToSize(DMHelper::CombatantSize_Medium), DMHelper::CombatantSize_Medium);
    ui->cmbSize->addItem(MonsterClass::convertCategoryToSize(DMHelper::CombatantSize_Large), DMHelper::CombatantSize_Large);
    ui->cmbSize->addItem(MonsterClass::convertCategoryToSize(DMHelper::CombatantSize_Huge), DMHelper::CombatantSize_Huge);
    ui->cmbSize->addItem(MonsterClass::convertCategoryToSize(DMHelper::CombatantSize_Gargantuan), DMHelper::CombatantSize_Gargantuan);
    ui->cmbSize->addItem(MonsterClass::convertCategoryToSize(DMHelper::CombatantSize_Colossal), DMHelper::CombatantSize_Colossal);
    ui->cmbSize->insertSeparator(999); // Insert at the end of the list
    ui->cmbSize->addItem(QString("Custom..."), DMHelper::CombatantSize_Unknown);

    ui->cmbSize->setCurrentIndex(2); // Default to Medium
}
