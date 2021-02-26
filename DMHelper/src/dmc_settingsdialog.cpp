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

    connect(ui->edtURL, &QLineEdit::textChanged, this, &DMC_SettingsDialog::textChanged);
    connect(ui->edtUserName, &QLineEdit::textChanged, &_settings, &DMC_OptionsContainer::setUserName);
    connect(ui->edtPassword, &QLineEdit::textChanged, &_settings, &DMC_OptionsContainer::setPassword);
    connect(ui->edtSession, &QLineEdit::textChanged, &_settings, &DMC_OptionsContainer::setSession);
}

DMC_SettingsDialog::~DMC_SettingsDialog()
{
    delete ui;
}

void DMC_SettingsDialog::textChanged(const QString& text)
{
    QString correctedText = text;
    if(correctedText.contains("\\"))
        correctedText.replace("\\", "/");

    _settings.setURLString(correctedText);
}
