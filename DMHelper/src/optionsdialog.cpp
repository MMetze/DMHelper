#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QIntValidator>

OptionsDialog::OptionsDialog(OptionsContainer* options, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog),
    _options(options)
{
    ui->setupUi(this);

    if(_options)
    {
        ui->edtBestiary->setText(_options->getBestiaryFileName());
        ui->edtChase->setText(_options->getChaseFileName());
        ui->chkShowAnimations->setChecked(_options->getShowAnimations());
        ui->chkShowOnDeck->setChecked(_options->getShowOnDeck());
        ui->chkShowCountdown->setChecked(_options->getShowCountdown());
        ui->edtCountdownDuration->setValidator(new QIntValidator(1,1000,this));
        ui->edtCountdownDuration->setText(QString::number(_options->getCountdownDuration()));
        ui->chkEnableNetworkClient->setChecked(_options->getNetworkEnabled());
        ui->edtUserName->setText(_options->getUserName());
        ui->edtURL->setText(_options->getURLString());
        ui->chkSavePassword->setChecked(_options->getSavePassword());
        ui->edtPassword->setText(_options->getPassword());
        ui->edtSessionID->setText(_options->getSessionID());
        ui->edtInviteID->setText(_options->getInviteID());

        ui->lblURL->setEnabled(_options->getNetworkEnabled());
        ui->edtURL->setEnabled(_options->getNetworkEnabled());
        ui->lblUsername->setEnabled(_options->getNetworkEnabled());
        ui->edtUserName->setEnabled(_options->getNetworkEnabled());
        ui->lblPassword->setEnabled(_options->getNetworkEnabled());
        ui->chkSavePassword->setEnabled(_options->getNetworkEnabled());
        ui->edtPassword->setEnabled(_options->getNetworkEnabled());
        ui->lblSessionID->setEnabled(_options->getNetworkEnabled());
        ui->edtSessionID->setEnabled(_options->getNetworkEnabled());
        ui->lblInviteID->setEnabled(_options->getNetworkEnabled());
        ui->edtInviteID->setEnabled(_options->getNetworkEnabled());
        ui->btnGenerateInvite->setEnabled(_options->getNetworkEnabled());

        connect(ui->btnBestiary,SIGNAL(clicked()),this,SLOT(browseBestiary()));
        connect(ui->btnChase,SIGNAL(clicked()),this,SLOT(browseChase()));
        connect(ui->chkShowAnimations,SIGNAL(clicked(bool)),_options,SLOT(setShowAnimations(bool)));
        connect(ui->chkShowOnDeck,SIGNAL(clicked(bool)),_options,SLOT(setShowOnDeck(bool)));
        connect(ui->chkShowCountdown,SIGNAL(clicked(bool)),_options,SLOT(setShowCountdown(bool)));
        connect(ui->edtCountdownDuration,SIGNAL(textChanged(QString)),_options,SLOT(setCountdownDuration(QString)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), _options, SLOT(setNetworkEnabled(bool)));
        connect(ui->edtURL, SIGNAL(textChanged(QString)), _options, SLOT(setURLString(QString)));
        connect(ui->edtUserName, SIGNAL(textChanged(QString)), _options, SLOT(setUserName(QString)));
        connect(ui->chkSavePassword, SIGNAL(clicked(bool)), _options, SLOT(setSavePassword(bool)));
        connect(ui->edtPassword, SIGNAL(textChanged(QString)), _options, SLOT(setPassword(QString)));
        connect(ui->edtSessionID, SIGNAL(textChanged(QString)), _options, SLOT(setSessionID(QString)));
        connect(ui->edtInviteID, SIGNAL(textChanged(QString)), _options, SLOT(setInviteID(QString)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->lblURL, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->edtURL, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->lblUsername, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->edtUserName, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->lblPassword, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->chkSavePassword, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->edtPassword, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->lblSessionID, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->edtSessionID, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->lblInviteID, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->edtInviteID, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->btnGenerateInvite, SLOT(setEnabled(bool)));

    }
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

OptionsContainer* OptionsDialog::getOptions() const
{
    return _options;
}

void OptionsDialog::browseBestiary()
{
    QString bestiaryFileName = QFileDialog::getOpenFileName(this,QString("Select Bestiary File"),QString(),QString("XML files (*.xml)"));

    if((bestiaryFileName.isEmpty()) || (!QFile::exists(bestiaryFileName)))
        return;

    ui->edtBestiary->setText(bestiaryFileName);
    _options->setBestiaryFileName(bestiaryFileName);
}

void OptionsDialog::browseChase()
{
    QString chaseFileName = QFileDialog::getOpenFileName(this,QString("Select Chase File"),QString(),QString("XML files (*.xml)"));

    if((chaseFileName.isEmpty()) || (!QFile::exists(chaseFileName)))
        return;

    ui->edtChase->setText(chaseFileName);
    _options->setChaseFileName(chaseFileName);
}
