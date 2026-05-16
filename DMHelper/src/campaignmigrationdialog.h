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
    static constexpr int LegacyModeResult = 2;

    explicit CampaignMigrationDialog(QWidget *parent = nullptr);
    ~CampaignMigrationDialog();

    QString getFilesDirectory() const;
    void setDefaultFilesDirectory(const QString& defaultDir);

private slots:
    void handleBrowseFilesDirectory();
    void handleLegacy();

private:
    Ui::CampaignMigrationDialog *ui;
};

#endif // CAMPAIGNMIGRATIONDIALOG_H
