#ifndef CLOCKFRAME_H
#define CLOCKFRAME_H

#include <QFrame>
#include <QVector2D>

namespace Ui {
class ClockFrame;
}

class ClockFrame : public QFrame
{
    Q_OBJECT

public:
    explicit ClockFrame(QWidget *parent = 0);
    ~ClockFrame();

    float getSmallHandAngle() const;
    float getLargeHandAngle() const;
    bool isSmallHandActive() const;

public slots:
    void setSmallHandAngle(float angle);
    void setLargeHandAngle(float angle);
    void setLargeHandAngleTarget(float target);
    void setSmallHandActive(bool active);

signals:
    void handsChanged();

protected:
    virtual void resizeEvent(QResizeEvent *event);

    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

    virtual void timerEvent(QTimerEvent *event);

private:

    void scaleClockFrame(int scaleFactor);
    void positionClockHands();

    enum MouseDownValue
    {
        MouseDownValue_up = 0,
        MouseDownValue_small,
        MouseDownValue_large
    };

    Ui::ClockFrame *ui;

    QImage _astrolobe;
    QImage _smallHand;
    QImage _largeHand;

    int _scaleFactor;
    QImage _smallHandScaled;
    QImage _largeHandScaled;

    float _smallHandAngle;
    float _largeHandAngle;
    float _largeHandAngleTarget;
    bool _smallHandActive;
    int _timerId;

    MouseDownValue _mouseDown;
    QVector2D _lastMousePos;
};

#endif // CLOCKFRAME_H
