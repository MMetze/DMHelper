#ifndef SCROLLINGTEXTWINDOW_H
#define SCROLLINGTEXTWINDOW_H

#include <QMainWindow>
#include <QElapsedTimer>

namespace Ui {
class ScrollingTextWindow;
}

class EncounterScrollingText;

class ScrollingTextWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ScrollingTextWindow(const EncounterScrollingText& encounter, QWidget *parent = 0);
    ~ScrollingTextWindow();

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void showEvent(QShowEvent *event);
    virtual void timerEvent(QTimerEvent *event);
    virtual void closeEvent(QCloseEvent *event);

protected slots:
    void rewind();
    void togglePlay(bool checked);

private:
    void prepareScene();
    void prepareImages();
    void drawScene();

    Ui::ScrollingTextWindow *ui;

    const EncounterScrollingText& _encounter;
    QRect _targetRect;
    QPoint _textPos;
    int _previousHeight;
    int _timerId;
    QElapsedTimer _elapsed;
    QImage _rawBackground;
    QPixmap _scaledBackground;
    QPixmap _textImage;
};

#endif // SCROLLINGTEXTWINDOW_H
