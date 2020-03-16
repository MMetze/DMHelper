#ifndef BATTLEFRAME_H
#define BATTLEFRAME_H

#include <QFrame>
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
class UndoPath;

namespace Ui {
class BattleFrame;
}

class BattleFrame : public QFrame
{
    Q_OBJECT

public:
    explicit BattleFrame(QWidget *parent = nullptr);
    ~BattleFrame();

    void setBattle(EncounterBattle* battle);
    EncounterBattle* getBattle();

//    BattleDialogModel& getModel();
//    const BattleDialogModel& getModel() const;

    //EncounterBattle* battle() const;

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

public slots:
    void clear();
    void sort();
    void next();

    void setTargetSize(const QSize& targetSize);

    void setGridScale(int gridScale);
    void setXOffset(int xOffset);
    void setYOffset(int yOffset);
    void setGridVisible(bool gridVisible);

    void setShowOnDeck(bool showOnDeck);
    void setShowCountdown(bool showCountdown);
    void setCountdownDuration(int countdownDuration);

    void zoomIn();
    void zoomOut();
    void zoomFit();
    void zoomSelect();
    void cancelSelect();

    void cancelPublish();

signals:
    void battleComplete();
    void characterSelected(QUuid id);
    void monsterSelected(const QString& monsterClass);
    void publishImage(QImage img, QColor color);
    void animationStarted(QColor color);
    void animateImage(QImage img);
    void showPublishWindow();
    //void selectNewMap();
    //void addMonsters();
    //void addCharacter();
    //void addNPC();

protected:
    virtual void keyPressEvent(QKeyEvent * e);
    virtual bool eventFilter(QObject *obj, QEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void showEvent(QShowEvent *event);
    virtual void timerEvent(QTimerEvent *event);

private slots:
    void setCompassVisibility(bool visible);
    void setPointerVisibility(bool visible);
    void updateCombatantVisibility();
    void updateEffectLayerVisibility();
    void updateMap();
    void reloadMap();
    void updateVideoBackground();
    void handleContextMenu(BattleDialogModelCombatant* combatant, const QPoint& position);
    void handleBattleComplete();
    void handleSelectionChanged();
    void handleEffectChanged(QAbstractGraphicsShapeItem* effect);
    void handleCombatantMoved(BattleDialogModelCombatant* combatant);
    void handleApplyEffect(QAbstractGraphicsShapeItem* effect);

    void handleItemMouseDown(QGraphicsPixmapItem* item);
    void handleItemMoved(QGraphicsPixmapItem* item, bool* result);
    void handleItemMouseUp(QGraphicsPixmapItem* item);
    void handleItemChanged(QGraphicsItem* item);

    void removeCombatant();
    void activateCombatant();
    void damageCombatant();
    void setSelectedCombatant(BattleDialogModelCombatant* selected);
    void updateCombatantWidget(BattleDialogModelCombatant* combatant);
    void updateCombatantIcon(BattleDialogModelCombatant* combatant);
    void registerCombatantDamage(BattleDialogModelCombatant* combatant, int damage);

    void togglePublishing(bool publishing);
    void publishImage();
    void executePublishImage();
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
    void rotateCCW();
    void rotateCW();
    void storeViewRect();
    void setBackgroundColor(QColor color);
    void setDistanceText();

    void setModel(BattleDialogModel* model);
    void selectBattleMap();
    Map* selectRelatedMap();
    void addMonsters();
    void addCharacter();
    void addNPC();
    void selectAddCharacter(QList<Character*> characters, const QString& title, const QString& label);

    void setCameraCouple();
    void cancelCameraCouple();
    void setCameraMap();

    void setRibbonPage(int id);

    void setEditMode();
    void updateFowImage(const QPixmap& fow);
    void setItemsInert(bool inert);

    // State Machine
    void stateUpdated();

private:

    enum BattleDialogItemChild
    {
        BattleDialogItemChild_Base = 0,
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

    const int BattleDialogItemChild_Index = 0;

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
    void setCameraRect(bool cameraOn);
    void updateCameraRect();
    QRectF getCameraRect();
    void setCameraToView();

    // Helper functions to simplify rendering
    void renderPrescaledBackground(QPainter& painter, QSize targetSize);
    void renderVideoBackground(QPainter& painter);

    bool isItemInEffect(QGraphicsPixmapItem* item, QAbstractGraphicsShapeItem* effect);
    void removeEffectsFromItem(QGraphicsPixmapItem* item);
    void applyEffectToItem(QGraphicsPixmapItem* item, QAbstractGraphicsShapeItem* effect);
    void applyPersonalEffectToItem(QGraphicsPixmapItem* item);

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

    BattleDialogGraphicsScene* _scene;
    QGraphicsPixmapItem* _background;
    QGraphicsPixmapItem* _fow;
    QGraphicsPixmapItem* _activePixmap;
    int _activeScale;
    QGraphicsPixmapItem* _selectedPixmap;
    int _selectedScale;
    QGraphicsPixmapItem* _compassPixmap;
    QGraphicsEllipseItem* _movementPixmap;
    CameraRect* _publishRect;
    QRectF _publishRectValue;

    QTimer* _countdownTimer;
    qreal _countdown;

    bool _publishing;
    QTimer* _publishTimer;

    QPixmap _prescaledBackground;
    QPixmap _fowImage;
    QImage _combatantFrame;
    QImage _countdownFrame;
    QSize _targetSize;

    BattleFrameMapDrawer* _mapDrawer;

    bool _showOnDeck;
    bool _showCountdown;
    int _countdownDuration;
    QColor _countdownColor;

    QRect _rubberBandRect;
    qreal _scale;
    int _rotation;

    qreal _moveRadius;
    QPointF _moveStart;
    int _moveTimer;

    VideoPlayer* _videoPlayer;
    QImage _bwFoWImage;
    QRect _sourceRect;
    QSize _videoSize;
};

#endif // BATTLEFRAME_H
