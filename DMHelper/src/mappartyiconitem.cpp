#include "mappartyiconitem.h"

MapPartyIconItem::MapPartyIconItem(QObject* parent)
    : QObject(parent),
      QGraphicsPixmapItem(nullptr)
{
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

QVariant MapPartyIconItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if(change == ItemScenePositionHasChanged)
        emit positionChanged(value.toPointF());

    return QGraphicsPixmapItem::itemChange(change, value);
}
