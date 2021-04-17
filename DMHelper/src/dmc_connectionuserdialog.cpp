#include "dmc_connectionuserdialog.h"
#include "ui_dmc_connectionuserdialog.h"

DMC_ConnectionUserDialog::DMC_ConnectionUserDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DMC_ConnectionUserDialog)
{
    ui->setupUi(this);
}

DMC_ConnectionUserDialog::~DMC_ConnectionUserDialog()
{
    delete ui;
}

QString DMC_ConnectionUserDialog::getUsername() const
{
    return ui->edtUsername->text();
}

QString DMC_ConnectionUserDialog::getPassword() const
{
    return ui->edtPassword->text();
}

bool DMC_ConnectionUserDialog::doesPasswordMatch() const
{
    return((!ui->edtPassword->text().isEmpty()) &&
           (ui->edtPassword->text() == ui->edtRepeat->text()));
}

QString DMC_ConnectionUserDialog::getEmail() const
{
    return ui->edtEmail->text();
}

QString DMC_ConnectionUserDialog::getScreenName() const
{
    return ui->edtScreenName->text();
}
