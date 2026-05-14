#ifndef CAMPAIGNMIGRATIONDIALOG_H
#define CAMPAIGNMIGRATIONDIALOG_H

#include <QDialog>

namespace Ui {
class CampaignMigrationDialog;
}

class CampaignMigrationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CampaignMigrationDialog(QWidget *parent = nullptr);
    ~CampaignMigrationDialog();

private:
    Ui::CampaignMigrationDialog *ui;
};

#endif // CAMPAIGNMIGRATIONDIALOG_H
