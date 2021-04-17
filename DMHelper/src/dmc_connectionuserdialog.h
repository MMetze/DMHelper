#ifndef DMC_CONNECTIONUSERDIALOG_H
#define DMC_CONNECTIONUSERDIALOG_H

#include <QDialog>

namespace Ui {
class DMC_ConnectionUserDialog;
}

class DMC_ConnectionUserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DMC_ConnectionUserDialog(QWidget *parent = nullptr);
    ~DMC_ConnectionUserDialog();

    QString getUsername() const;
    QString getPassword() const;
    bool doesPasswordMatch() const;
    QString getEmail() const;
    QString getScreenName() const;

private:
    Ui::DMC_ConnectionUserDialog *ui;
};

#endif // DMC_CONNECTIONUSERDIALOG_H
