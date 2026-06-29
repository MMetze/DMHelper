#ifndef BATTLEFRAME_H
#define BATTLEFRAME_H

#include "campaignobjectframe.h"
#include <QMap>
#include <QUuid>
#include "battledialoggraphicsscene.h"
#include "battleframemapdrawer.h"
#include "battleframestatemachine.h"

#include "characterv2.h"
#include "battledialogmodelcombatant.h"
#include "battledialogmodel.h"
#include "layer.h"
#include "publishglrenderer.h"

class BattleDialogModelCombatantGroup;
class CombatantWidget;
class CombatantGroupWidget;
class CombatantDialog;
class QVBoxLayout;
class EncounterBattle;
class BattleDialogLogger;
class Grid;
class GridConfig;
class GridSizer;
class Map;
class QTimer;
class RuleHealth;
class CameraRect;
class BattleCombatantFrame;
class MapPartyIconItem;
class UndoMarker;
class QGraphicsPolygonItem;
class QGraphicsRectItem;
class QGraphicsLineItem;
class UnselectedPixmap;
class CombatantRolloverFrame;
class PublishGLBattleRenderer;
class LayerTokens;
class LayerDrawEngine;

namespace Ui {
class BattleFrame;
}

//#define DEBUG_FILL_BOUNDING_RECTS

class BattleFrame : public CampaignObjectFrame
{
    Q_OBJECT

public:
    explicit BattleFrame(QWidget *parent = nullptr);
    virtual ~BattleFrame() override;

    virtual void activateObject(CampaignObjectBase* object, PublishGLRenderer* currentRenderer) override;
    virtual void deactivateObject() override;

    void setBattle(EncounterBattle* battle);
    EncounterBattle* getBattle();

    void setBattleMap();

    void addCombatant(BattleDialogModelCombatant* combatant, LayerTokens* targetLayer = nullptr);
    void addCombatants(QList<BattleDialogModelCombatant*> combatants);
    QList<BattleDialogModelCombatant*> getCombatants() const;
    QList<BattleDialogModelCombatant*> getLivingCombatants() const;
    BattleDialogModelCombatant* getFirstLivingCombatant() const;
    QList<BattleDialogModelCombatant*> getMonsters() const;
    QList<BattleDialogModelCombatant*> getLivingMonsters() const;

    void recreateCombatantWidgets();

    QRect viewportRect();
    QPoint viewportCenter();

    BattleFrameMapDrawer* getMapDrawer() const;

    enum BattleFrameMode
    {
        BattleFrameMode_Battle = 0,
        BattleFrameMode_Combatants,
        BattleFrameMode_Map,
        BattleFrameMode_Grid
    };

public slots:
    void clear();
    void roll();
    void sort();
    void top();
    void next();

    void setTargetSize(const QSize& targetSize);
    void setTargetLabelSize(const QSize& targetSize);
    void publishWindowMouseDown(const QPointF& position);
    void publishWindowMouseMove(const QPointF& position);
    void publishWindowMouseRelease(const QPointF& position);

    void setGridScale(int gridScale);
    void setGridScale(int gridScale, int xOffset, int yOffset);
    void selectGridCount();
    void resizeGrid();
    void setGridAngle(int gridAngle);
    void setGridType(int gridType);
    void setXOffset(int xOffset);
    void setYOffset(int yOffset);
    void setGridWidth(int gridWidth);
    void setGridColor(const QColor& gridColor);
    void setRatioLocked(bool ratioLocked);
    void setGridLocked(bool gridLocked);
    void setGridLockScale(qreal gridLockScale);
    void setSnapToGrid(bool snapToGrid);

    void setInitiativeType(int initiativeType);
    void setInitiativeScale(qreal initiativeScale);
    void setCombatantTokenType(int combatantTokenType);
    void setShowCountdown(bool showCountdown);
    void setCountdownDuration(int countdownDuration);
    void setPointerFile(const QString& filename);
    void setSelectedIcon(const QString& selectedIcon);
    void setActiveIcon(const QString& activeIcon);
    void createActiveIcon();
    void setCombatantFrame(const QString& combatantFrame);
    void createCombatantFrame();
    void setCountdownFrame(const QString& countdownFrame);
    void createCountdownFrame();

