#ifndef CONNECTIONUSERDIALOG_H
#define CONNECTIONUSERDIALOG_H

#include <QDialog>

namespace Ui {
class ConnectionUserDialog;
}

class ConnectionUserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionUserDialog(QWidget *parent = nullptr);
    ~ConnectionUserDialog();

    QString getUsername() const;
    QString getPassword() const;
    bool doesPasswordMatch() const;
    QString getEmail() const;
    QString getScreenName() const;

private slots:
    void tryAccept();

private:
    Ui::ConnectionUserDialog *ui;
};

#endif // CONNECTIONUSERDIALOG_H
