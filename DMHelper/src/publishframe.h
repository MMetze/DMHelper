#ifndef PUBLISHFRAME_H
#define PUBLISHFRAME_H

#include <QFrame>

class QLabel;
class QScrollArea;

class PublishFrame : public QFrame
{
    Q_OBJECT

public:
    explicit PublishFrame(QWidget *parent = nullptr);
    ~PublishFrame();

    virtual QSize sizeHint() const;
    virtual bool eventFilter(QObject *watched, QEvent *event);

signals:
    void arrowVisibleChanged(bool visible);
    void publishMouseDown(const QPointF& position);
    void publishMouseMove(const QPointF& position);
    void publishMouseRelease(const QPointF& position);
    void positionChanged(const QPointF& position);
    void frameResized(const QSize& newSize);
    void labelResized(const QSize& newSize);

public slots:
    void setImage(QImage img);
    void setImageNoScale(QImage img);
    void setArrowVisible(bool visible);
    void setArrowPosition(const QPointF& position);
    void setPointerFile(const QString& filename);

protected:
    virtual void resizeEvent(QResizeEvent * event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
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
