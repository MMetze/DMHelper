#include "campaignmigrationdialog.h"
#include "ui_campaignmigrationdialog.h"
#include <QFileDialog>
#include <QDir>

CampaignMigrationDialog::CampaignMigrationDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CampaignMigrationDialog)
{
    ui->setupUi(this);
    connect(ui->btnBrowseFilesDirectory, &QPushButton::clicked, this, &CampaignMigrationDialog::handleBrowseFilesDirectory);
    connect(ui->btnLegacy, &QPushButton::clicked, this, &CampaignMigrationDialog::handleLegacy);
}

CampaignMigrationDialog::~CampaignMigrationDialog()
{
    delete ui;
}

QString CampaignMigrationDialog::getFilesDirectory() const
{
    return ui->edtFilesDirectory->text();
}

void CampaignMigrationDialog::setDefaultFilesDirectory(const QString& defaultDir)
{
    ui->edtFilesDirectory->setText(defaultDir);
}

void CampaignMigrationDialog::handleBrowseFilesDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Files Directory"), QDir::homePath());
    if(!dir.isEmpty())
        ui->edtFilesDirectory->setText(dir);
}

void CampaignMigrationDialog::handleLegacy()
{
    done(LegacyModeResult);
}
