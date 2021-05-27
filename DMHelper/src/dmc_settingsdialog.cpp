#include "dmc_settingsdialog.h"
#include "dmc_optionscontainer.h"
#include "ui_dmc_settingsdialog.h"
#include <QFileDialog>

DMC_SettingsDialog::DMC_SettingsDialog(DMC_OptionsContainer& settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DMC_SettingsDialog),
    _settings(settings),
    _dirty(false)
{
    ui->setupUi(this);
    ui->frame->setSettings(&settings);

    ui->edtCacheDirectory->setText(settings.getCacheDirectory());

    connect(ui->edtCacheDirectory, &QLineEdit::textChanged, &_settings, &DMC_OptionsContainer::setCacheDirectory);
    connect(ui->btnBrowseCache, &QAbstractButton::clicked, this, &DMC_SettingsDialog::browseCache);

    connect(&settings, &DMC_OptionsContainer::urlStringChanged, this, &DMC_SettingsDialog::setDirty);
    connect(&settings, &DMC_OptionsContainer::userNameChanged, this, &DMC_SettingsDialog::setDirty);
    connect(&settings, &DMC_OptionsContainer::userIdChanged, this, &DMC_SettingsDialog::setDirty);
    connect(&settings, &DMC_OptionsContainer::passwordChanged, this, &DMC_SettingsDialog::setDirty);
    connect(&settings, &DMC_OptionsContainer::currentInviteChanged, this, &DMC_SettingsDialog::setDirty);
    connect(&settings, &DMC_OptionsContainer::inviteChanged, this, &DMC_SettingsDialog::setDirty);
    connect(&settings, &DMC_OptionsContainer::cacheDirectoryChanged, this, &DMC_SettingsDialog::setDirty);
}

DMC_SettingsDialog::~DMC_SettingsDialog()
{
    delete ui;
}

bool DMC_SettingsDialog::isDirty() const
{
    return _dirty;
}

int DMC_SettingsDialog::exec()
{
    _dirty = false;
    return QDialog::exec();
}

void DMC_SettingsDialog::logMessage(const QString& message)
{
    ui->frame->logMessage(message);
}

void DMC_SettingsDialog::browseCache()
{
    QString selectedDir = QFileDialog::getExistingDirectory(this,
                                                            QString("Select Cache Directory"),
                                                            _settings.getCacheDirectory());

    if(!selectedDir.isEmpty())
        _settings.setCacheDirectory(selectedDir);
}

void DMC_SettingsDialog::setDirty()
{
    _dirty = true;
}
