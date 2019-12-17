#ifndef BATTLEDIALOGGRAPHICSSCENE_H
#define BATTLEDIALOGGRAPHICSSCENE_H

#include <QGraphicsScene>

class BattleDialogModel;
class BattleDialogModelEffect;
class Grid;
class QAbstractGraphicsShapeItem;

class BattleDialogGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit BattleDialogGraphicsScene(BattleDialogModel& model, QObject *parent = nullptr);
    virtual ~BattleDialogGraphicsScene();

    void createBattleContents(const QRect& rect);
    void resizeBattleContents(const QRect& rect);
    void updateBattleContents();
    void clearBattleContents();
    void setEffectVisibility(bool visible);
    void setGridVisibility(bool visible);

    QList<QGraphicsItem*> getEffectItems() const;

public slots:
    void setShowDistance(bool showDistance, qreal heightDelta);

signals:
    void effectChanged(QAbstractGraphicsShapeItem* effect);
    void applyEffect(QAbstractGraphicsShapeItem* effect);
    void distanceChanged(const QString& distance);

    void itemMouseDown(QGraphicsPixmapItem* item);
    void itemMoved(QGraphicsPixmapItem* item, bool* result);
    void itemMouseUp(QGraphicsPixmapItem* item);

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

    QGraphicsItem* findTopObject(const QPointF &pos);

    QGraphicsItem* _contextMenuItem;
    Grid* _grid;
    BattleDialogModel& _model;
    QList<QGraphicsItem*> _itemList;

    bool _mouseDown;
    QPointF _mouseDownPos;
    QGraphicsItem* _mouseDownItem;
    qreal _previousRotation;

    bool _distanceShown;
    qreal _heightDelta;
    QGraphicsLineItem* _distanceLine;
    QGraphicsSimpleTextItem* _distanceText;

};

#endif // BATTLEDIALOGGRAPHICSSCENE_H
