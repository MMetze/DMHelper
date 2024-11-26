#ifndef BATTLEDIALOGGRAPHICSSCENE_H
#define BATTLEDIALOGGRAPHICSSCENE_H

#include <QPixmap>
#include "camerascene.h"
#include "battledialoggraphicsscenemousehandler.h"

class BattleDialogModel;
class BattleDialogModelObject;
class BattleDialogModelEffect;
class BattleDialogModelCombatant;
class BattleDialogModelMonsterClass;
class QAbstractGraphicsShapeItem;
class QMimeData;

class BattleDialogGraphicsScene : public CameraScene
{
    Q_OBJECT

public:
    explicit BattleDialogGraphicsScene(QObject *parent = nullptr);
    virtual ~BattleDialogGraphicsScene();

    void setModel(BattleDialogModel* model);
    BattleDialogModel* getModel() const;

    void createBattleContents();
    void scaleBattleContents();
    void clearBattleContents();

    void setPointerVisibility(bool visible);
    void setPointerPos(const QPointF& pos);
    void setPointerPixmap(QPixmap pixmap);

    QPixmap getSelectedIcon() const;
    QString getSelectedIconFile() const;
    QPointF getCommandPosition() const;

    QGraphicsItem* getDistanceLine() const;
    QGraphicsSimpleTextItem* getDistanceText() const;

    QGraphicsItem* findTopObject(const QPointF &pos);
    BattleDialogModelObject* getFinalObjectFromItem(QGraphicsItem* item);

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

    void setSelectedIcon(const QString& selectedIcon);

signals:
    void addEffectRadius();
    void addEffectCone();
    void addEffectCube();
    void addEffectLine();
    void duplicateSelection();
    void addPC();
    void addMonsters();
    void addNPC();
    void addEffectObject();
    void addEffectObjectVideo();
    void addLayerImageFile(const QString& filename);
    void addEffectObjectFile(const QString& filename);
    void castSpell();

    void effectChanged(QGraphicsItem* effect);
    void applyEffect(QGraphicsItem* effect);
    void distanceChanged(const QString& distance);
    void distanceItemChanged(QGraphicsItem* shapeItem, QGraphicsSimpleTextItem* textItem);

    void pointerMove(const QPointF& pos);

    void battleMousePress(const QPointF& pos);
    void battleMouseMove(const QPointF& pos);
    void battleMouseRelease(const QPointF& pos);

    void mapMoveToggled();
    void mapMousePress(const QPointF& pos);
    void mapMouseMove(const QPointF& pos);
    void mapMouseRelease(const QPointF& pos);

    void mapZoom(int zoomFactor);

    void itemMouseDown(QGraphicsPixmapItem* item, bool showMovement);
    void itemMoved(QGraphicsPixmapItem* item, bool* result);
    void itemMouseUp(QGraphicsPixmapItem* item);
    void itemMouseDoubleClick(QGraphicsPixmapItem* item);

    void itemChangeLayer(BattleDialogModelObject* effect);
    void itemLink(BattleDialogModelObject* item);
    void itemUnlink(BattleDialogModelObject* item);

    void combatantActivate(BattleDialogModelCombatant* combatant);
    void combatantRemove(BattleDialogModelCombatant* combatant);
    void combatantDamage(BattleDialogModelCombatant* combatant);
    void combatantHeal(BattleDialogModelCombatant* combatant);
    void monsterChangeToken(BattleDialogModelMonsterClass* monster, int iconIndex);

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
    void changeMonsterToken(BattleDialogModelMonsterClass* monster, int iconIndex);

    void changeEffectLayer();
    void handleSelectionChanged();

protected:
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    virtual void wheelEvent(QGraphicsSceneWheelEvent *wheelEvent) override;
    virtual void keyPressEvent(QKeyEvent *keyEvent) override;
    virtual void keyReleaseEvent(QKeyEvent *keyEvent) override;

    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
    virtual void dropEvent(QGraphicsSceneDragDropEvent *event) override;

    bool isMimeDataImage(const QMimeData* mimeData) const;
    QString getMimeDataImageFile(const QMimeData* mimeData) const;

//    BattleDialogModelEffect* createEffect(int type, int size, int width, const QColor& color, const QString& filename);

    BattleDialogGraphicsSceneMouseHandlerBase* getMouseHandler(QGraphicsSceneMouseEvent *mouseEvent);

    QGraphicsItem* _contextMenuItem;
    BattleDialogModel* _model;

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
    int _selectionCount;

    BattleDialogGraphicsSceneMouseHandlerDistance _distanceMouseHandler;
    BattleDialogGraphicsSceneMouseHandlerFreeDistance _freeDistanceMouseHandler;
    BattleDialogGraphicsSceneMouseHandlerPointer _pointerMouseHandler;
    BattleDialogGraphicsSceneMouseHandlerRaw _rawMouseHandler;
    BattleDialogGraphicsSceneMouseHandlerCamera _cameraMouseHandler;
    BattleDialogGraphicsSceneMouseHandlerCombatants _combatantMouseHandler;
    BattleDialogGraphicsSceneMouseHandlerMaps _mapsMouseHandler;
};

#endif // BATTLEDIALOGGRAPHICSSCENE_H
