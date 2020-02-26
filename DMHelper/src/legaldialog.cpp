#include "legaldialog.h"
#include "ui_legaldialog.h"
#include <QMessageBox>

LegalDialog::LegalDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LegalDialog)
{
    ui->setupUi(this);
    connect(ui->btnOK, &QPushButton::clicked, this, &LegalDialog::okClicked);

}

LegalDialog::~LegalDialog()
{
    delete ui;
}

bool LegalDialog::isUpdatesEnabled() const
{
    return ui->chkEnableUpdates->isChecked();
}

bool LegalDialog::isStatisticsAccepted() const
{
    return ui->chkAllowStatistics->isChecked();
}

void LegalDialog::okClicked()
{
    if(ui->chkEnableUpdates->isChecked() == false)
    {
        if(QMessageBox::warning(this,
                                QString("Checking for Updates disabled"),
                                QString("You have chosen that the DM Helper should not check occassionally for new updates. This means you won't be informed when a new version of the DM Helper becomes available. Are you sure?"),
                                QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
            return;
    }

    if(ui->chkAllowStatistics->isChecked() == false)
    {
        QMessageBox::information(this,
                                 QString("Anonymous statistics disabled"),
                                 QString("You have chosen that the DM Helper may not gather anonymous usage data. We respect your choice. If you would like to change this setting in the future, you can find it in the menu under Tools --> Options"));
    }

    accept();
}