    void setShowParty(bool showParty);
    void setShowMarkers(bool showMarkers);
    void setPartySelected(bool selected);
    void addNewMarker();
    void addMarker(const QPointF& markerPosition);
    void editMapMarker(UndoMarker* marker);
    void deleteMapMarker(UndoMarker* marker);

    void zoomIn();
    void zoomOut();
    void zoomFit();
    void zoomSelect(bool enabled);
    void zoomDelta(int delta);
    void cancelSelect();
    void ribbonTabChanged();

    // Public for connection to battle ribbon
    bool createNewBattle();
    void reloadMap();
    void colorize();
    void editMapFile();
    void addMonsters();
    void addCharacter();
    void addNPC();
    void addInitiativeEvent();
    void addLairActionsEvent();
    void addEffectObject();
    void addEffectObjectFile(const QString& filename);
    void addMonsterImageFile(const QString& filename, const QPointF& position);
    void addEffectObjectVideo();
    void addEffectObjectVideoFile(const QString& filename);
    void addLayerImageFile(const QString& filename);
    void castSpell();

    void addEffectRadius();
    void addEffectCone();
    void addEffectCube();
    void addEffectLine();
    void addEffectSmoke();
    void addEffectFire();
    void addEffectSparks();
    void addEffectLight();
    void registerEffect(BattleDialogModelEffect* effect);

    void duplicateSelection();
    bool duplicateItem(QGraphicsItem* item);
    bool duplicateCombatant(LayerTokens* tokenLayer, BattleDialogModelCombatant* combatant);
    bool duplicateEffect(LayerTokens* tokenLayer, BattleDialogModelEffect* effect);

    // Public for connection to map ribbon
    void setCameraCouple();
    void setCameraMap();
    void setCameraVisible();
    void setCameraSelect(bool enabled);
    void setCameraEdit(bool enabled);

    void setDistance(bool enabled);
    void setFreeDistance(bool enabled);
    void setDistanceHeight(bool heightEnabled, qreal height);
    void setDistanceScale(int scale);
    void setDistanceLineColor(const QColor& color);
    void setDistanceLineType(int lineType);
    void setDistanceLineWidth(int lineWidth);

    void setShowHeight(bool showHeight);
    void setHeight(qreal height);

    void setFoWEdit(bool enabled);
    void setFoWSelect(bool enabled);

    void setPointerOn(bool enabled);
    void setDrawOn(bool enabled);
    void showStatistics();

    void layerSelected(int selected);

    // Publish slots from CampaignObjectFrame
    virtual void publishClicked(bool checked) override;
    virtual void setRotation(int rotation) override;
    virtual void setBackgroundColor(const QColor& color) override;
    virtual void editLayers() override;

signals:
    void characterSelected(QUuid id);
    void monsterSelected(const QString& monsterClass);

    void registerRenderer(PublishGLRenderer* renderer);
    void setLayers(QList<Layer*> layers, int selected);

    void initiativeActiveChanged(bool initiativeActive);

    void showPublishWindow();
    void pointerChanged(const QCursor& cursor);

    void modelChanged(BattleDialogModel* model);

    void gridConfigChanged(const GridConfig& config);
    void zoomSelectToggled(bool enabled);

    void cameraSelectToggled(bool enabled);
    void cameraEditToggled(bool enabled);
    void cameraRectChanged(const QRectF& cameraRect);

    void distanceToggled(bool enabled);
    void freeDistanceToggled(bool enabled);
    void distanceChanged(const QString&);

    void showPartyChanged(bool showParty);
    void showMarkersChanged(bool showMarkers);
    void markerChanged();

    void mapMoveToggled();

    void foWEditToggled(bool enabled);
    void foWSelectToggled(bool enabled);
    void mapCreated();

    void pointerToggled(bool enabled);
    void pointerFileNameChanged(const QString& filename);
    void drawToggled(bool enabled);

