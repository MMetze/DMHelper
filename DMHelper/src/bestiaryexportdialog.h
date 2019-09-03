#ifndef BESTIARYEXPORTDIALOG_H
#define BESTIARYEXPORTDIALOG_H

#include <QDialog>

namespace Ui {
class BestiaryExportDialog;
}

class BestiaryExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BestiaryExportDialog(QWidget *parent = nullptr);
    ~BestiaryExportDialog();

private slots:
    void selectAll();
    void selectNone();
    void massSelect(bool select);

    void exportSelected();

private:
    Ui::BestiaryExportDialog *ui;
};

#endif // BESTIARYEXPORTDIALOG_H
