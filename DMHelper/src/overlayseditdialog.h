#ifndef OVERLAYSEDITDIALOG_H
#define OVERLAYSEDITDIALOG_H

#include <QDialog>

namespace Ui {
class OverlaysEditDialog;
}

class OverlaysEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OverlaysEditDialog(QWidget *parent = nullptr);
    ~OverlaysEditDialog();

private:
    Ui::OverlaysEditDialog *ui;
};

#endif // OVERLAYSEDITDIALOG_H
