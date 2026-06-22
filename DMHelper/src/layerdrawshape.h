#ifndef LAYERDRAWSHAPE_H
#define LAYERDRAWSHAPE_H

#include <QGraphicsPathItem>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsSimpleTextItem>

class LayerDrawObjectPath;
class LayerDrawObjectLine;
class LayerDrawObjectRect;
class LayerDrawObjectEllipse;
class LayerDrawObjectText;

class LayerDrawShapePath : public QGraphicsPathItem
{
public:
    explicit LayerDrawShapePath(const QPainterPath &path, LayerDrawObjectPath* pathObject);

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    LayerDrawObjectPath* _object;
};

class LayerDrawShapeLine : public QGraphicsLineItem
{
public:
    explicit LayerDrawShapeLine(const QLineF &line, LayerDrawObjectLine* lineObject);

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    LayerDrawObjectLine* _object;
};

class LayerDrawShapeRect : public QGraphicsRectItem
{
public:
    explicit LayerDrawShapeRect(const QRectF &rect, LayerDrawObjectRect* rectObject);

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    LayerDrawObjectRect* _object;
};

class LayerDrawShapeEllipse : public QGraphicsEllipseItem
{
public:
    explicit LayerDrawShapeEllipse(const QRectF &rect, LayerDrawObjectEllipse* ellipseObject);

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    LayerDrawObjectEllipse* _object;
};

class LayerDrawShapeText : public QGraphicsSimpleTextItem
{
public:
    explicit LayerDrawShapeText(const QString &text, LayerDrawObjectText* textObject);

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    LayerDrawObjectText* _object;
};

#endif // LAYERDRAWSHAPE_H
