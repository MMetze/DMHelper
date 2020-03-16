#ifndef BATTLEDIALOGGRAPHICSSCENE_H
#define BATTLEDIALOGGRAPHICSSCENE_H

#include <QGraphicsScene>
#include "battledialoggraphicsscenemousehandler.h"

class BattleDialogModel;
class BattleDialogModelEffect;
class Grid;
class QAbstractGraphicsShapeItem;

class BattleDialogGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit BattleDialogGraphicsScene(QObject *parent = nullptr);
    virtual ~BattleDialogGraphicsScene();

    void setModel(BattleDialogModel* model);
    BattleDialogModel* getModel() const;

    void createBattleContents(const QRect& rect);
    void resizeBattleContents(const QRect& rect);
    void updateBattleContents();
    void scaleBattleContents();
    void clearBattleContents();
    void setEffectVisibility(bool visible);
    void setGridVisibility(bool visible);

    void setPointerVisibility(bool visible);
    void setPointerPos(const QPointF& pos);

    QList<QGraphicsItem*> getEffectItems() const;

    bool isSceneEmpty() const;
    void handleItemChanged(QGraphicsItem* item);

    QGraphicsItem* findTopObject(const QPointF &pos);

    bool handleMouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent);
    bool handleMouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    bool handleMousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    bool handleMouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);

public slots:
    void setDistanceHeight(qreal heightDelta);
    //void setRawMouse(bool rawMouse);
    void setInputMode(int inputMode);

signals:
    void effectChanged(QAbstractGraphicsShapeItem* effect);
    void applyEffect(QAbstractGraphicsShapeItem* effect);
    void distanceChanged(const QString& distance);

    void battleMousePress(const QPointF& pos);
    void battleMouseMove(const QPointF& pos);
    void battleMouseRelease(const QPointF& pos);

    void itemMouseDown(QGraphicsPixmapItem* item);
    void itemMoved(QGraphicsPixmapItem* item, bool* result);
    void itemMouseUp(QGraphicsPixmapItem* item);
    void itemChanged(QGraphicsItem* item);

protected slots:
    void editItem();
    void rollItem();
    void deleteItem();

    void addEffectRadius();
    void addEffectCone();
    void addEffectCube();
    void addEffectLine();

protected:
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);

    void addEffect(BattleDialogModelEffect* effect);
    QAbstractGraphicsShapeItem* addEffectShape(BattleDialogModelEffect& effect);

    BattleDialogGraphicsSceneMouseHandlerBase* getMouseHandler();

    QGraphicsItem* _contextMenuItem;
    Grid* _grid;
    BattleDialogModel* _model;
    QList<QGraphicsItem*> _itemList;

    bool _mouseDown;
    QPointF _mouseDownPos;
    QGraphicsItem* _mouseDownItem;
    qreal _previousRotation;

    //bool _rawMouse;
    int _inputMode;

    QGraphicsPixmapItem* _pointerPixmap;
    bool _pointerVisible;

    BattleDialogGraphicsSceneMouseHandlerDistance _distanceMouseHandler;
    BattleDialogGraphicsSceneMouseHandlerPointer _pointerMouseHandler;
    BattleDialogGraphicsSceneMouseHandlerRaw _rawMouseHandler;
    BattleDialogGraphicsSceneMouseHandlerCamera _cameraMouseHandler;
    BattleDialogGraphicsSceneMouseHandlerCombatants _combatantMouseHandler;
    //BattleDialogGraphicsSceneMouseHandlerSelect _selectMouseHandler;
};

#endif // BATTLEDIALOGGRAPHICSSCENE_H
