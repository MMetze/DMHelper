#ifndef TEXTEDITFORMATTERFRAME_H
#define TEXTEDITFORMATTERFRAME_H

#include <QFrame>

namespace Ui {
class TextEditFormatterFrame;
}

class QTextEdit;

class TextEditFormatterFrame : public QFrame
{
    Q_OBJECT

public:
    explicit TextEditFormatterFrame(QWidget *parent = nullptr);
    ~TextEditFormatterFrame();

    void setTextEdit(QTextEdit* textEdit);

protected slots:
    void loadCurrentFormat();
    void setFont();
    void setFontSize();
    void setBold(bool bold);
    void setItalics(bool italics);
    void setUnterline(bool underline);
    void setColor();
    void setAlignment(int id);

protected:
    Ui::TextEditFormatterFrame *ui;

    QTextEdit* _textEdit;
};

#endif // TEXTEDITFORMATTERFRAME_H
