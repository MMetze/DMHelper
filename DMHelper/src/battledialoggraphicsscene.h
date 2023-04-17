#ifndef BATTLEDIALOGGRAPHICSSCENE_H
#define BATTLEDIALOGGRAPHICSSCENE_H

#include <QPixmap>
#include "camerascene.h"
#include "battledialoggraphicsscenemousehandler.h"

class BattleDialogModel;
class BattleDialogModelObject;
class BattleDialogModelEffect;
class BattleDialogModelCombatant;
class Grid;
class QAbstractGraphicsShapeItem;

class BattleDialogGraphicsScene : public CameraScene
{
    Q_OBJECT

public:
    explicit BattleDialogGraphicsScene(QObject *parent = nullptr);
    virtual ~BattleDialogGraphicsScene();

    void setModel(BattleDialogModel* model);
    BattleDialogModel* getModel() const;

    void createBattleContents();
    void resizeBattleContents();
    void updateBattleContents();
    void scaleBattleContents();
    void clearBattleContents();
    void setEffectVisibility(bool visible, bool allEffects = true);
    void setGridVisibility(bool visible);
    void paintGrid(QPainter* painter);

    void setPointerVisibility(bool visible);
    void setPointerPos(const QPointF& pos);
    void setPointerPixmap(QPixmap pixmap);

    QPixmap getSelectedIcon() const;
    QString getSelectedIconFile() const;
    QPointF getCommandPosition() const;

    QList<QGraphicsItem*> getEffectItems() const;

    bool isSceneEmpty() const;

    QGraphicsItem* findTopObject(const QPointF &pos);

    bool handleMouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent);
    bool handleMouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    bool handleMousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    bool handleMouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);

    QPointF getViewportCenter();

public slots:
    void setDistanceHeight(qreal heightDelta);
    void setDistanceScale(int scale);
    void setDistanceLineColor(const QColor& color);
    void setDistanceLineType(int lineType);
    void setDistanceLineWidth(int lineWidth);
    void setInputMode(int inputMode);

    void castSpell();

    void setSelectedIcon(const QString& selectedIcon);

signals:
    void addEffectRadius();
    void addEffectCone();
    void addEffectCube();
    void addEffectLine();
    void addEffectObject();

    void effectChanged(QGraphicsItem* effect);
    void effectRemoved(QGraphicsItem* effect);
    void applyEffect(QGraphicsItem* effect);
    void distanceChanged(const QString& distance);
    void distanceItemChanged(QGraphicsItem* shapeItem, QGraphicsSimpleTextItem* textItem);

    void pointerMove(const QPointF& pos);

    void battleMousePress(const QPointF& pos);
    void battleMouseMove(const QPointF& pos);
    void battleMouseRelease(const QPointF& pos);

    void mapMousePress(const QPointF& pos);
    void mapMouseMove(const QPointF& pos);
    void mapMouseRelease(const QPointF& pos);

    void mapZoom(int zoomFactor);

    void itemMouseDown(QGraphicsPixmapItem* item);
    void itemMoved(QGraphicsPixmapItem* item, bool* result);
    void itemMouseUp(QGraphicsPixmapItem* item);
    void itemMouseDoubleClick(QGraphicsPixmapItem* item);

    void itemLink(BattleDialogModelObject* item);
    void itemUnlink(BattleDialogModelObject* item);

    void combatantActivate(BattleDialogModelCombatant* combatant);
    void combatantRemove(BattleDialogModelCombatant* combatant);
    void combatantChangeLayer(BattleDialogModelCombatant* combatant);
    void combatantDamage(BattleDialogModelCombatant* combatant);
    void combatantHeal(BattleDialogModelCombatant* combatant);

    void combatantHover(BattleDialogModelCombatant* combatant, bool hover);

protected slots:
    void editItem();
    void rollItem();
    void deleteItem();

    void linkItem();
    void unlinkItem();

    void activateCombatant();
    void removeCombatant();
    void changeCombatantLayer();
    void damageCombatant();
    void healCombatant();

protected:
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);
    virtual void wheelEvent(QGraphicsSceneWheelEvent *wheelEvent);
    virtual void keyPressEvent(QKeyEvent *keyEvent);
    virtual void keyReleaseEvent(QKeyEvent *keyEvent);

    BattleDialogModelEffect* createEffect(int type, int size, int width, const QColor& color, const QString& filename);
//    QGraphicsItem* addEffect(BattleDialogModelEffect* effect);
//    QGraphicsItem* addEffectShape(BattleDialogModelEffect& effect);
//    QGraphicsItem* addSpellEffect(BattleDialogModelEffect& effect);

    BattleDialogGraphicsSceneMouseHandlerBase* getMouseHandler(QGraphicsSceneMouseEvent *mouseEvent);

    QGraphicsItem* _contextMenuItem;
    Grid* _grid;
    BattleDialogModel* _model;
    QList<QGraphicsItem*> _itemList;

    bool _mouseDown;
    QPointF _mouseDownPos;
    QGraphicsItem* _mouseDownItem;
    BattleDialogModelCombatant* _mouseHoverItem;
    qreal _previousRotation;
    bool _isRotation;
    QPointF _commandPosition;

    bool _spaceDown;
    int _inputMode;

    QGraphicsPixmapItem* _pointerPixmapItem;
    bool _pointerVisible;
    QPixmap _pointerPixmap;

    QString _selectedIcon;

    BattleDialogGraphicsSceneMouseHandlerDistance _distanceMouseHandler;
    BattleDialogGraphicsSceneMouseHandlerFreeDistance _freeDistanceMouseHandler;
    BattleDialogGraphicsSceneMouseHandlerPointer _pointerMouseHandler;
    BattleDialogGraphicsSceneMouseHandlerRaw _rawMouseHandler;
    BattleDialogGraphicsSceneMouseHandlerCamera _cameraMouseHandler;
    BattleDialogGraphicsSceneMouseHandlerCombatants _combatantMouseHandler;
    BattleDialogGraphicsSceneMouseHandlerMaps _mapsMouseHandler;
};

#endif // BATTLEDIALOGGRAPHICSSCENE_H
