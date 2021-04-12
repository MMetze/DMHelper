#include "dmc_settingsdialog.h"
#include "dmc_optionscontainer.h"
#include "ui_dmc_settingsdialog.h"
#include "dmc_connectioninviteseditdialog.h"
#include <QFileDialog>

DMC_SettingsDialog::DMC_SettingsDialog(DMC_OptionsContainer& settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DMC_SettingsDialog),
    _settings(settings)
{
    ui->setupUi(this);

    ui->edtURL->setText(_settings.getURLString());
    ui->edtUserName->setText(_settings.getUserName());
    ui->edtPassword->setText(_settings.getPassword());

    populateInvites();

    ui->edtCacheDirectory->setText(settings.getCacheDirectory());

    connect(ui->edtURL, &QLineEdit::textChanged, this, &DMC_SettingsDialog::urlTextChanged);
    connect(ui->edtUserName, &QLineEdit::textChanged, &_settings, &DMC_OptionsContainer::setUserName);
    connect(ui->edtPassword, &QLineEdit::textChanged, &_settings, &DMC_OptionsContainer::setPassword);
    connect(ui->cmbInvite, &QComboBox::currentTextChanged, this, &DMC_SettingsDialog::inviteChanged);
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

void DMC_SettingsDialog::inviteChanged(const QString& invite)
{
    _settings.setCurrentInvite(invite);
}

void DMC_SettingsDialog::editInvites()
{
    DMC_ConnectionInvitesEditDialog dlg(_settings);
    dlg.exec();
    populateInvites();
}

void DMC_SettingsDialog::populateInvites()
{
    ui->cmbInvite->clear();

    QStringList invites = _settings.getInvites();
    for(QString invite : invites)
    {
        if(!invite.isEmpty())
        {
            QString entry = _settings.getInviteName(invite) + QString("(") + invite + QString(")");
            ui->cmbInvite->addItem(entry, invite);
        }
    }
    for(int i = 0; i < ui->cmbInvite->count(); ++i)
        if(ui->cmbInvite->currentData().toString() == _settings.getCurrentInvite())
            ui->cmbInvite->setCurrentIndex(i);

}
