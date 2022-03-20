#ifndef WHATSNEWDIALOG_H
#define WHATSNEWDIALOG_H

#include <QDialog>

namespace Ui {
class WhatsNewDialog;
}

class WhatsNewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WhatsNewDialog(QWidget *parent = nullptr);
    ~WhatsNewDialog();

    virtual bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    virtual void closeEvent(QCloseEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    void scaleBackgroundImage();

private:
    Ui::WhatsNewDialog *ui;

    QImage _backgroundImage;
    QImage _backgroundImageScaled;
};

#endif // WHATSNEWDIALOG_H
