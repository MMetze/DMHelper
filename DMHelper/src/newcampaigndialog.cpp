#include "newcampaigndialog.h"
#include "ui_newcampaigndialog.h"
#include "rulefactory.h"
#include <QFileDialog>

NewCampaignDialog::NewCampaignDialog(const QString& rulesetName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewCampaignDialog)
{
    ui->setupUi(this);

    QList<QString> rulesets = RuleFactory::Instance()->getRulesetNames();
    for(const QString& ruleset : rulesets)
    {
        ui->cmbRulesets->addItem(ruleset);
    }

    QStringList ruleInitiativeNames = RuleFactory::getRuleInitiativeNames();
    for(int i = 0; i  < ruleInitiativeNames.count() / 2; ++i)
    {
        ui->cmbInitiative->addItem(ruleInitiativeNames.at(2 * i + 1), ruleInitiativeNames.at(2 * i));
    }

    if(rulesets.contains(rulesetName))
        ui->cmbRulesets->setCurrentText(rulesetName);

    connect(ui->cmbRulesets, &QComboBox::currentIndexChanged, this, &NewCampaignDialog::handleRulesetSelected);
    connect(ui->btnBrowseCharacterData, &QPushButton::clicked, this, &NewCampaignDialog::handleCharacterDataBrowse);
    connect(ui->btnBrowseCharacterUI, &QPushButton::clicked, this, &NewCampaignDialog::handleCharacterUIBrowse);
    connect(ui->btnBrowseBestiaryFile, &QPushButton::clicked, this, &NewCampaignDialog::handleBestiaryFileBrowse);
    connect(ui->btnBrowseMonsterData, &QPushButton::clicked, this, &NewCampaignDialog::handleMonsterDataBrowse);
    connect(ui->btnBrowseMonsterUI, &QPushButton::clicked, this, &NewCampaignDialog::handleMonsterUIBrowse);

    handleRulesetSelected();
}

NewCampaignDialog::~NewCampaignDialog()
{
    delete ui;
}

QString NewCampaignDialog::getCampaignName() const
{
    return ui->edtCampaignName->text();
}

QString NewCampaignDialog::getInitiativeType() const
{
    return ui->cmbInitiative->currentData().toString();
}

QString NewCampaignDialog::getInitiativeDescription() const
{
    return ui->cmbInitiative->currentText();
}

QString NewCampaignDialog::getMovementString() const
{
    return ui->edtMovement->text();
}

QString NewCampaignDialog::getCharacterDataFile() const
{
    return ui->edtCharacterData->text();
}

QString NewCampaignDialog::getCharacterUIFile() const
{
    return ui->edtCharacterUI->text();
}

QString NewCampaignDialog::getBestiaryFile() const
{
    return ui->edtBestiaryFile->text();
}

QString NewCampaignDialog::getMonsterDataFile() const
{
    return ui->edtMonsterData->text();
}

QString NewCampaignDialog::getMonsterUIFile() const
{
    return ui->edtMonsterUI->text();
}

bool NewCampaignDialog::isCombatantDone() const
{
    return ui->chkCombatantDone->isChecked();
}

bool NewCampaignDialog::isShowFear() const
{
    return ui->chkShowFear->isChecked();
}

QString NewCampaignDialog::getRuleset() const
{
    return ui->cmbRulesets->currentText();
}

void NewCampaignDialog::setRuleset(const QString& rulesetName)
{
    if((rulesetName.isEmpty()) || (rulesetName == ui->cmbRulesets->currentText()))
        return;

    ui->cmbRulesets->setCurrentText(rulesetName);
}

void NewCampaignDialog::handleRulesetSelected()
{
    if(!RuleFactory::Instance())
        return;

    QString rulesetName = ui->cmbRulesets->currentText();
    RuleFactory::RulesetTemplate ruleset = RuleFactory::Instance()->getRulesetTemplate(rulesetName);

    if(ruleset._name != rulesetName)
    {
        qDebug() << "[New Campaign Dialog] Ruleset not found: " << rulesetName;
        return;
    }

    int initiativeIndex = ui->cmbInitiative->findData(ruleset._initiative);
    if(initiativeIndex != -1)
        ui->cmbInitiative->setCurrentIndex(initiativeIndex);

    ui->edtMovement->setText(ruleset._movement.isEmpty() ? QString("distance") : ruleset._movement);

    ui->edtCharacterData->setText(QDir::cleanPath(ruleset._rulesetDir.absoluteFilePath(ruleset._characterData)));
    ui->edtCharacterUI->setText(QDir::cleanPath(ruleset._rulesetDir.absoluteFilePath(ruleset._characterUI)));
    ui->edtMonsterData->setText(QDir::cleanPath(ruleset._rulesetDir.absoluteFilePath(ruleset._monsterData)));
    ui->edtMonsterUI->setText(QDir::cleanPath(ruleset._rulesetDir.absoluteFilePath(ruleset._monsterUI)));

    if((RuleFactory::Instance()) && (rulesetName == RuleFactory::DEFAULT_RULESET_NAME) && (!RuleFactory::Instance()->getDefaultBestiary().isEmpty()))
        ui->edtBestiaryFile->setText(RuleFactory::Instance()->getDefaultBestiary());
    else
        ui->edtBestiaryFile->setText(QDir::cleanPath(ruleset._rulesetDir.absoluteFilePath(ruleset._bestiary)));

    ui->chkCombatantDone->setChecked(ruleset._combatantDone);

    bool daggerheart = ruleset._name.contains(QString("daggerheart"), Qt::CaseInsensitive);
    ui->chkShowFear->setChecked(daggerheart);
    ui->chkShowFear->setVisible(daggerheart);
}

void NewCampaignDialog::handleCharacterDataBrowse()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Character Data File"), QString(), tr("XML Files (*.xml)"));
    if(!fileName.isEmpty())
        ui->edtCharacterData->setText(fileName);
}

void NewCampaignDialog::handleCharacterUIBrowse()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Character UI File"), QString(), tr("UI Files (*.ui)"));
    if(!fileName.isEmpty())
        ui->edtCharacterUI->setText(fileName);
}

void NewCampaignDialog::handleBestiaryFileBrowse()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Bestiary File"), QString(), tr("XML Files (*.xml)"));
    if(!fileName.isEmpty())
        ui->edtBestiaryFile->setText(fileName);
}

void NewCampaignDialog::handleMonsterDataBrowse()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Monster Data File"), QString(), tr("XML Files (*.xml)"));
    if(!fileName.isEmpty())
        ui->edtMonsterData->setText(fileName);
}

void NewCampaignDialog::handleMonsterUIBrowse()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Monster UI File"), QString(), tr("UI Files (*.ui)"));
    if(!fileName.isEmpty())
        ui->edtMonsterUI->setText(fileName);
}