    void movementChanged(bool visible, BattleDialogModelCombatant* combatant, qreal remaining);

    void navigateForwards();
    void navigateBackwards();

protected:
    virtual void keyPressEvent(QKeyEvent * event) override;
    virtual bool eventFilter(QObject *obj, QEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;

private slots:
    void updateCombatantVisibility();
    void updateMap();
    void updateRounds();
    void checkPartyUpdate();
    void handlePartyIconMoved(const QPointF& pos);
    void handleMarkerMoved(UndoMarker* marker);
    void handleContextMenu(BattleDialogModelCombatant* combatant, const QPoint& position);
    void handleGroupContextMenu(BattleDialogModelCombatantGroup* group, const QPoint& position);
    void handleGroupClicked(CombatantGroupWidget* groupWidget);
    void handleCombatantSelected(BattleDialogModelCombatant* combatant);
    void handleCombatantHover(BattleDialogModelCombatant* combatant, bool hover);
    void handleCombatantActivate(BattleDialogModelCombatant* combatant);
    void handleCombatantRemove(BattleDialogModelCombatant* combatant);
    void handleCombatantAdded(BattleDialogModelCombatant* combatant);
    void handleCombatantRemoved(BattleDialogModelCombatant* combatant);
    void handleCombatantVisibilityChanged(BattleDialogModelCombatant* combatant);
    void handleCombatantDamage(BattleDialogModelCombatant* combatant);
    void handleCombatantHeal(BattleDialogModelCombatant* combatant);
    void handleCombatantEditConditions(BattleDialogModelCombatant* combatant);
    void handleCombatantHideSelected(BattleDialogModelCombatant* combatant);
    void handleCombatantUnhideSelected(BattleDialogModelCombatant* combatant);
    void handleCombatantKnowSelected(BattleDialogModelCombatant* combatant);
    void handleCombatantUnknowSelected(BattleDialogModelCombatant* combatant);
    void handleCombatantGroupSelected();
    void handleCombatantUngroupSelected(BattleDialogModelCombatant* combatant);
    void handleCombatantRemoveFromGroup(BattleDialogModelCombatant* combatant);
    void handleChangeMonsterToken(BattleDialogModelMonsterClass* monster, int iconIndex);
    void handleChangeMonsterTokenCustom(BattleDialogModelMonsterClass* monster);
    void handleChangeCharacterToken(BattleDialogModelCharacter* character, int iconIndex);
    void handleChangeCharacterTokenCustom(BattleDialogModelCharacter* character);
    void handleApplyEffect(QGraphicsItem* effect);

    void handleItemChangeLayer(BattleDialogModelObject* battleObject);
    void handleItemLink(BattleDialogModelObject* item);
    void handleItemUnlink(BattleDialogModelObject* item);

    void handleItemMouseDown(QGraphicsPixmapItem* item, bool showMovement);
    void handleItemMoved(QGraphicsPixmapItem* item, bool* result);
    void handleItemMouseUp(QGraphicsPixmapItem* item);
    void handleItemChanged(QGraphicsItem* item);
    void handleItemMouseDoubleClick(QGraphicsPixmapItem* item);

    void handleMapMousePress(const QPointF& pos);
    void handleMapMouseMove(const QPointF& pos);
    void handleMapMouseRelease(const QPointF& pos);
    void handleSceneChanged(const QList<QRectF> &region);
    void handleLayersChanged();
    void handleLayerSelected(Layer* layer);

    void handleDrawToggled(bool enabled);

    void itemLink();
    void itemUnlink();

    void removeCombatant();
    void activateCombatant();
    void changeCombatantLayer();
    void damageCombatant();
    void healCombatant();
    void editSelectedCombatantConditions();
    void hideSelectedCombatant();
    void unhideSelectedCombatant();
    void knowSelectedCombatant();
    void unknowSelectedCombatant();
    void groupSelectedCombatants();
    void ungroupSelectedCombatants();
    void removeFromGroup();
    void applyCombatantHPChange(BattleDialogModelCombatant* combatant, int hpChange);
    void setSelectedCombatant(BattleDialogModelCombatant* selected);
    void setUniqueSelection(BattleDialogModelCombatant* selected);
    void updateCombatantWidget(BattleDialogModelCombatant* combatant);
    void updateCombatantIcon(BattleDialogModelCombatant* combatant);
    void registerCombatantDamage(BattleDialogModelCombatant* combatant, int damage);

    void addMonsterFinished(CombatantDialog* combatantDlg, int result);
    void copyMonsters();
    void clearCopy();
    void pasteMonsters();

    void updateHighlights();
    void countdownTimerExpired();
    void updateCountdownText();
    void handleRubberBandChanged(QRect rubberBandRect, QPointF fromScenePoint, QPointF toScenePoint);

    void setCombatantVisibility(bool aliveVisible, bool deadVisible);
    void setSingleCombatantVisibility(BattleDialogModelCombatant* combatant, bool aliveVisible, bool deadVisible);

    void setMapCursor();
    void setCameraSelectable(bool selectable);
    void setScale(qreal s);
    void storeViewRect();

    void gridSizerAccepted();
    void gridSizerRejected();

    void setModel(BattleDialogModel* model);
    Map* selectRelatedMap();
    void selectAddCharacter(QList<Characterv2*> characters, const QString& title, const QString& label);

    void setEditMode();
    void setItemsInert(bool inert);

    void handlePolygonChanged(const QPolygonF& polygon);
    void handlePolygonCancelled();
    void handlePolygonPendingLineChanged(const QLineF& line);

    void handleSelectRectChanged(const QRectF& rect);
    void handleSelectRectCancelled();

    void removeRollover();
    void clearDoneFlags();

    void rendererActivated(PublishGLBattleRenderer* renderer);
    void rendererDeactivated();

    // State Machine
    void stateUpdated();

private:
    
    CombatantWidget* createCombatantWidget(BattleDialogModelCombatant* combatant);
    void clearCombatantWidgets();
    void buildCombatantWidgets();
    void reorderCombatantWidgets();
    void setActiveCombatant(BattleDialogModelCombatant* active);
    void relocateCombatantIcon(QGraphicsPixmapItem* icon);

    void newRound();

    // Resolve the active ruleset's RuleHealth, or nullptr when no campaign /
    // ruleset is reachable (e.g. detached unit-test paths). Callers should
    // fall back to the legacy getHitPoints() <= 0 predicate when this returns
    // nullptr so behaviour matches the pre-refactor code.
    RuleHealth* currentRuleHealth() const;
    bool isCombatantDead(const BattleDialogModelCombatant* combatant) const;

    QWidget* findCombatantWidgetFromPosition(const QPoint& position) const;
    QGraphicsPixmapItem* getItemFromCombatant(BattleDialogModelCombatant* combatant) const;
    BattleDialogModelObject* getObjectFromItem(QGraphicsItem* item) const;
    BattleDialogModelCombatant* getCombatantFromItem(QGraphicsItem* item) const;
    BattleDialogModelCombatant* getCombatantFromItem(QGraphicsPixmapItem* item) const;
    CombatantWidget* getWidgetFromCombatant(BattleDialogModelCombatant* combatant) const;
    void moveRectToPixmap(QGraphicsItem* rectItem, QGraphicsPixmapItem* pixmapItem);
    BattleDialogModelCombatant* getNextCombatant(BattleDialogModelCombatant* combatant);
    void removeSingleCombatant(BattleDialogModelCombatant* combatant);
    QList<BattleDialogModelCombatant*> getContextMenuCombatants(BattleDialogModelCombatant* combatant) const;
    void editCombatantConditions(const QList<BattleDialogModelCombatant*>& combatants);

    bool validateTokenLayerExists();
    void moveCombatantToLayer(BattleDialogModelCombatant* combatant, LayerTokens* newLayer);
    void moveEffectToLayer(BattleDialogModelEffect* effect, LayerTokens* newLayer, QList<Layer*> tokenLayers);

    void updatePublishEnable();

    void clearBattleFrame();
    void cleanupBattleMap();
    void replaceBattleMap();
    bool doSceneContentsExist();
    void createSceneContents();
    void resizeBattleMap();
    int widthFrameToBackground(int frameWidth);
    int widthBackgroundToFrame(int backgroundWidth);
    QSize sizeFrameToBackground(const QSize& frameSize);
    QSize sizeBackgroundToFrame(const QSize& backgroundSize);
    int getFrameWidth();
    int getFrameHeight();
    QSize getTargetBackgroundSize(const QSize& originalBackgroundSize, const QSize& targetSize);
    QSize getRotatedTargetBackgroundSize(const QSize& originalBackgroundSize);
    QSize getRotatedTargetFrameSize(const QSize& originalBackgroundSize);

    bool convertPublishToScene(const QPointF& publishPosition, QPointF& scenePosition);

    void updateCameraRect();
    QRectF getCameraRect();
    void setCameraToView();

    BattleDialogModelEffect* createEffect(int type, int size, int width, const QColor& color, const QString& filename);

    bool isItemInEffect(QGraphicsPixmapItem* item, QGraphicsItem* effect);
    void applyPersonalEffectToItem(QGraphicsPixmapItem* item);

    void startMovement(BattleDialogModelCombatant* combatant, QGraphicsPixmapItem* item, int speed);
    void updateMovement(BattleDialogModelCombatant* combatant, QGraphicsPixmapItem* item);
    void endMovement();

    QPixmap getPointerPixmap();

    // State Machine
    void prepareStateMachine();

    Ui::BattleFrame *ui;
    EncounterBattle* _battle;
    BattleDialogModel* _model;
    QVBoxLayout* _combatantLayout;
    BattleDialogLogger* _logger;
    LayerDrawEngine* _drawEngine;
    
    QMap<BattleDialogModelCombatant*, CombatantWidget*> _combatantWidgets;
    QMap<QUuid, CombatantGroupWidget*> _groupWidgets;

    BattleFrameStateMachine _stateMachine;

    BattleDialogModelCombatant* _selectedCombatant;
    BattleDialogModelCombatant* _contextMenuCombatant;
    bool _mouseDown;
    QPoint _mouseDownPos;
    CombatantRolloverFrame* _hoverFrame;
    QWidget* _hoverFrameOwner;
    bool _dragInProgress;
    QWidget* _dragLastTarget;

    bool _publishMouseDown;
    QPointF _publishMouseDownPos;
    QGraphicsItem* _publishEffectItem;

    BattleDialogGraphicsScene* _scene;
    QGraphicsPixmapItem* _activePixmap;
    qreal _selectedScale;
    QGraphicsEllipseItem* _movementPixmap;
    CameraRect* _cameraRect;
    MapPartyIconItem* _partyIcon;
    QRectF _publishRectValue;
    bool _includeHeight;
    qreal _pitchHeight;

    QTimer* _countdownTimer;
    qreal _countdown;

    bool _isPublishing;
    bool _isVideo;

    QPixmap _fowImage;
    QImage _bwFoWImage;
    QImage _combatantFrame;
    QImage _countdownFrame;
    QSize _targetSize;
    QSize _targetLabelSize;

    GridSizer* _gridSizer;
    bool _isRatioLocked;
    bool _isGridLocked;
    qreal _gridLockScale;

    BattleFrameMapDrawer* _mapDrawer;
    QGraphicsPolygonItem* _polygonPreview;
    QGraphicsLineItem* _polygonPendingLine;
    QGraphicsRectItem* _selectRectPreview;

    PublishGLBattleRenderer* _renderer;

    int _initiativeType;
    qreal _initiativeScale;
    int _combatantTokenType;
    bool _showCountdown;
    int _countdownDuration;
    QColor _countdownColor;
    QString _pointerFile;
    QString _activeFile;
    QString _combatantFile;
    QString _countdownFile;

    QRect _rubberBandRect;
    qreal _scale;
    int _rotation;
    QList<BattleDialogModelCombatant*> _copyList;

    qreal _moveRadius;
    QPointF _moveStart;
};

#endif // BATTLEFRAME_H
