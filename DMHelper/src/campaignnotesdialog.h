#ifndef CAMPAIGNNOTESDIALOG_H
#define CAMPAIGNNOTESDIALOG_H

#include <QDialog>

namespace Ui {
class CampaignNotesDialog;
}

class CampaignNotesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CampaignNotesDialog(QStringList notes, QWidget *parent = nullptr);
    ~CampaignNotesDialog();

    QString getNotes() const;

private:
    Ui::CampaignNotesDialog *ui;
};

#endif // CAMPAIGNNOTESDIALOG_H
