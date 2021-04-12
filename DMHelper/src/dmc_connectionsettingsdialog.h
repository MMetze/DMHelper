#ifndef DMC_CONNECTIONSETTINGSDIALOG_H
#define DMC_CONNECTIONSETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class DMC_ConnectionSettingsDialog;
}

class DMC_OptionsContainer;

class DMC_ConnectionSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DMC_ConnectionSettingsDialog(DMC_OptionsContainer& options, QWidget *parent = nullptr);
    ~DMC_ConnectionSettingsDialog();

protected slots:
    void inviteChanged(const QString& invite);
    void editInvites();
    void populateInvites();

private:
    Ui::DMC_ConnectionSettingsDialog *ui;

    DMC_OptionsContainer& _options;
};

#endif // DMC_CONNECTIONSETTINGSDIALOG_H
