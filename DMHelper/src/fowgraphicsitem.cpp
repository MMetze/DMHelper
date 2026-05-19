#include "fowgraphicsitem.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDebug>

FowGraphicsItem::FowGraphicsItem(const QImage* image, QGraphicsItem* parent)
    : QGraphicsItem(parent),
      _image(image)
{
    // ItemUsesExtendedStyleOption is required for option->exposedRect to be populated
    // in paint(). Without this flag the exposed rect is always the full bounding rect,
    // defeating the purpose of rect-scoped repainting.
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
}

QRectF FowGraphicsItem::boundingRect() const
{
    return (_image && !_image->isNull()) ? QRectF(_image->rect()) : QRectF();
}

void FowGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);
    if(!_image || _image->isNull())
        return;

    const QRectF exposedRect = option->exposedRect;
    const QRectF imageBounds = QRectF(_image->rect());
    const QRectF drawRect = exposedRect.intersected(imageBounds);

    if(drawRect.isEmpty())
        return;

    painter->drawImage(drawRect, *_image, drawRect.toAlignedRect());
}

void FowGraphicsItem::updateRegion(const QRect& region)
{
    // Guard: QRectF(QRect()) is null, and Qt treats a null rect in update() as
    // "repaint full bounding rect". Only forward non-empty regions.
    if(!region.isEmpty())
        QGraphicsItem::update(QRectF(region));
    #ifdef QT_DEBUG
    else
        qDebug() << "[FowGraphicsItem] updateRegion called with empty region — ignoring to avoid full repaint";
    #endif
}

void FowGraphicsItem::notifyGeometryChange()
{
    prepareGeometryChange();
}
