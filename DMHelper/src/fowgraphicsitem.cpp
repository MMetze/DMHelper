#include "fowgraphicsitem.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QElapsedTimer>
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

    static int s_paintCount = 0;
    static QElapsedTimer s_paintInterval;
    static bool s_paintFirst = true;
    qint64 paintIntervalUs = 0;
    if(!s_paintFirst)
        paintIntervalUs = s_paintInterval.nsecsElapsed() / 1000;
    s_paintInterval.restart();
    s_paintFirst = false;

    QElapsedTimer t;
    t.start();

    const QRectF exposedRect = option->exposedRect;
    const QRectF imageBounds = QRectF(_image->rect());
    const QRectF drawRect = exposedRect.intersected(imageBounds);

    if(drawRect.isEmpty())
        return;

    painter->drawImage(drawRect, *_image, drawRect.toAlignedRect());

    ++s_paintCount;
    qDebug() << "[FoW-perf] FowGraphicsItem::paint #" << s_paintCount
             << " interval:" << paintIntervalUs << "us"
             << " drawImage:" << t.nsecsElapsed() / 1000 << "us"
             << " rect:" << drawRect;
}

void FowGraphicsItem::updateRegion(const QRect& region)
{
    QGraphicsItem::update(QRectF(region));
}

void FowGraphicsItem::notifyGeometryChange()
{
    prepareGeometryChange();
}
