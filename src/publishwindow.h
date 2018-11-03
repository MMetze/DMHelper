#ifndef PUBLISHWINDOW_H
#define PUBLISHWINDOW_H

#include <QMainWindow>

class PublishFrame;

class PublishWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit PublishWindow(const QString& title, QWidget *parent = 0);

signals:
    void visibleChanged(bool visible);
    void positionChanged(const QPointF& position);
    void frameResized(const QSize& newSize);

public slots:
    void setImage(QImage img);
    void setImageNoScale(QImage img);
    void setArrowVisible(bool visible);
    void setArrowPosition(const QPointF& position);

protected:
    virtual void keyPressEvent(QKeyEvent * event);

private:
    PublishFrame* _publishFrame;
};

#endif // PUBLISHWINDOW_H
