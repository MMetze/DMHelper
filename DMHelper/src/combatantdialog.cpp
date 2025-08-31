#include "combatantdialog.h"
#include "monster.h"
#include "monsterclassv2.h"
#include "bestiary.h"
#include "dmconstants.h"
#include "layer.h"
#include "layertokens.h"
#include "layerscene.h"
#include "ui_combatantdialog.h"
#include <QInputDialog>
#include <QFileDialog>
#include <QImageReader>
#include <QLineEdit>
#include <QCompleter>
#include <QDebug>

CombatantDialog::CombatantDialog(LayerScene& layerScene, QDialogButtonBox::StandardButtons buttons, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CombatantDialog),
    _iconIndex(0),
    _iconFile()
{
    ui->setupUi(this);

    ui->edtCount->setValidator(new QIntValidator(1, 100, this));
    ui->edtHitPointsLocal->setValidator(new QIntValidator(-10, 1000, this));
    ui->buttonBox->setStandardButtons(buttons);

    connect(ui->btnPreviousToken, &QAbstractButton::clicked, this, &CombatantDialog::previousIcon);
    connect(ui->btnCustomToken, &QAbstractButton::clicked, this, &CombatantDialog::selectCustomToken);
    connect(ui->btnNextToken, &QAbstractButton::clicked, this, &CombatantDialog::nextIcon);

    connect(ui->cmbMonsterClass,  &QComboBox::textActivated, this, &CombatantDialog::monsterClassChanged);
    connect(ui->chkUseAverage, SIGNAL(clicked(bool)), ui->edtHitPointsLocal, SLOT(setDisabled(bool)));
    connect(ui->btnOpenMonster, SIGNAL(clicked(bool)), this, SLOT(openMonsterClicked()));

    connect(ui->edtHitDice, &QLineEdit::editingFinished, this, &CombatantDialog::setHitPointAverageChanged);

    connect(ui->chkRandomInitiative, &QAbstractButton::clicked, ui->edtInitiative, &QWidget::setDisabled);
    ui->edtInitiative->setValidator(new QIntValidator(-100, 1000, this));

    ui->edtSize->setValidator(new QDoubleValidator(0.25, 1000.0, 2, this));
    connect(ui->cmbSize, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CombatantDialog::sizeSelected);
    fillSizeCombo();

    ui->cmbMonsterClass->addItems(Bestiary::Instance()->getMonsterList());
    // Create a completer and attach it to the search combo box
    QCompleter *completer = new QCompleter(ui->cmbMonsterClass->model(), this);
    completer->setFilterMode(Qt::MatchContains);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    ui->cmbMonsterClass->setCompleter(completer);

    Layer* currentLayer = layerScene.getPriority(DMHelper::LayerType_Tokens);
    int currentLayerIndex = -1;
    QList<Layer*> layers = layerScene.getLayers(DMHelper::LayerType_Tokens);
    foreach(Layer* layer, layers)
    {
        LayerTokens* layerToken = dynamic_cast<LayerTokens*>(layer);
        if(layerToken)
        {
            ui->cmbLayer->addItem(layerToken->getName(), QVariant::fromValue(reinterpret_cast<quint64>(layerToken)));
            if(layer == currentLayer)
                currentLayerIndex = ui->cmbLayer->count() - 1;
        }
    }

    if((currentLayerIndex >= 0) && (currentLayerIndex < ui->cmbLayer->count()))
        ui->cmbLayer->setCurrentIndex(currentLayerIndex);    
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

LayerTokens* CombatantDialog::getLayer() const
{
    return reinterpret_cast<LayerTokens*>(ui->cmbLayer->currentData().value<quint64>());
}

int CombatantDialog::getCombatantHitPoints() const
{
    MonsterClassv2* monsterClass = getMonsterClass();
    if(!monsterClass)
        return 0;

    if(ui->chkUseAverage->isChecked())
        return getMonsterHitDice(*monsterClass).average();
    else if(!ui->edtHitPointsLocal->text().isEmpty())
        return ui->edtHitPointsLocal->text().toInt();
    else
        return getMonsterHitDice(*monsterClass).roll();
}

bool CombatantDialog::isRandomInitiative() const
{
    return ui->chkRandomInitiative->isChecked();
}

bool CombatantDialog::isSortInitiative() const
{
    return ui->chkSortInitiative->isChecked();
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

MonsterClassv2* CombatantDialog::getMonsterClass() const
{
    MonsterClassv2* monsterClass = Bestiary::Instance()->getMonsterClass(ui->cmbMonsterClass->currentText());
    if(!monsterClass)
        qDebug() << "[Combatant Dialog] Unable to find monster class: " << ui->cmbMonsterClass->currentText();

    return monsterClass;
}

int CombatantDialog::getIconIndex() const
{
    if(ui->chkRandomTokens->isChecked())
    {
        MonsterClassv2* monsterClass = Bestiary::Instance()->getMonsterClass(ui->cmbMonsterClass->currentText());
        if(monsterClass)
            return Dice::dX(monsterClass->getIconCount()) - 1;
    }

    return _iconIndex;
}

QString CombatantDialog::getIconFile() const
{
    return _iconFile;
}

void CombatantDialog::writeCombatant(Combatant* combatant)
{
    if((!combatant) || (combatant->getCombatantType() != DMHelper::CombatantType_Monster))
        return;

    Monster* monster = dynamic_cast<Monster*>(combatant);
    if(!monster)
        return;

    MonsterClassv2* monsterClass = getMonsterClass();
    if(monsterClass == nullptr)
        return;

    monster->setMonsterClass(monsterClass);

    combatant->setName(ui->edtNameLocal->text());
    combatant->setHitPoints(ui->edtHitPointsLocal->text().toInt());
}

void CombatantDialog::accept()
{
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
    MonsterClassv2* monsterClass = Bestiary::Instance()->getMonsterClass(text);
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
    ui->edtHitDice->setText(monsterClass->getDiceValue("hit_dice").toString());

    setHitPointAverageChanged();

    if(ui->cmbSize->currentData().toInt() != DMHelper::CombatantSize_Unknown)
        ui->cmbSize->setCurrentIndex(monsterClass->MonsterClassv2::convertSizeToCategory(monsterClass->getStringValue("size")) - 1);
}

void CombatantDialog::setIconIndex(int index)
{
    MonsterClassv2* monsterClass = getMonsterClass();
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

    QPixmap pmp;

    if(!_iconFile.isEmpty())
    {
        if(!pmp.load(_iconFile))
            return;
    }
    else
    {
        MonsterClassv2* monsterClass = getMonsterClass();
        if(!monsterClass)
            return;

        pmp = monsterClass->getIconPixmap(DMHelper::PixmapSize_Full, _iconIndex);
        if(pmp.isNull())
        {
            pmp = ScaledPixmap::defaultPixmap()->getPixmap(DMHelper::PixmapSize_Full);
            if(pmp.isNull())
                return;
        }
    }

    ui->lblIcon->setPixmap(pmp.scaled(ui->lblIcon->size(), Qt::KeepAspectRatio));
}

void CombatantDialog::previousIcon()
{
    _iconFile.clear();
    setIconIndex(_iconIndex - 1);
}

void CombatantDialog::selectCustomToken()
{
    _iconFile.clear();

    QString filename = QFileDialog::getOpenFileName(nullptr, QString("Select monster token..."));
    if(filename.isEmpty())
        return;

    if(!QImageReader(filename).canRead())
    {
        qDebug() << "[CombatantDialog] selectCustomToken: " << filename << " is not a valid image file.";
        return;
    }

    _iconIndex = -1;
    _iconFile = filename;
    updateIcon();

    ui->btnNextToken->setVisible(true);
    ui->btnPreviousToken->setEnabled(false);
}

void CombatantDialog::nextIcon()
{
    _iconFile.clear();
    setIconIndex(_iconIndex + 1);
}

void CombatantDialog::setHitPointAverageChanged()
{
    MonsterClassv2* monsterClass = getMonsterClass();
    if(!monsterClass)
        return;

    ui->chkUseAverage->setText(QString("Use Average HP (") + QString::number(getMonsterHitDice(*monsterClass).average()) + QString(")"));
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
        ui->edtSize->setText(QString::number(MonsterClassv2::convertSizeCategoryToScaleFactor(sizeCategory)));
}

void CombatantDialog::fillSizeCombo()
{
    ui->cmbSize->clear();

    ui->cmbSize->addItem(MonsterClassv2::convertCategoryToSize(DMHelper::CombatantSize_Tiny), DMHelper::CombatantSize_Tiny);
    ui->cmbSize->addItem(MonsterClassv2::convertCategoryToSize(DMHelper::CombatantSize_Small), DMHelper::CombatantSize_Small);
    ui->cmbSize->addItem(MonsterClassv2::convertCategoryToSize(DMHelper::CombatantSize_Medium), DMHelper::CombatantSize_Medium);
    ui->cmbSize->addItem(MonsterClassv2::convertCategoryToSize(DMHelper::CombatantSize_Large), DMHelper::CombatantSize_Large);
    ui->cmbSize->addItem(MonsterClassv2::convertCategoryToSize(DMHelper::CombatantSize_Huge), DMHelper::CombatantSize_Huge);
    ui->cmbSize->addItem(MonsterClassv2::convertCategoryToSize(DMHelper::CombatantSize_Gargantuan), DMHelper::CombatantSize_Gargantuan);
    ui->cmbSize->addItem(MonsterClassv2::convertCategoryToSize(DMHelper::CombatantSize_Colossal), DMHelper::CombatantSize_Colossal);
    ui->cmbSize->insertSeparator(999); // Insert at the end of the list
    ui->cmbSize->addItem(QString("Custom..."), DMHelper::CombatantSize_Unknown);

    ui->cmbSize->setCurrentIndex(2); // Default to Medium
}

Dice CombatantDialog::getMonsterHitDice(const MonsterClassv2& monsterClass) const
{
    Dice dialogHitDice(ui->edtHitDice->text());

    if(dialogHitDice.isValid())
        return dialogHitDice;
    else
        return monsterClass.getDiceValue("hit_dice");
}
