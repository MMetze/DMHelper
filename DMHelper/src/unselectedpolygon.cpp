#include "unselectedpolygon.h"
#include "battledialogmodelobject.h"
#include "battledialogmodel.h"
#include "battleframe.h"
#include "layertokens.h"
#include "layergrid.h"
#include "dmconstants.h"
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QPen>

#ifdef DEBUG_FILL_BOUNDING_RECTS
    #include <QPainter>
#endif

UnselectedPolygon::UnselectedPolygon(BattleDialogModelObject* object, const QPolygonF &polygon, QGraphicsItem *parent) :
    QGraphicsPolygonItem(polygon, parent),
    _object(object)
{
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

void UnselectedPolygon::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
#ifdef SHOW_UNSELECTED_SHAPE_SELECTION
    QGraphicsPolygonItem::paint(painter, option, widget);
#else
    QStyleOptionGraphicsItem myoption = (*option);
    myoption.state &= ~QStyle::State_Selected;
    QGraphicsPolygonItem::paint(painter, &myoption, widget);
#endif

#ifdef DEBUG_FILL_BOUNDING_RECTS
        if(painter)
            painter->fillRect(boundingRect(), Qt::blue);
#endif
}

QVariant UnselectedPolygon::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if((change == ItemPositionChange) && (scene()))
    {
        scene()->update(mapRectToScene(boundingRect() | childrenBoundingRect()));

        // Ensure the new position is within the scene rect
        QPointF newPos = mapToScene(mapFromParent(value.toPointF()));

        // Check if snap to grid is activated for this layer
        if((_object) && (_object->getLayer()))
        {
            LayerTokens* tokenLayer = _object->getLayer();
            BattleDialogGraphicsScene* battleScene = dynamic_cast<BattleDialogGraphicsScene*>(scene());
            if((battleScene) && (battleScene->getModel()))
            {
                LayerGrid* gridLayer = dynamic_cast<LayerGrid*>(battleScene->getModel()->getLayerScene().getNearest(tokenLayer, DMHelper::LayerType_Grid));
                if((gridLayer) && (gridLayer->getConfig().isSnapToGrid()))
                {
                    // Snap the current position to the grid
                    QPointF offset = gridLayer->getConfig().getGridOffset() * gridLayer->getConfig().getGridScale() / 100.0;
                    newPos -= offset;
                    int intGridSize = static_cast<int>(tokenLayer->getScale());
                    newPos.setX((static_cast<qreal>(static_cast<int>(newPos.x()) / (intGridSize / 2)) * (tokenLayer->getScale() / 2.0)) + (tokenLayer->getScale() / 2.0));
                    newPos.setY((static_cast<qreal>(static_cast<int>(newPos.y()) / (intGridSize / 2)) * (tokenLayer->getScale() / 2.0)) + (tokenLayer->getScale() / 2.0));
                    newPos += offset;
                    return mapToParent(mapFromScene(newPos));
                }
            }
        }
    }
    else if(change == ItemSelectedHasChanged)
    {
        QPen itemPen = pen();
        itemPen.setWidth(isSelected() ? 3 : 1);
        setPen(itemPen);
    }
    else if(change == ItemScenePositionHasChanged)
    {
        if(_object)
        {
            QPointF newPos = value.toPointF();
            _object->setPosition(newPos);
        }
    }

    return QGraphicsItem::itemChange(change, value);
}
