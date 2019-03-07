#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include "optionscontainer.h"
#include "dmconstants.h"

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(OptionsContainer* options, QWidget *parent = nullptr);
    ~OptionsDialog();

    OptionsContainer* getOptions() const;

private slots:
    void browseBestiary();
#ifdef INCLUDE_CHASE_SUPPORT
    void browseChase();
#endif

private:
    Ui::OptionsDialog *ui;

    OptionsContainer* _options;
};

#endif // OPTIONSDIALOG_H
