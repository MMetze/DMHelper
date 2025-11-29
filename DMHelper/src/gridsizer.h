#ifndef GRIDSIZER_H
#define GRIDSIZER_H

#include <QGraphicsObject>

class GridSizer : public QGraphicsObject
{
    Q_OBJECT

public:
    GridSizer(qreal size = 250.0, bool showAccept = true, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

    qreal getSize() const;
    qreal getHandleSize() const;

    void setSize(qreal size);

    void setPenColor(const QColor &color);
    void setPenWidth(int width);
    void setBackgroundColor(const QColor &color);

signals:
    void accepted();
    void rejected();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    bool isInResizeHandle(const QPointF &pos) const;
    bool isInAcceptButton(const QPointF &pos) const;
    bool isInRejectButton(const QPointF &pos) const;
    void updateAspectRatioResize(const QPointF &mousePos);

    QRectF _gridSizerRect;
    bool _resizing;
    QPointF _mouseDownPos;
    qreal _gridSize;

    QColor _penColor;
    int _penWidth;
    QColor _backgroundColor;

    QPixmap _acceptPixmap;
    QPixmap _rejectPixmap;
};

#endif // GRIDSIZER_H
