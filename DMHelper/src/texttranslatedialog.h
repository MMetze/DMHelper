#ifndef TEXTTRANSLATEDIALOG_H
#define TEXTTRANSLATEDIALOG_H

#include <QDialog>

namespace Ui {
class TextTranslateDialog;
}

class TextTranslateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextTranslateDialog(QWidget *parent = nullptr);
    ~TextTranslateDialog();

public slots:
    void translateText();
    void publishTextImage();

signals:
    void publishImage(QImage img, QColor color);

protected:
    virtual void keyPressEvent(QKeyEvent * event);
    virtual void hideEvent(QHideEvent * event);

private:
    Ui::TextTranslateDialog *ui;
};

#endif // TEXTTRANSLATEDIALOG_H
