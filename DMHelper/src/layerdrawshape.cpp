#include "layerdrawshape.h"
#include "layerdrawobject.h"

// --- LayerDrawShapePath ---

LayerDrawShapePath::LayerDrawShapePath(const QPainterPath &path, LayerDrawObjectPath* pathObject) :
    QGraphicsPathItem(path),
    _object(pathObject)
{
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

QVariant LayerDrawShapePath::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if((change == ItemScenePositionHasChanged) && (_object))
    {
        QPointF newPos = value.toPointF();
        _object->setPosition(newPos);
    }

    return QGraphicsItem::itemChange(change, value);
}

// --- LayerDrawShapeLine ---

LayerDrawShapeLine::LayerDrawShapeLine(const QLineF &line, LayerDrawObjectLine* lineObject) :
    QGraphicsLineItem(line),
    _object(lineObject)
{
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

QVariant LayerDrawShapeLine::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if((change == ItemScenePositionHasChanged) && (_object))
    {
        QPointF newPos = value.toPointF();
        _object->setPosition(newPos);
    }

    return QGraphicsItem::itemChange(change, value);
}

// --- LayerDrawShapeRect ---

LayerDrawShapeRect::LayerDrawShapeRect(const QRectF &rect, LayerDrawObjectRect* rectObject) :
    QGraphicsRectItem(rect),
    _object(rectObject)
{
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

QVariant LayerDrawShapeRect::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if((change == ItemScenePositionHasChanged) && (_object))
    {
        QPointF newPos = value.toPointF();
        _object->setPosition(newPos);
    }

    return QGraphicsItem::itemChange(change, value);
}

// --- LayerDrawShapeEllipse ---

LayerDrawShapeEllipse::LayerDrawShapeEllipse(const QRectF &rect, LayerDrawObjectEllipse* ellipseObject) :
    QGraphicsEllipseItem(rect),
    _object(ellipseObject)
{
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

QVariant LayerDrawShapeEllipse::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if((change == ItemScenePositionHasChanged) && (_object))
    {
        QPointF newPos = value.toPointF();
        _object->setPosition(newPos);
    }

    return QGraphicsItem::itemChange(change, value);
}

// --- LayerDrawShapeText ---

LayerDrawShapeText::LayerDrawShapeText(const QString &text, LayerDrawObjectText* textObject) :
    QGraphicsSimpleTextItem(text),
    _object(textObject)
{
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

QVariant LayerDrawShapeText::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if((change == ItemScenePositionHasChanged) && (_object))
    {
        QPointF newPos = value.toPointF();
        _object->setPosition(newPos);
    }

    return QGraphicsItem::itemChange(change, value);
}
