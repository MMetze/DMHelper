#ifndef BATTLEDIALOGGRAPHICSSCENEMOUSEHANDLERBASE_H
#define BATTLEDIALOGGRAPHICSSCENEMOUSEHANDLERBASE_H

#include <QObject>

class BattleDialogGraphicsScene;
class QGraphicsSceneMouseEvent;
class QGraphicsLineItem;
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


class BattleDialogGraphicsSceneMouseHandlerDistance : public BattleDialogGraphicsSceneMouseHandlerBase
{
    Q_OBJECT
public:
    explicit BattleDialogGraphicsSceneMouseHandlerDistance(BattleDialogGraphicsScene& scene);
    virtual ~BattleDialogGraphicsSceneMouseHandlerDistance() override;

public:
    virtual bool mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual bool mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual bool mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;

    void setHeightDelta(qreal heightDelta);

signals:
    void distanceChanged(const QString& distance);

protected:
    qreal _heightDelta;
    QGraphicsLineItem* _distanceLine;
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
