#include "settingsdialog.h"
#include "optionscontainer.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(OptionsContainer& settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog),
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

SettingsDialog::~SettingsDialog()
{
    delete ui;
}
