#include "newcampaigndialog.h"
#include "ui_newcampaigndialog.h"
#include "rulefactory.h"
#include <QFileDialog>

NewCampaignDialog::NewCampaignDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewCampaignDialog)
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

    connect(ui->cmbRulesets, &QComboBox::currentIndexChanged, this, &NewCampaignDialog::handleRulesetSelected);
    connect(ui->btnBrowseCharacterData, &QPushButton::clicked, this, &NewCampaignDialog::handleCharacterDataBrowse);
    connect(ui->btnBrowseCharacterUI, &QPushButton::clicked, this, &NewCampaignDialog::handleCharacterUIBrowse);

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

QString NewCampaignDialog::getCharacterDataFile() const
{
    return ui->edtCharacterData->text();
}

QString NewCampaignDialog::getCharacterUIFile() const
{
    return ui->edtCharacterUI->text();
}

bool NewCampaignDialog::isCombatantDone() const
{
    return ui->chkCombatantDone->isChecked();
}

void NewCampaignDialog::handleRulesetSelected()
{
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
    ui->edtCharacterData->setText(ruleset._characterData);
    ui->edtCharacterUI->setText(ruleset._characterUI);
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
