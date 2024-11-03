#ifndef BATTLEDIALOGGRAPHICSSCENEMOUSEHANDLERBASE_H
#define BATTLEDIALOGGRAPHICSSCENEMOUSEHANDLERBASE_H

#include <QObject>
#include <QColor>
#include <QPointF>

class BattleDialogGraphicsScene;
class QGraphicsSceneMouseEvent;
class QGraphicsItem;
class QGraphicsLineItem;
class QGraphicsPathItem;
class QGraphicsSimpleTextItem;

class BattleDialogGraphicsSceneMouseHandlerBase : public QObject
{
    Q_OBJECT
public:
    explicit BattleDialogGraphicsSceneMouseHandlerBase(BattleDialogGraphicsScene& scene);
    virtual ~BattleDialogGraphicsSceneMouseHandlerBase();

public:
    virtual bool mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual bool mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual bool mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual bool mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);

protected:
    BattleDialogGraphicsScene& _scene;
};


/******************************************************************************************************/


class BattleDialogGraphicsSceneMouseHandlerDistanceBase : public BattleDialogGraphicsSceneMouseHandlerBase
{
    Q_OBJECT
public:
    explicit BattleDialogGraphicsSceneMouseHandlerDistanceBase(BattleDialogGraphicsScene& scene);
    virtual ~BattleDialogGraphicsSceneMouseHandlerDistanceBase() override;

public:
    virtual void cleanup() = 0;
    virtual void setHeightDelta(qreal heightDelta);
    virtual void setDistanceScale(int scale);
    virtual void setDistanceLineColor(const QColor& color);
    virtual void setDistanceLineType(int lineType);
    virtual void setDistanceLineWidth(int lineWidth);

    virtual QString createDistanceString(qreal lineDistance) const;

    virtual QGraphicsItem* getDistanceLine() const = 0;
    virtual QGraphicsSimpleTextItem* getDistanceText() const = 0;
    virtual void updateDistance() = 0;

signals:
    void distanceChanged(const QString& distance);
    void distanceItemChanged(QGraphicsItem* shapeItem, QGraphicsSimpleTextItem* textItem);

protected:
    qreal _heightDelta;
    int _scale;
    QColor _color;
    int _lineType;
    int _lineWidth;
};


/******************************************************************************************************/


class BattleDialogGraphicsSceneMouseHandlerDistance : public BattleDialogGraphicsSceneMouseHandlerDistanceBase
{
    Q_OBJECT
public:
    explicit BattleDialogGraphicsSceneMouseHandlerDistance(BattleDialogGraphicsScene& scene);
    virtual ~BattleDialogGraphicsSceneMouseHandlerDistance() override;

public:
    virtual void cleanup() override;
    virtual bool mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual bool mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual bool mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual QGraphicsItem* getDistanceLine() const override;
    virtual QGraphicsSimpleTextItem* getDistanceText() const override;
    virtual void updateDistance() override;

protected:
    QGraphicsLineItem* _distanceLine;
    QGraphicsSimpleTextItem* _distanceText;

};


/******************************************************************************************************/


class BattleDialogGraphicsSceneMouseHandlerFreeDistance : public BattleDialogGraphicsSceneMouseHandlerDistanceBase
{
    Q_OBJECT
public:
    explicit BattleDialogGraphicsSceneMouseHandlerFreeDistance(BattleDialogGraphicsScene& scene);
    virtual ~BattleDialogGraphicsSceneMouseHandlerFreeDistance() override;

public:
    virtual void cleanup() override;
    virtual bool mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual bool mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual bool mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual QGraphicsItem* getDistanceLine() const override;
    virtual QGraphicsSimpleTextItem* getDistanceText() const override;
    virtual void updateDistance() override;

protected:
    QPointF _mouseDownPos;
    QGraphicsPathItem* _distancePath;
    QGraphicsSimpleTextItem* _distanceText;

};


/******************************************************************************************************/


class BattleDialogGraphicsSceneMouseHandlerPointer : public BattleDialogGraphicsSceneMouseHandlerBase
{
    Q_OBJECT
public:
    explicit BattleDialogGraphicsSceneMouseHandlerPointer(BattleDialogGraphicsScene& scene);
    virtual ~BattleDialogGraphicsSceneMouseHandlerPointer() override;

public:
    virtual bool mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual bool mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual bool mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;

signals:
    void pointerMoved(const QPointF& pos);
};


/******************************************************************************************************/


class BattleDialogGraphicsSceneMouseHandlerRaw : public BattleDialogGraphicsSceneMouseHandlerBase
{
    Q_OBJECT
public:
    explicit BattleDialogGraphicsSceneMouseHandlerRaw(BattleDialogGraphicsScene& scene);
    virtual ~BattleDialogGraphicsSceneMouseHandlerRaw() override;

public:
    virtual bool mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual bool mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual bool mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;

signals:
    void rawMousePress(const QPointF& pos);
    void rawMouseMove(const QPointF& pos);
    void rawMouseRelease(const QPointF& pos);
};


/******************************************************************************************************/


class BattleDialogGraphicsSceneMouseHandlerCamera : public BattleDialogGraphicsSceneMouseHandlerBase
{
    Q_OBJECT
public:
    explicit BattleDialogGraphicsSceneMouseHandlerCamera(BattleDialogGraphicsScene& scene);
    virtual ~BattleDialogGraphicsSceneMouseHandlerCamera() override;

public:
    virtual bool mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual bool mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual bool mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;

protected:
    virtual bool checkMouseEvent(QGraphicsSceneMouseEvent *mouseEvent);
};


/******************************************************************************************************/


class BattleDialogGraphicsSceneMouseHandlerCombatants : public BattleDialogGraphicsSceneMouseHandlerBase
{
    Q_OBJECT
public:
    explicit BattleDialogGraphicsSceneMouseHandlerCombatants(BattleDialogGraphicsScene& scene);
    virtual ~BattleDialogGraphicsSceneMouseHandlerCombatants() override;

public:
    virtual bool mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual bool mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual bool mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual bool mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
};


/******************************************************************************************************/


class BattleDialogGraphicsSceneMouseHandlerMaps : public BattleDialogGraphicsSceneMouseHandlerBase
{
    Q_OBJECT
public:
    explicit BattleDialogGraphicsSceneMouseHandlerMaps(BattleDialogGraphicsScene& scene);
    virtual ~BattleDialogGraphicsSceneMouseHandlerMaps() override;

public:
    virtual bool mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual bool mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual bool mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;

signals:
    void mapMousePress(const QPointF& pos);
    void mapMouseMove(const QPointF& pos);
    void mapMouseRelease(const QPointF& pos);
};

#endif // BATTLEDIALOGGRAPHICSSCENEMOUSEHANDLERBASE_H
