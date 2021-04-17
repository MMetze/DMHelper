#include "dmc_connectioninviteseditdialog.h"
#include "ui_dmc_connectioninviteseditdialog.h"
#include "dmc_optionscontainer.h"
#include <QInputDialog>

DMC_ConnectionInvitesEditDialog::DMC_ConnectionInvitesEditDialog(DMC_OptionsContainer& options, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DMC_ConnectionInvitesEditDialog),
    _options(options)
{
    ui->setupUi(this);

    connect(ui->btnAddInvite, &QAbstractButton::clicked, this, &DMC_ConnectionInvitesEditDialog::addInvite);
    connect(ui->btnRenameInvite, &QAbstractButton::clicked, this, &DMC_ConnectionInvitesEditDialog::renameInvite);
    connect(ui->btnRemoveInvite, &QAbstractButton::clicked, this, &DMC_ConnectionInvitesEditDialog::removeInvite);
}

DMC_ConnectionInvitesEditDialog::~DMC_ConnectionInvitesEditDialog()
{
    delete ui;
}

void DMC_ConnectionInvitesEditDialog::addInvite()
{
    QString inviteName = QInputDialog::getText(this, QString("DMHelper Edit Invites"), QString("Enter a name for the invite: "));
    if(inviteName.isEmpty())
        return;

    QString invite = QInputDialog::getText(this, QString("DMHelper Edit Invites"), QString("Enter the invite: "));
    if(invite.isEmpty())
        return;

    _options.addInvite(invite, inviteName);
    populateInvites();
}

void DMC_ConnectionInvitesEditDialog::renameInvite()
{
    QList<QTableWidgetItem*> items = ui->tableWidget->selectedItems();
    if((items.count() < 2) || (!items.at(1)))
        return;

    QString invite = items.at(1)->text();
    if(invite.isEmpty())
        return;

    QString newInviteName = QInputDialog::getText(this, QString("DMHelper Edit Invites"), QString("Enter the new name for this invite: "));
    if(newInviteName.isEmpty())
        return;

    _options.setInviteName(invite, newInviteName);
    populateInvites();
}

void DMC_ConnectionInvitesEditDialog::removeInvite()
{
    QList<QTableWidgetItem*> items = ui->tableWidget->selectedItems();
    if((items.count() < 2) || (!items.at(1)))
        return;

    QString invite = items.at(1)->text();
    if(invite.isEmpty())
        return;

    _options.removeInvite(invite);
    populateInvites();
}

void DMC_ConnectionInvitesEditDialog::populateInvites()
{
    ui->tableWidget->clear();

    QStringList invites = _options.getInvites();
    ui->tableWidget->setRowCount(invites.count());
    for(int i = 0; i < invites.count(); ++i)
    {
        QString invite = invites.at(i);

        QTableWidgetItem* nameItem = new QTableWidgetItem(_options.getInviteName(invite));
        nameItem->setToolTip(nameItem->text());
        ui->tableWidget->setItem(i, 0, nameItem);

        QTableWidgetItem* sessionItem = new QTableWidgetItem(invite);
        nameItem->setToolTip(sessionItem->text());
        ui->tableWidget->setItem(i, 1, sessionItem);
    }
}

