#ifndef NEWCAMPAIGNDIALOG_H
#define NEWCAMPAIGNDIALOG_H

#include <QDialog>

namespace Ui {
class NewCampaignDialog;
}

class NewCampaignDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewCampaignDialog(QWidget *parent = nullptr);
    ~NewCampaignDialog();

    QString getCampaignName() const;
    QString getInitiative() const;
    QString getCharacterDataFile() const;
    QString getCharacterUIFile() const;
    bool isCombatantDone() const;

private slots:
    void handleRulesetSelected();
    void handleCharacterDataBrowse();
    void handleCharacterUIBrowse();

private:
    Ui::NewCampaignDialog *ui;
};

#endif // NEWCAMPAIGNDIALOG_H
