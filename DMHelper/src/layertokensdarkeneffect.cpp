#include "layertokensdarkeneffect.h"
#include <QPainter>

LayerTokensDarkenEffect::LayerTokensDarkenEffect(QObject *parent) :
    QGraphicsEffect(parent)
{
}

QRectF LayerTokensDarkenEffect::boundingRectFor(const QRectF &sourceRect) const
{
    return sourceRect;
}

void LayerTokensDarkenEffect::draw(QPainter *painter)
{
    QPoint offset;
    QPixmap pixmap = sourcePixmap(Qt::DeviceCoordinates, &offset);
    if (pixmap.isNull())
        return;

    // Paint the dark overlay only onto opaque pixels of the source
    QPainter overlayPainter(&pixmap);
    overlayPainter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    overlayPainter.fillRect(pixmap.rect(), QColor(0, 0, 0, 200));
    overlayPainter.end();

    // Draw in device coords so the dirty region matches exactly
    QTransform restoreTransform = painter->worldTransform();
    painter->setWorldTransform(QTransform());
    painter->drawPixmap(offset, pixmap);
    painter->setWorldTransform(restoreTransform);
}
