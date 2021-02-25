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
    explicit TextTranslateDialog(const QString& originalText, const QString& translatedText, QImage backgroundImage, QWidget *parent = nullptr);
    virtual ~TextTranslateDialog() override;

    virtual bool eventFilter(QObject *watched, QEvent *event) override;

    void getOriginalText(QString& originalText) const;
    void getTranslatedText(QString& translatedText) const;

public slots:
    void translateText();

signals:
    void publishImage(QImage img, QColor color);

protected:
    virtual void keyPressEvent(QKeyEvent * event) override;
    virtual void hideEvent(QHideEvent * event) override;
    virtual void resizeEvent(QResizeEvent *event) override;

protected slots:
    void rolld20();

private:
    QFont getSelectedFont(const QString& fontName, int pointSize, int weight, bool italic);

    Ui::TextTranslateDialog *ui;

    QImage _backgroundImage;
    QImage _backgroundImageScaled;
};

#endif // TEXTTRANSLATEDIALOG_H
