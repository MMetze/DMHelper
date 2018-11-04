#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include "optionscontainer.h"

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(OptionsContainer* options, QWidget *parent = 0);
    ~OptionsDialog();

    OptionsContainer* getOptions() const;

private slots:
    void browseBestiary();
    void browseChase();

private:
    Ui::OptionsDialog *ui;

    OptionsContainer* _options;
};

#endif // OPTIONSDIALOG_H
