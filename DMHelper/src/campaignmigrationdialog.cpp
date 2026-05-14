#include "campaignmigrationdialog.h"
#include "ui_campaignmigrationdialog.h"

CampaignMigrationDialog::CampaignMigrationDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CampaignMigrationDialog)
{
    ui->setupUi(this);
}

CampaignMigrationDialog::~CampaignMigrationDialog()
{
    delete ui;
}
