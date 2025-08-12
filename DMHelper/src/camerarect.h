#ifndef CAMERARECT_H
#define CAMERARECT_H

#include <QGraphicsRectItem>

class CameraRect : public QGraphicsRectItem
{
public:
    CameraRect(qreal width, qreal height, QGraphicsScene& scene, QWidget* viewport, bool ratioLocked = false);
    CameraRect(const QRectF& rect, QGraphicsScene& scene, QWidget* viewport, bool ratioLocked = false);
    virtual ~CameraRect();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);
    QRectF getCameraRect() const;
    void setCameraRect(const QRectF& rect);
    void setCameraSelectable(bool selectable);
    void setDraw(bool draw);
    void setPublishing(bool publishing);

    void setRatioLocked(bool locked);
    void setSizeLocked(bool locked);

protected:
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void initialize(QGraphicsScene& scene);

    int getRectSection(const QPointF point);
    void resizeRectangle(QGraphicsSceneMouseEvent& event, qreal& dx, qreal& dy, qreal& w, qreal& h);
    void resizeRectangleFixed(QGraphicsSceneMouseEvent& event, qreal& dx, qreal& dy, qreal& w, qreal& h);

private:
    void setCameraRectColor();

    bool _draw;
    bool _mouseDown;
    QPointF _mouseDownPos;
    QPointF _mouseLastPos;
    int _mouseDownSection;
    QRectF _trackingRect;

    QGraphicsRectItem* _drawItem;
    QGraphicsSimpleTextItem* _drawText;
    QGraphicsRectItem* _drawTextRect;
    QGraphicsRectItem* _cameraIconRect;
    QGraphicsPixmapItem* _cameraIcon;


    bool _publishing;
    bool _ratioLocked;
    bool _sizeLocked;
    QWidget* _viewport;

    enum RectSection
    {
        RectSection_None        = 0x0000,
        RectSection_Top         = 0x0001,
        RectSection_Bottom      = 0x0002,
        RectSection_Left        = 0x0004,
        RectSection_Right       = 0x0008,
        RectSection_TopLeft     = RectSection_Top | RectSection_Left,
        RectSection_TopRight    = RectSection_Top | RectSection_Right,
        RectSection_BottomLeft  = RectSection_Bottom | RectSection_Left,
        RectSection_BottomRight = RectSection_Bottom | RectSection_Right,
        RectSection_Middle      = 0x0010
    };

    // Add this class definition somewhere in your project (e.g., camerarect.h or a new file)
    class FixedBorderRectItem : public QGraphicsRectItem
    {
    public:
        FixedBorderRectItem(QGraphicsItem* parent = nullptr);

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    };
};

#endif // CAMERARECT_H
