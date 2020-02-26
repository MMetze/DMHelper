#ifndef LEGALDIALOG_H
#define LEGALDIALOG_H

#include <QDialog>

namespace Ui {
class LegalDialog;
}

class LegalDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LegalDialog(QWidget *parent = nullptr);
    ~LegalDialog();

    bool isUpdatesEnabled() const;
    bool isStatisticsAccepted() const;

private slots:
    void okClicked();

private:
    Ui::LegalDialog *ui;
};

#endif // LEGALDIALOG_H
