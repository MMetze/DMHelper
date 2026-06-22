#ifndef MAPPARTYICONITEM_H
#define MAPPARTYICONITEM_H

#include <QObject>
#include <QGraphicsPixmapItem>

class MapPartyIconItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit MapPartyIconItem(QObject* parent = nullptr);

signals:
    void positionChanged(const QPointF& pos);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};

#endif // MAPPARTYICONITEM_H
