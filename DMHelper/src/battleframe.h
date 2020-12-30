#ifndef BATTLEFRAME_H
#define BATTLEFRAME_H

#include "campaignobjectframe.h"
#include <QMap>
#include <QUuid>
#include "battledialoggraphicsscene.h"
#include "battleframemapdrawer.h"
#include "battleframestatemachine.h"

class BattleDialogModelCombatant;
class CombatantWidget;
class QVBoxLayout;
class EncounterBattle;
class BattleDialogModel;
class BattleDialogLogger;
class Grid;
class Character;
class Map;
class QTimer;
class VideoPlayer;
class CameraRect;
class BattleCombatantFrame;
class UnselectedPixmap;
class CombatantRolloverFrame;

namespace Ui {
class BattleFrame;
}

const int BattleDialogItemChild_Index = 0;
const int BattleDialogItemChild_EffectId = 0;

class BattleFrame : public CampaignObjectFrame
{
    Q_OBJECT

public:
    explicit BattleFrame(QWidget *parent = nullptr);
    virtual ~BattleFrame() override;

    virtual void activateObject(CampaignObjectBase* object) override;
    virtual void deactivateObject() override;

    void setBattle(EncounterBattle* battle);
    EncounterBattle* getBattle();

    void setBattleMap();

    void addCombatant(BattleDialogModelCombatant* combatant);
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

    enum BattleDialogItemChild
    {
        BattleDialogItemChild_Base = 0,
        BattleDialogItemChild_Selection,
        BattleDialogItemChild_AreaEffect,
        BattleDialogItemChild_Area,
        BattleDialogItemChild_PersonalEffect
    };

    enum BattleFrameMode
    {
        BattleFrameMode_Battle = 0,
        BattleFrameMode_Combatants,
        BattleFrameMode_Map,
        BattleFrameMode_Grid,
        BattleFrameMode_Markers
    };

public slots:
    void clear();
    void sort();
    void next();

    void setTargetSize(const QSize& targetSize);
    void setTargetLabelSize(const QSize& targetSize);
    void publishWindowMouseDown(const QPointF& position);
    void publishWindowMouseMove(const QPointF& position);
    void publishWindowMouseRelease(const QPointF& position);

    void setGridScale(int gridScale);
    void setXOffset(int xOffset);
    void setYOffset(int yOffset);
    void setGridVisible(bool gridVisible);

    void setShowOnDeck(bool showOnDeck);
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

    void zoomIn();
    void zoomOut();
    void zoomFit();
    void zoomSelect(bool enabled);
    void cancelSelect();

    // Public for connection to battle ribbon
    void selectBattleMap();
    void reloadMap();
    void addMonsters();
    void addCharacter();
    void addNPC();
    void addObject();
    void castSpell();

    void addEffectRadius();
    void addEffectCone();
    void addEffectCube();
    void addEffectLine();

    // Public for connection to map ribbon
    void setCameraCouple(bool couple);
    void cancelCameraCouple();
    void setCameraMap();
    void setCameraSelect(bool enabled);
    void setCameraEdit(bool enabled);

    void setDistance(bool enabled);
    void setDistanceHeight(bool heightEnabled, qreal height);

    void setShowHeight(bool showHeight);
    void setHeight(qreal height);

    void setFoWEdit(bool enabled);
    void setFoWSelect(bool enabled);

    void setPointerOn(bool enabled);
    void showStatistics();

    // Publish slots from CampaignObjectFrame
    virtual void publishClicked(bool checked) override;
    virtual void setRotation(int rotation) override;
    virtual void setBackgroundColor(QColor color) override;
    virtual void reloadObject() override;

signals:
    void characterSelected(QUuid id);
    void monsterSelected(const QString& monsterClass);
    void animationStarted();
    void animateImage(QImage img);
    void showPublishWindow();
    void pointerChanged(const QCursor& cursor);

    void modelChanged(BattleDialogModel* model);

    void zoomSelectToggled(bool enabled);

    void cameraCoupleChanged(bool coupled);
    void cameraSelectToggled(bool enabled);
    void cameraEditToggled(bool enabled);

    void distanceToggled(bool enabled);
    void distanceChanged(const QString&);

    void foWEditToggled(bool enabled);
    void foWSelectToggled(bool enabled);

