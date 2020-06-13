#include "battledialogmodeleffectline.h"
#include "battledialogeffectsettings.h"
#include <unselectedrect.h>
#include <QDebug>
#include <QDomElement>
#include <QPen>

BattleDialogModelEffectLine::BattleDialogModelEffectLine(const QString& name, QObject *parent) :
    BattleDialogModelEffectShape(name, parent),
    _width(5)
{
}

BattleDialogModelEffectLine::BattleDialogModelEffectLine(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip) :
    BattleDialogModelEffectShape(size, position, rotation, color, tip),
    _width(5)
{
}

BattleDialogModelEffectLine::~BattleDialogModelEffectLine()
{
}

void BattleDialogModelEffectLine::inputXML(const QDomElement &element, bool isImport)
{
    _width = element.attribute("width", QString::number(5)).toInt();

    BattleDialogModelEffectShape::inputXML(element, isImport);
}

BattleDialogModelEffect* BattleDialogModelEffectLine::clone() const
{
    BattleDialogModelEffectLine* newEffect = new BattleDialogModelEffectLine(getName());
    newEffect->copyValues(*this);
    return newEffect;
}

int BattleDialogModelEffectLine::getEffectType() const
{
    return BattleDialogModelEffect_Line;
}

BattleDialogEffectSettings* BattleDialogModelEffectLine::getEffectEditor() const
{
    BattleDialogEffectSettings* result = new BattleDialogEffectSettings(*this);
    result->setSizeLabel(QString("Length"));
    result->setShowWidth(true);
    return result;
}

QGraphicsItem* BattleDialogModelEffectLine::createEffectShape(qreal gridScale) const
{
    qreal effectWidth = static_cast<qreal>(getWidth());
    qreal effectHeight = static_cast<qreal>(getSize());
    qreal rectWidth = (effectHeight > 0.1) ? 100.0 * effectWidth / effectHeight : 100.0 * effectWidth;
    QGraphicsRectItem* rectItem = new UnselectedRect(-rectWidth / 2.0, 0.0, rectWidth, 100.0);

    setEffectItemData(rectItem);

    prepareItem(*rectItem);
    applyEffectValues(*rectItem, gridScale);

    return rectItem;
}

void BattleDialogModelEffectLine::applyEffectValues(QGraphicsItem& item, qreal gridScale) const
{
    // First apply the base information
    qDebug() << "[Battle Dialog Model Effect] applying effect values for " << this << " to " << &item;
    item.setPos(getPosition());
    item.setRotation(getRotation());
    item.setToolTip(getTip());

    qDebug() << "[Battle Dialog Model Effect Line] applying extra line effect values...";

    // Now correct the special case information for the line effect
    qreal effectWidth = static_cast<qreal>(getWidth());
    qreal effectHeight = static_cast<qreal>(getSize());
    qreal rectWidth = (effectHeight > 0.1) ? 100.0 * effectWidth / effectHeight : 100.0 * effectWidth;
    QGraphicsRectItem* rectItem = dynamic_cast<QGraphicsRectItem*>(&item);
    if(rectItem)
    {
        rectItem->setRect(QRectF(-rectWidth / 2.0, 0.0, rectWidth, 100.0));
    }
    else
    {
        qDebug() << "[Battle Dialog Model Effect Line] ERROR: Line Effect found without QGraphicsRectItem!";
    }

    //item.setScale(gridScale / 500.0);
    item.setScale(effectHeight * gridScale / 500.0);

    QAbstractGraphicsShapeItem* shapeItem = dynamic_cast<QAbstractGraphicsShapeItem*>(&item);
    if(shapeItem)
    {
        shapeItem->setPen(QPen(getColor(), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        shapeItem->setBrush(QBrush(QColor(getColor().red(),getColor().green(),getColor().blue(),getColor().alpha())));
    }
}

/*
 *     // First apply the base information
    BattleDialogModelEffectShape::applyEffectValues(item, gridScale);
    qDebug() << "[Battle Dialog Model Effect Line] applying extra line effect values...";

    // Now correct the special case information for the line effect
    QGraphicsRectItem* rectItem = dynamic_cast<QGraphicsRectItem*>(&item);
    if(rectItem)
    {
        qreal rectWidth = static_cast<qreal>(getWidth()) * 100.0;
        rectItem->setRect(QRectF(-rectWidth / 2.0, 0.0, rectWidth, static_cast<qreal>(getSize()) * 100.0));
    }
    else
    {
        qDebug() << "[Battle Dialog Model Effect Line] ERROR: Line Effect found without QGraphicsRectItem!";
    }

    item.setScale(gridScale / 500.0);
    QAbstractGraphicsShapeItem* shapeItem = dynamic_cast<QAbstractGraphicsShapeItem*>(&item);
    if(shapeItem)
        shapeItem->setPen(QPen(getColor(), 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
*/

int BattleDialogModelEffectLine::getWidth() const
{
    return _width;
}

void BattleDialogModelEffectLine::setWidth(int width)
{
    _width = width;
}

void BattleDialogModelEffectLine::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("width", getWidth());

    BattleDialogModelEffectShape::internalOutputXML(doc, element, targetDirectory, isExport);
}
