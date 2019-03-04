#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "dmconstants.h"
#include "bestiary.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    QString licenseText;

    licenseText += QString("Quick reference icons provided by http://game-icons.net/\n\n\n");

    if(Bestiary::Instance())
        licenseText += Bestiary::Instance()->getLicenseText().join(QString("\n")).append(QString("\n"));

    ui->edtLicenses->setText(licenseText);

    ui->lblVersion->setText(QString::number(DMHelper::DMHELPER_MAJOR_VERSION) + "." + QString::number(DMHelper::DMHELPER_MINOR_VERSION));
    ui->lblBestiaryVersion->setText(Bestiary::getExpectedVersion());
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
