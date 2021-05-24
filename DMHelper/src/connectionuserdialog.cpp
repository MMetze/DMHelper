#include "connectionuserdialog.h"
#include "ui_connectionuserdialog.h"
#include <QMessageBox>
#include <QDebug>

ConnectionUserDialog::ConnectionUserDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionUserDialog)
{
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ConnectionUserDialog::tryAccept);
}

ConnectionUserDialog::~ConnectionUserDialog()
{
    delete ui;
}

QString ConnectionUserDialog::getUsername() const
{
    return ui->edtUsername->text();
}

QString ConnectionUserDialog::getPassword() const
{
    return ui->edtPassword->text();
}

bool ConnectionUserDialog::doesPasswordMatch() const
{
    return((!ui->edtPassword->text().isEmpty()) &&
           (ui->edtPassword->text() == ui->edtRepeat->text()));
}

QString ConnectionUserDialog::getEmail() const
{
    return ui->edtEmail->text();
}

QString ConnectionUserDialog::getScreenName() const
{
    return ui->edtScreenName->text();
}

void ConnectionUserDialog::tryAccept()
{
    if(doesPasswordMatch())
    {
        accept();
    }
    else
    {
        qDebug() << "[ConnectionUserDialog] Passwords don't match, can't create user...";
        QMessageBox::critical(this, QString("Passwords don't match"), QString("The passwords don't match, the new user can't be created!"));
    }
}
