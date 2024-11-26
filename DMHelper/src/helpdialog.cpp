#include "helpdialog.h"
#include "ui_helpdialog.h"

HelpDialog::HelpDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::HelpDialog)
{
    ui->setupUi(this);

    connect(ui->btnGettingStarted, &QPushButton::clicked, this, &HelpDialog::openGettingStarted);
    connect(ui->btnUsersGuide, &QPushButton::clicked, this, &HelpDialog::openUsersGuide);
    connect(ui->btnOpenLogsDir, &QPushButton::clicked, this, &HelpDialog::openLogsDirectory);
    connect(ui->btnOpenBackupDir, &QPushButton::clicked, this, &HelpDialog::openBackupDirectory);
}

HelpDialog::~HelpDialog()
{
    delete ui;
}
