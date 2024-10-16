#ifndef DMHWAITINGDIALOG_H
#define DMHWAITINGDIALOG_H

#include <QDialog>
#include <QElapsedTimer>

namespace Ui {
class DMHWaitingDialog;
}

/*
 *  Usage examples:

    DMHWaitingDialog dlg(QString("Please wait..."), this);
    QTimer::singleShot(5000, &dlg, &DMHWaitingDialog::accept);
    dlg.exec();

    or

    DMHWaitingDialog* _waitingDlg;
    _waitingDlg = new DMHWaitingDialog(QString("Contacting HeroForge..."), this);
    _waitingDlg->setModal(true);
    _waitingDlg->show();
 */

class DMHWaitingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DMHWaitingDialog(const QString& statusString = QString(), QWidget *parent = nullptr);
    ~DMHWaitingDialog();

public slots:
    void setStatus(const QString& statusString);
    void setSplitStatus(const QString& primary, const QString& secondary);

protected:
    virtual void closeEvent(QCloseEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;
    virtual void timerEvent(QTimerEvent *event) override;
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::DMHWaitingDialog *ui;

    QImage _dieImage;
    QImage _backgroundImage;
    QString _initialStatus;

    QElapsedTimer _elapsed;
    int _timerId;
    qreal _rotation;
};

#endif // DMHWAITINGDIALOG_H
