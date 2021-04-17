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
    ui->frame->setSettings(&settings);

    ui->edtCacheDirectory->setText(settings.getCacheDirectory());

    connect(ui->edtCacheDirectory, &QLineEdit::textChanged, &_settings, &DMC_OptionsContainer::setCacheDirectory);
    connect(ui->btnBrowseCache, &QAbstractButton::clicked, this, &DMC_SettingsDialog::browseCache);
}

DMC_SettingsDialog::~DMC_SettingsDialog()
{
    delete ui;
}

void DMC_SettingsDialog::browseCache()
{
    QString selectedDir = QFileDialog::getExistingDirectory(this,
                                                            QString("Select Cache Directory"),
                                                            _settings.getCacheDirectory());

    if(!selectedDir.isEmpty())
        _settings.setCacheDirectory(selectedDir);
}
