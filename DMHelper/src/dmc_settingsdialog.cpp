#include "dmc_settingsdialog.h"
#include "dmc_optionscontainer.h"
#include "ui_dmc_settingsdialog.h"
#include <QFileDialog>

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
    ui->edtCacheDirectory->setText(settings.getCacheDirectory());

    connect(ui->edtURL, &QLineEdit::textChanged, this, &DMC_SettingsDialog::urlTextChanged);
    connect(ui->edtUserName, &QLineEdit::textChanged, &_settings, &DMC_OptionsContainer::setUserName);
    connect(ui->edtPassword, &QLineEdit::textChanged, &_settings, &DMC_OptionsContainer::setPassword);
    connect(ui->edtSession, &QLineEdit::textChanged, &_settings, &DMC_OptionsContainer::setSession);
    connect(ui->edtCacheDirectory, &QLineEdit::textChanged, &_settings, &DMC_OptionsContainer::setCacheDirectory);
    connect(ui->btnBrowseCache, &QAbstractButton::clicked, this, &DMC_SettingsDialog::browseCache);
}

DMC_SettingsDialog::~DMC_SettingsDialog()
{
    delete ui;
}

void DMC_SettingsDialog::urlTextChanged(const QString& text)
{
    QString correctedText = text;
    if(correctedText.contains("\\"))
        correctedText.replace("\\", "/");

    _settings.setURLString(correctedText);
}

void DMC_SettingsDialog::browseCache()
{
    QString selectedDir = QFileDialog::getExistingDirectory(this,
                                                            QString("Select Cache Directory"),
                                                            _settings.getCacheDirectory());

    if(!selectedDir.isEmpty())
        _settings.setCacheDirectory(selectedDir);
}
