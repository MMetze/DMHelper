#ifndef FOWGRAPHICSITEM_H
#define FOWGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QImage>

// FowGraphicsItem — a lightweight QGraphicsItem that renders the composed FOW image
// using only the region Qt requests (option->exposedRect). The image is owned by
// LayerFow (_cachedImage member) and must outlive this item.
//
// Caller responsibilities:
//   - Call updateRegion(region) whenever the image content changes in a sub-rect.
//   - Never cache this item's rendering (DeviceCoordinateCache must not be set —
//     the contents mutate during brush strokes).
class FowGraphicsItem : public QGraphicsItem
{
public:
    // image: pointer to LayerFow::_cachedImage. Lifetime is owned by LayerFow —
    // this item must not outlive its owner.
    explicit FowGraphicsItem(const QImage* image, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    // Calls QGraphicsItem::update(QRectF(region)) so the scene repaints exactly
    // the changed sub-rect. Callers do not need to convert the type.
    void updateRegion(const QRect& region);

    // Must be called before externally changing the value returned by boundingRect().
    // Wraps QGraphicsItem::prepareGeometryChange(), which is protected.
    void notifyGeometryChange();

private:
    // Pointer to LayerFow::_cachedImage. Not owned. Stable for the item's lifetime.
    const QImage* _image;
};

#endif // FOWGRAPHICSITEM_H
