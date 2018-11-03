#ifndef DMSCREENDIALOG_H
#define DMSCREENDIALOG_H

#include <QDialog>

namespace Ui {
class DMScreenDialog;
}

class DMScreenDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DMScreenDialog(QWidget *parent = 0);
    ~DMScreenDialog();

public slots:
    void triggerRoll(const QString& link);

private:
    Ui::DMScreenDialog *ui;
};

#endif // DMSCREENDIALOG_H
