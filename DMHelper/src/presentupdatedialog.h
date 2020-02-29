#ifndef PRESENTUPDATEDIALOG_H
#define PRESENTUPDATEDIALOG_H

#include <QDialog>

namespace Ui {
class PresentUpdateDialog;
}

class PresentUpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PresentUpdateDialog(const QString& newVersion, const QString& releaseNotes, QWidget *parent = nullptr);
    ~PresentUpdateDialog();

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::PresentUpdateDialog *ui;
};

#endif // PRESENTUPDATEDIALOG_H
