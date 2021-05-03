#include "connectionuserdialog.h"
#include "ui_connectionuserdialog.h"

ConnectionUserDialog::ConnectionUserDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionUserDialog)
{
    ui->setupUi(this);
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