    void pointerToggled(bool enabled);

protected:
    virtual void keyPressEvent(QKeyEvent * e) override;
    virtual bool eventFilter(QObject *obj, QEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;

private slots:
    void setCompassVisibility(bool visible);
    void updateCombatantVisibility();
    void updateEffectLayerVisibility();
    void updateMap();
    void updateRounds();
    void updateVideoBackground();
    void handleContextMenu(BattleDialogModelCombatant* combatant, const QPoint& position);
    void handleEffectChanged(QGraphicsItem* effectItem);
    void handleEffectRemoved(QGraphicsItem* effectItem);
    void handleCombatantMoved(BattleDialogModelCombatant* combatant);
    void handleCombatantSelected(BattleDialogModelCombatant* combatant);
    void handleCombatantHover(BattleDialogModelCombatant* combatant, bool hover);
    void handleApplyEffect(QGraphicsItem* effect);

    void handleItemMouseDown(QGraphicsPixmapItem* item);
    void handleItemMoved(QGraphicsPixmapItem* item, bool* result);
    void handleItemMouseUp(QGraphicsPixmapItem* item);
    void handleItemChanged(QGraphicsItem* item);

    void removeCombatant();
    void activateCombatant();
    void damageCombatant();
    void setSelectedCombatant(BattleDialogModelCombatant* selected);
    void setUniqueSelection(BattleDialogModelCombatant* selected);
    void updateCombatantWidget(BattleDialogModelCombatant* combatant);
    void updateCombatantIcon(BattleDialogModelCombatant* combatant);
    void registerCombatantDamage(BattleDialogModelCombatant* combatant, int damage);

    void publishImage();
    void executeAnimateImage();
    void updateHighlights();
    void countdownTimerExpired();
    void updateCountdownText();
    void createPrescaledBackground();
    void handleRubberBandChanged(QRect rubberBandRect, QPointF fromScenePoint, QPointF toScenePoint);

    void setCombatantVisibility(bool aliveVisible, bool deadVisible, bool widgetsIncluded);
    void setEffectLayerVisibility(bool visibility);
    void setPublishVisibility(bool publish);
    void setGridOnlyVisibility(bool gridOnly);

    void setMapCursor();
    void setCameraSelectable(bool selectable);
    void setScale(qreal s);
    void storeViewRect();

    void setModel(BattleDialogModel* model);
    Map* selectRelatedMap();
    void selectAddCharacter(QList<Character*> characters, const QString& title, const QString& label);

    void setEditMode();
    void updateFowImage(const QPixmap& fow);
    void setItemsInert(bool inert);

    void removeRollover();

    // State Machine
    void stateUpdated();

private:

    CombatantWidget* createCombatantWidget(BattleDialogModelCombatant* combatant);
    void clearCombatantWidgets();
    void buildCombatantWidgets();
    void reorderCombatantWidgets();
    void setActiveCombatant(BattleDialogModelCombatant* active);
    void createCombatantIcon(BattleDialogModelCombatant* combatant);
    void relocateCombatantIcon(QGraphicsPixmapItem* icon);

    QWidget* findCombatantWidgetFromPosition(const QPoint& position) const;
    CombatantWidget* getWidgetFromCombatant(BattleDialogModelCombatant* combatant) const;
    void moveRectToPixmap(QGraphicsItem* rectItem, QGraphicsPixmapItem* pixmapItem);
    BattleDialogModelCombatant* getNextCombatant(BattleDialogModelCombatant* combatant);

    void getImageForPublishing(QImage& imageForPublishing);
    void updatePublishEnable();
    void createVideoPlayer(bool dmPlayer);
    void resetVideoSizes();

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
    QPoint getPrescaledRenderPos(QSize targetSize);

    bool convertPublishToScene(const QPointF& publishPosition, QPointF& scenePosition);

    void setCameraRect(bool cameraOn);
    void updateCameraRect();
    QRectF getCameraRect();
    void setCameraToView();

    // Helper functions to simplify rendering
    void renderPrescaledBackground(QPainter& painter, QSize targetSize);
    void renderVideoBackground(QPainter& painter);

    bool isItemInEffect(QGraphicsPixmapItem* item, QGraphicsItem* effect);
    void removeEffectsFromItem(QGraphicsPixmapItem* item);
    void applyEffectToItem(QGraphicsPixmapItem* item, BattleDialogModelEffect* effect);
    void applyPersonalEffectToItem(QGraphicsPixmapItem* item);

    void startMovement(QGraphicsPixmapItem* item, int speed);
    void updateMovement(QGraphicsPixmapItem* item);
    void endMovement();

    QPixmap getPointerPixmap();

    // State Machine
    void prepareStateMachine();

    Ui::BattleFrame *ui;
    EncounterBattle* _battle;
    BattleDialogModel* _model;
    QVBoxLayout* _combatantLayout;
    BattleDialogLogger* _logger;

    QMap<BattleDialogModelCombatant*, CombatantWidget*> _combatantWidgets;
    QMap<BattleDialogModelCombatant*, QGraphicsPixmapItem*> _combatantIcons;

    BattleFrameStateMachine _stateMachine;

    BattleDialogModelCombatant* _selectedCombatant;
    BattleDialogModelCombatant* _contextMenuCombatant;
    bool _mouseDown;
    QPoint _mouseDownPos;
    CombatantRolloverFrame* _hoverFrame;

    bool _publishMouseDown;
    QPointF _publishMouseDownPos;

    BattleDialogGraphicsScene* _scene;
    QGraphicsPixmapItem* _background;
    QGraphicsPixmapItem* _fow;
    QGraphicsPixmapItem* _activePixmap;
    qreal _activeScale;
    qreal _selectedScale;
    QGraphicsPixmapItem* _compassPixmap;
    QGraphicsEllipseItem* _movementPixmap;
    CameraRect* _publishRect;
    QRectF _publishRectValue;
    bool _cameraCoupled;
    bool _includeHeight;
    qreal _pitchHeight;

    QTimer* _countdownTimer;
    qreal _countdown;

    bool _publishing;
    QTimer* _publishTimer;

    QPixmap _prescaledBackground;
    QPixmap _fowImage;
    QImage _combatantFrame;
    QImage _countdownFrame;
    QSize _targetSize;
    QSize _targetLabelSize;

    BattleFrameMapDrawer* _mapDrawer;

    bool _showOnDeck;
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

    qreal _moveRadius;
    QPointF _moveStart;

    VideoPlayer* _videoPlayer;
    QImage _bwFoWImage;
    QRect _sourceRect;
    QSize _videoSize;
};

#endif // BATTLEFRAME_H
