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
    explicit NewCampaignDialog(const QString& rulesetName = QString(), QWidget *parent = nullptr);
    ~NewCampaignDialog();

    QString getCampaignName() const;
    QString getInitiativeType() const;
    QString getInitiativeDescription() const;
    QString getCharacterDataFile() const;
    QString getCharacterUIFile() const;
    QString getBestiaryFile() const;
    QString getMonsterDataFile() const;
    QString getMonsterUIFile() const;
    bool isCombatantDone() const;

    QString getRuleset() const;
    void setRuleset(const QString& rulesetName);

private slots:
    void handleRulesetSelected();
    void handleCharacterDataBrowse();
    void handleCharacterUIBrowse();
    void handleBestiaryFileBrowse();
    void handleMonsterDataBrowse();
    void handleMonsterUIBrowse();

private:
    Ui::NewCampaignDialog *ui;
};

#endif // NEWCAMPAIGNDIALOG_H
