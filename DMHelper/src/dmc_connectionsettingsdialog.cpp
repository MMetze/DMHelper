#include "dmc_connectionsettingsdialog.h"
#include "ui_dmc_connectionsettingsdialog.h"
#include "dmc_optionscontainer.h"
#include "dmhlogon.h"

DMC_ConnectionSettingsDialog::DMC_ConnectionSettingsDialog(DMC_OptionsContainer& options, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DMC_ConnectionSettingsDialog),
    _options(options)
{
    ui->setupUi(this);
    ui->frameConnection->setSettings(&options);
}

DMC_ConnectionSettingsDialog::~DMC_ConnectionSettingsDialog()
{
    delete ui;
}

void DMC_ConnectionSettingsDialog::logMessage(const QString& message)
{
    ui->frameConnection->logMessage(message);
}
