#ifndef GRIDSIZER_H
#define GRIDSIZER_H

#include <QGraphicsItem>

class GridSizer : public QGraphicsItem
{
public:
    GridSizer(qreal size = 250.0, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

    qreal getSize() const;
    qreal getHandleSize() const;

    void setSize(qreal size);

    void setPenColor(const QColor &color);
    void setPenWidth(int width);
    void setBackgroundColor(const QColor &color);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    bool isInResizeHandle(const QPointF &pos) const;
    void updateAspectRatioResize(const QPointF &mousePos);

    QRectF _gridSizerRect;
    bool _resizing;
    QPointF _mouseDownPos;
    qreal _gridSize;

    QColor _penColor;
    int _penWidth;
    QColor _backgroundColor;
};

#endif // GRIDSIZER_H
