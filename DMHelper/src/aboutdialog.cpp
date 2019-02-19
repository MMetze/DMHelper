#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "dmconstants.h"
#include "bestiary.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    if(Bestiary::Instance())
        ui->edtLicenses->setText(Bestiary::Instance()->getLicenseText().join(QString("\n")));

    ui->lblVersion->setText(QString::number(DMHelper::DMHELPER_MAJOR_VERSION) + "." + QString::number(DMHelper::DMHELPER_MINOR_VERSION));
    ui->lblBestiaryVersion->setText(Bestiary::getExpectedVersion());
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
