#ifndef TEXTBROWSERMARGINS_H
#define TEXTBROWSERMARGINS_H

#include <QTextBrowser>

class TextBrowserMargins : public QTextBrowser
{
    Q_OBJECT
public:
    explicit TextBrowserMargins(QWidget *parent = nullptr);

public slots:
    void setTextWidth(int textWidth);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;

    void updateTextWidth();

private:
    int _textWidth;
};

#endif // TEXTBROWSERMARGINS_H
