#ifndef LAYERDRAWENGINE_H
#define LAYERDRAWENGINE_H

#include "dmconstants.h"
#include <QObject>
#include <QPointF>
#include <QCursor>

class LayerDraw;
class LayerDrawToolDialog;
class LayerDrawObject;
class QGraphicsItem;
class QGraphicsScene;
class QKeyEvent;

class LayerDrawEngine : public QObject
{
    Q_OBJECT
public:
    explicit LayerDrawEngine(QObject *parent = nullptr);

public slots:
    void setDrawLayer(LayerDraw* drawLayer);
    void setActive(bool active);

    void handleMouseDown(const QPointF& pos, const Qt::MouseButtons buttons, const Qt::KeyboardModifiers modifiers);
    void handleMouseMoved(const QPointF& pos, const Qt::MouseButtons buttons, const Qt::KeyboardModifiers modifiers);
    void handleMouseUp(const QPointF& pos, const Qt::MouseButtons buttons, const Qt::KeyboardModifiers modifiers);
    bool handleKeyPress(QKeyEvent* event);

signals:
    void cursorChanged(const QCursor& cursor);

private:
    void handlePathMouseDown(const QPointF& pos);
    void handlePathMouseMoved(const QPointF& pos);
    void handleLineMouseDown(const QPointF& pos);
    void handleLineMouseMoved(const QPointF& pos);
    void handleRectMouseDown(const QPointF& pos);
    void handleRectMouseMoved(const QPointF& pos);
    void handleEllipseMouseDown(const QPointF& pos);
    void handleEllipseMouseMoved(const QPointF& pos);
    void handleTextMouseDown(const QPointF& pos);
    void handleEraserMouseDown(const QPointF& pos);
    void handleToolTypeChanged(DMHelper::DrawToolType toolType);
    void commitActiveObject();
    void discardActiveObject();

    LayerDraw* _drawLayer;
    LayerDrawToolDialog* _toolDialog;

    QPointF _mouseDownPos;
    DMHelper::DrawToolType _activeTool;
    LayerDrawObject* _activeDrawObject;
    QGraphicsItem* _activeItem;
};

#endif // LAYERDRAWENGINE_H
