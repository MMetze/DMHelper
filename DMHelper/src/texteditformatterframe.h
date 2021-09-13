#ifndef TEXTEDITFORMATTERFRAME_H
#define TEXTEDITFORMATTERFRAME_H

#include <QObject>
#include <QColor>

/*
#include <QFrame>

namespace Ui {
class TextEditFormatterFrame;
}
*/

class QTextEdit;

class TextEditFormatterFrame : public QObject
{
    Q_OBJECT

public:
    explicit TextEditFormatterFrame(QWidget *parent = nullptr);
    ~TextEditFormatterFrame();

    void setTextEdit(QTextEdit* textEdit);

public slots:
    void loadCurrentFormat();
    void setFont(const QString& fontFamily);
    void setFontSize(int fontSize);
    void setBold(bool bold);
    void setItalics(bool italics);
    void setUnterline(bool underline);
    void setColor(const QColor& color);
    void setAlignment(Qt::Alignment alignment);

signals:
    void fontFamilyChanged(const QString& fontFamily);
    void fontSizeChanged(int fontSize);
    void fontBoldChanged(bool fontBold);
    void fontItalicsChanged(bool fontItalics);
    void fontUnderlineChanged(bool fontItalics);
    void alignmentChanged(Qt::Alignment alignment);
    void colorChanged(const QColor& color);

protected:
//    Ui::TextEditFormatterFrame *ui;

    QTextEdit* _textEdit;
};

#endif // TEXTEDITFORMATTERFRAME_H
