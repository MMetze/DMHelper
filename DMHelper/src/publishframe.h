#ifndef PUBLISHFRAME_H
#define PUBLISHFRAME_H

#include <QWidget>

class QLabel;
class QScrollArea;

class PublishFrame : public QWidget
{
    Q_OBJECT

public:
    explicit PublishFrame(QWidget *parent = nullptr);
    ~PublishFrame();

    virtual QSize sizeHint() const;

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
    virtual void resizeEvent(QResizeEvent * event);
    virtual void mouseMoveEvent(QMouseEvent * event);
    virtual void keyPressEvent(QKeyEvent * event);
    virtual void focusInEvent(QFocusEvent *event);

    void setScaledImg();

private:
    QScrollArea* _scrollArea;
    QLabel* _label;
    QLabel* _arrow;
    QImage _publishImg;
    bool _isScaled;
    bool _arrowVisible;
    QPointF _arrowPosition;
};

#endif // PUBLISHFRAME_H
