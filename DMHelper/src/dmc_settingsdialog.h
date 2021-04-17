#ifndef DMC_SETTINGSDIALOG_H
#define DMC_SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class DMC_SettingsDialog;
}

class DMC_OptionsContainer;

class DMC_SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DMC_SettingsDialog(DMC_OptionsContainer& settings, QWidget *parent = 0);
    ~DMC_SettingsDialog();

protected slots:
    void browseCache();

private:
    Ui::DMC_SettingsDialog *ui;
    DMC_OptionsContainer& _settings;
};

#endif // SETTINGSDIALOG_H
