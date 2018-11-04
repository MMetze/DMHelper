#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class OptionsContainer;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(OptionsContainer& settings, QWidget *parent = 0);
    ~SettingsDialog();

private:
    Ui::SettingsDialog *ui;

    OptionsContainer& _settings;
};

#endif // SETTINGSDIALOG_H
