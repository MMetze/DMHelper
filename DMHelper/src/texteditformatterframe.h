#ifndef TEXTEDITFORMATTERFRAME_H
#define TEXTEDITFORMATTERFRAME_H

#include <QObject>
#include <QColor>

class QTextEdit;
class QTextCharFormat;

class TextEditFormatterFrame : public QObject
{
    Q_OBJECT

public:
    explicit TextEditFormatterFrame(QWidget *parent = nullptr);
    ~TextEditFormatterFrame();

    void setTextEdit(QTextEdit* textEdit);

public slots:
    void loadCurrentFormat();
    void loadCurrentCharFormat(const QTextCharFormat &f);
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

    QTextEdit* _textEdit;
};

#endif // TEXTEDITFORMATTERFRAME_H
