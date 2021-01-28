#include "dmc_settingsdialog.h"
#include "dmc_optionscontainer.h"
#include "ui_dmc_settingsdialog.h"

DMC_SettingsDialog::DMC_SettingsDialog(DMC_OptionsContainer& settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DMC_SettingsDialog),
    _settings(settings)
{
    ui->setupUi(this);

    ui->edtURL->setText(settings.getURLString());
    ui->edtUserName->setText(settings.getUserName());
    ui->edtPassword->setText(settings.getPassword());
    ui->edtSession->setText(settings.getSession());

    connect(ui->edtURL, SIGNAL(textChanged(QString)), &_settings, SLOT(setURLString(QString)));
    connect(ui->edtUserName, SIGNAL(textChanged(QString)), &_settings, SLOT(setUserName(QString)));
    connect(ui->edtPassword, SIGNAL(textChanged(QString)), &_settings, SLOT(setPassword(QString)));
    connect(ui->edtSession, SIGNAL(textChanged(QString)), &_settings, SLOT(setSession(QString)));
}

DMC_SettingsDialog::~DMC_SettingsDialog()
{
    delete ui;
}
