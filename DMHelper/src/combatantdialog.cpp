#include "combatantdialog.h"
#include "monster.h"
#include "monsterclass.h"
#include "bestiary.h"
#include "dmconstants.h"
#include "dice.h"
#include "ui_combatantdialog.h"
#include <QInputDialog>
#include <QLineEdit>
#include <QDebug>

CombatantDialog::CombatantDialog(QDialogButtonBox::StandardButtons buttons, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CombatantDialog),
    //_combatant(nullptr),
    _iconIndex(0)
//    _count(1)
{
    ui->setupUi(this);

    ui->edtCount->setValidator(new QIntValidator(1, 100, this));
    ui->edtHitPointsLocal->setValidator(new QIntValidator(-10, 1000, this));
    ui->buttonBox->setStandardButtons(buttons);

    connect(ui->btnPreviousToken, &QAbstractButton::clicked, this, &CombatantDialog::previousIcon);
    connect(ui->btnNextToken, &QAbstractButton::clicked, this, &CombatantDialog::nextIcon);

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

int CombatantDialog::getCount() const
{
    return ui->edtCount->text().toInt();
}

QString CombatantDialog::getName() const
{
    if(!ui->edtNameLocal->text().isEmpty())
        return ui->edtNameLocal->text();
    else
        return ui->edtName->text();
}

int CombatantDialog::getCombatantHitPoints() const
{
    MonsterClass* monsterClass = getMonsterClass();
    if(!monsterClass)
        return 0;

    if(ui->chkUseAverage->isChecked())
        return monsterClass->getHitDice().average();
    else if(ui->edtHitPointsLocal->text().isEmpty())
        return monsterClass->getHitDice().roll();
    else
        return ui->edtHitPointsLocal->text().toInt();
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

int CombatantDialog::getIconIndex() const
{
    if(ui->chkRandomTokens->isChecked())
    {
        MonsterClass* monsterClass = Bestiary::Instance()->getMonsterClass(ui->cmbMonsterClass->currentText());
        if(monsterClass)
            return Dice::dX(monsterClass->getIconCount()) - 1;
    }

    return _iconIndex;
}

void CombatantDialog::writeCombatant(Combatant* combatant)
{
    if((!combatant) || (combatant->getCombatantType() != DMHelper::CombatantType_Monster))
        return;

    Monster* monster = dynamic_cast<Monster*>(combatant);
    if(!monster)
        return;

    MonsterClass* monsterClass = getMonsterClass();
    if(monsterClass == nullptr)
        return;

    //_count = ui->edtCount->text().toInt();

    monster->setMonsterClass(monsterClass);

    combatant->setName(ui->edtNameLocal->text());
    combatant->setHitPoints(ui->edtHitPointsLocal->text().toInt());
}

void CombatantDialog::accept()
{
    //if(_combatant)
    //    writeCombatant(_combatant);
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
    if(!monsterClass)
    {
        qDebug() << "[Combatant Dialog] invalid monster class change detected, monster class not found: " << text;
        return;
    }

    setIconIndex(0);
    updateIcon();
    ui->btnNextToken->setVisible(monsterClass->getIconCount() > 1);
    ui->btnPreviousToken->setVisible(monsterClass->getIconCount() > 1);
    ui->chkRandomTokens->setEnabled(monsterClass->getIconCount() > 1);

    ui->edtName->setText(text);
    ui->edtHitDice->setText(monsterClass->getHitDice().toString());

    setHitPointAverageChanged();

    if(ui->cmbSize->currentData().toInt() != DMHelper::CombatantSize_Unknown)
        ui->cmbSize->setCurrentIndex(monsterClass->getMonsterSizeCategory() - 1);
}

void CombatantDialog::setIconIndex(int index)
{
    MonsterClass* monsterClass = getMonsterClass();
    if(!monsterClass)
        return;

    if((index < 0) || (index >= monsterClass->getIconCount()))
        return;

    _iconIndex = index;
    updateIcon();

    ui->btnNextToken->setVisible(_iconIndex < monsterClass->getIconCount() - 1);
    ui->btnPreviousToken->setEnabled(_iconIndex > 0);
}

void CombatantDialog::updateIcon()
{
    if(!ui->lblIcon->size().isValid())
        return;

    MonsterClass* monsterClass = getMonsterClass();
    if(!monsterClass)
        return;

    QPixmap pmp = monsterClass->getIconPixmap(DMHelper::PixmapSize_Full, _iconIndex);
    if(pmp.isNull())
    {
        pmp = ScaledPixmap::defaultPixmap()->getPixmap(DMHelper::PixmapSize_Full);
        if(pmp.isNull())
            return;
    }

    ui->lblIcon->setPixmap(pmp.scaled(ui->lblIcon->size(), Qt::KeepAspectRatio));
}

void CombatantDialog::previousIcon()
{
    setIconIndex(_iconIndex - 1);
}

void CombatantDialog::nextIcon()
{
    setIconIndex(_iconIndex + 1);
}

void CombatantDialog::setHitPointAverageChanged()
{
    MonsterClass* monsterClass = getMonsterClass();
    if(!monsterClass)
        return;

    ui->chkUseAverage->setText(QString("Use Average HP (") + QString::number(monsterClass->getHitDice().average()) + QString(")"));
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
