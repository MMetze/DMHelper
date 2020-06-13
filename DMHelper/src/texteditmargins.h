#ifndef TEXTEDITMARGINS_H
#define TEXTEDITMARGINS_H

#include <QTextEdit>

class TextEditMargins : public QTextEdit
{
public:
    explicit TextEditMargins(QWidget *parent = nullptr);

public slots:
    void setTextWidth(int textWidth);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;

    void updateTextWidth();

private:
    int _textWidth;
};

#endif // TEXTEDITMARGINS_H
