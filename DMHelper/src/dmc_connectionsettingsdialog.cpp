#include "dmc_connectionsettingsdialog.h"
#include "ui_dmc_connectionsettingsdialog.h"
#include "dmc_optionscontainer.h"
#include "dmc_connectioninviteseditdialog.h"
#include "dmhlogon.h"

/*
 * C:\Users\greent>curl -X POST -d "user=m.metze2&password=12345asD&action=create" localhost/dmhelper/usr_mng.php -b XDEBUG_SESSION=xdebug-atom

<?xml version="1.0" encoding="UTF-8" ?><nodes><version>0.1.2</version><timestamp>2021-04-13 00:00:13</timestamp><mode><int>18</int><text>Create User</text></mode><state>0</state><error></error><data><user>550bbd57-d69d-4166-9d65-0bac0bf9da05</user><username>m.metze2</username><email></email></data></nodes>

jupp. du bekommst eine uuid = user und den username zurück.
`<?xml version="1.0" encoding="UTF-8" ?><nodes><version>0.1.2</version><timestamp>2021-04-13 00:08:54</timestamp><mode><int>18</int><text>Create User</text></mode><state>-1</state><error>Password too weak (min. 8 characters)!
Password must include at least one character!
Username already in use
</error><data></data></nodes>´


solange state= 0 -> alles gut.
-1 -> error
error codes in den error brackets.*/

DMC_ConnectionSettingsDialog::DMC_ConnectionSettingsDialog(DMC_OptionsContainer& options, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DMC_ConnectionSettingsDialog),
    _options(options)
{
    ui->setupUi(this);

    ui->edtURL->setText(_options.getURLString());
    ui->edtUserName->setText(_options.getUserName());
    ui->edtPassword->setText(_options.getPassword());

    populateInvites();
}

DMC_ConnectionSettingsDialog::~DMC_ConnectionSettingsDialog()
{
    delete ui;
}

void DMC_ConnectionSettingsDialog::inviteChanged(const QString& invite)
{
    _options.setCurrentInvite(invite);
}

void DMC_ConnectionSettingsDialog::editInvites()
{
    DMC_ConnectionInvitesEditDialog dlg(_options);
    dlg.exec();
    populateInvites();
}

void DMC_ConnectionSettingsDialog::populateInvites()
{
    ui->cmbInvites->clear();

    QStringList invites = _options.getInvites();
    for(QString invite : invites)
    {
        if(!invite.isEmpty())
        {
            QString entry = _options.getInviteName(invite) + QString("(") + invite + QString(")");
            ui->cmbInvites->addItem(entry, invite);
        }
    }
    for(int i = 0; i < ui->cmbInvites->count(); ++i)
        if(ui->cmbInvites->currentData().toString() == _options.getCurrentInvite())
            ui->cmbInvites->setCurrentIndex(i);
}
