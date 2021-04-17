#include "dmc_connectionsettingsframe.h"
#include "ui_dmc_connectionsettingsframe.h"
#include "dmc_optionscontainer.h"
#include "dmc_connectioninviteseditdialog.h"
#include "dmc_connectionuserdialog.h"
#include "dmhnetworkmanager.h"
#include <QGuiApplication>
#include <QScreen>

DMC_ConnectionSettingsFrame::DMC_ConnectionSettingsFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::DMC_ConnectionSettingsFrame),
    _settings(nullptr),
    _networkManager(new DMHNetworkManager(DMHLogon(), this))
{
    ui->setupUi(this);

    connect(ui->edtURL, &QLineEdit::textChanged, this, &DMC_ConnectionSettingsFrame::urlTextChanged);
    connect(ui->cmbInvites, &QComboBox::currentTextChanged, this, &DMC_ConnectionSettingsFrame::inviteChanged);
    connect(ui->btnReset, &QAbstractButton::clicked, this, &DMC_ConnectionSettingsFrame::resetUrl);
    connect(ui->btnCreateUser, &QAbstractButton::clicked, this, &DMC_ConnectionSettingsFrame::createUser);
    connect(ui->btnEditInvites, &QAbstractButton::clicked, this, &DMC_ConnectionSettingsFrame::editInvites);

    connect(_networkManager, &DMHNetworkManager::createUserComplete, this, &DMC_ConnectionSettingsFrame::userCreated);
}

DMC_ConnectionSettingsFrame::~DMC_ConnectionSettingsFrame()
{
    delete ui;
}

void DMC_ConnectionSettingsFrame::setSettings(DMC_OptionsContainer* settings)
{
    if(!settings)
        return;

    _settings = settings;

    ui->edtURL->setText(_settings->getURLString());
    ui->edtUserName->setText(_settings->getUserName());
    ui->chkSavePassword->setChecked(_settings->getSavePassword());
    ui->edtPassword->setText(_settings->getPassword());

    populateInvites();

    connect(ui->edtUserName, &QLineEdit::textChanged, _settings, &DMC_OptionsContainer::setUserName);
    connect(ui->edtPassword, &QLineEdit::textChanged, _settings, &DMC_OptionsContainer::setPassword);
    connect(ui->chkSavePassword, &QCheckBox::toggled, _settings, &DMC_OptionsContainer::setSavePassword);

    if(ui->edtURL->text().isEmpty())
        ui->edtURL->setFocus();
    else if(ui->edtUserName->text().isEmpty())
        ui->edtUserName->setFocus();
    else if(ui->edtPassword->text().isEmpty())
        ui->edtPassword->setFocus();
}

void DMC_ConnectionSettingsFrame::resetUrl()
{
    ui->edtURL->setText(QString("https://dmh.wwpd.de"));
}

void DMC_ConnectionSettingsFrame::createUser()
{
    if(!_settings)
        return;

    DMC_ConnectionUserDialog dlg;
    QScreen* primary = QGuiApplication::primaryScreen();
    if(primary)
        dlg.resize(primary->availableSize().width() / 4, primary->availableSize().height() / 3);

    if(dlg.exec() == QDialog::Accepted)
    {
        if(dlg.doesPasswordMatch())
        {
            _networkManager->setLogon(DMHLogon(_settings->getURLString(), QString(), QString(), QString(), QString()));
            _networkManager->createUser(dlg.getUsername(), dlg.getPassword(), dlg.getEmail(), dlg.getScreenName());
        }
    }
}

void DMC_ConnectionSettingsFrame::userCreated(int requestID, const QString& username, const QString& userId, const QString& email)
{
    Q_UNUSED(requestID);
    Q_UNUSED(email);

    ui->edtUserName->setText(username);
    ui->edtPassword->setText(QString());

    if(_settings)
        _settings->setUserId(userId);
}

void DMC_ConnectionSettingsFrame::urlTextChanged(const QString& text)
{
    if(!_settings)
        return;

    QString correctedText = text;
    if(correctedText.contains("\\"))
        correctedText.replace("\\", "/");

    _settings->setURLString(correctedText);
}

void DMC_ConnectionSettingsFrame::inviteChanged(const QString& invite)
{
    if(!_settings)
        return;

    _settings->setCurrentInvite(invite);
}

void DMC_ConnectionSettingsFrame::editInvites()
{
    if(!_settings)
        return;

    DMC_ConnectionInvitesEditDialog dlg(*_settings);
    dlg.exec();
    populateInvites();
}

void DMC_ConnectionSettingsFrame::populateInvites()
{
    if(!_settings)
        return;

    ui->cmbInvites->clear();

    QStringList invites = _settings->getInvites();
    for(QString invite : invites)
    {
        if(!invite.isEmpty())
        {
            QString entry = _settings->getInviteName(invite) + QString(" (") + invite + QString(")");
            ui->cmbInvites->addItem(entry, invite);
        }
    }

    for(int i = 0; i < ui->cmbInvites->count(); ++i)
    {
        if(ui->cmbInvites->currentData().toString() == _settings->getCurrentInvite())
        {
            ui->cmbInvites->setCurrentIndex(i);
            return;
        }
    }

    if((ui->cmbInvites->count() > 0) && (_settings->getCurrentInvite().isEmpty()))
        _settings->setCurrentInvite(ui->cmbInvites->currentData().toString());
}
