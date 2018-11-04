#ifndef BATTLEDIALOG_H
#define BATTLEDIALOG_H

#include <QDialog>
#include <QMap>
#include "dmhsrc/battledialoggraphicsscene.h"

class BattleDialogModelCombatant;
class QVBoxLayout;
class BattleDialogModel;
class Grid;
class QTimer;

namespace Ui {
class BattleDialog;
}

class BattleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BattleDialog(BattleDialogModel& model, QWidget *parent = nullptr);
    ~BattleDialog();

    BattleDialogModel& getModel();
    const BattleDialogModel& getModel() const;

    //void setBattleMap();

    void addCombatant(BattleDialogModelCombatant* combatant);
    void addCombatants(QList<BattleDialogModelCombatant*> combatants);
    QList<BattleDialogModelCombatant*> getCombatants() const;
    //QList<BattleDialogModelCombatant*> getLivingCombatants() const;
    //BattleDialogModelCombatant* getFirstLivingCombatant() const;
    //QList<BattleDialogModelCombatant*> getMonsters() const;
    //QList<BattleDialogModelCombatant*> getLivingMonsters() const;

//    void recreateCombatantWidgets();

    QRect viewportRect();
    QPoint viewportCenter();

public slots:

    void updateMap();
    //void setTargetSize(const QSize& targetSize);

    void setGridScale(int gridScale);
    void setXOffset(int xOffset);
    void setYOffset(int yOffset);
    void setGridVisible(bool gridVisible);

    void setCountdownDuration(int countdownDuration);

    void zoomIn();
    void zoomOut();
    void zoomFit();
    void zoomSelect();
    void cancelSelect();

signals:
    void battleComplete();

protected:
    virtual void keyPressEvent(QKeyEvent * e);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void showEvent(QShowEvent *event);

private slots:
    void setCompassVisibility(bool visible);
    void updateCombatantVisibility();
    void updateEffectLayerVisibility();
    //void handleContextMenu(BattleDialogModelCombatant* combatant, const QPoint& position);
    //void handleBattleComplete();
    void handleSelectionChanged();
    void handleEffectChanged(QAbstractGraphicsShapeItem* effect);
    void handleCombatantMoved(BattleDialogModelCombatant* combatant);
    //void handleApplyEffect(QAbstractGraphicsShapeItem* effect);

    //void removeCombatant();
    //void activateCombatant();
    void setSelectedCombatant(BattleDialogModelCombatant* selected);
    //void updateCombatantWidget(BattleDialogModelCombatant* combatant);

    //void togglePublishing(bool publishing);
    //void publishImage();
    //void executePublishImage();
    //void executeAnimateImage();
    void updateHighlights();
    void countdownTimerExpired();
    void updateCountdownText();
    void createPrescaledBackground();
    void handleRubberBandChanged(QRect rubberBandRect, QPointF fromScenePoint, QPointF toScenePoint);

    void setCombatantVisibility();
    void setEffectLayerVisibility(bool visibility);
    //void setPublishVisibility(bool publish);

    void setMapCursor();
    void setScale(qreal s);
    void storeViewRect();

private:

    enum
    {
        BattleDialogItemChild_Base = 0,
        BattleDialogItemChild_AreaEffect,
        BattleDialogItemChild_Area,
        BattleDialogItemChild_PersonalEffect
    };

    const int BattleDialogItemChild_Index = 0;

    /*
    CombatantWidget* createCombatantWidget(BattleDialogModelCombatant* combatant);
    void clearCombatantWidgets();
    void buildCombatantWidgets();
    void reorderCombatantWidgets();
    */
    void setActiveCombatant(BattleDialogModelCombatant* active);
    void createCombatantIcon(BattleDialogModelCombatant* combatant);
    void relocateCombatantIcon(QGraphicsPixmapItem* icon);

    //QWidget* findCombatantWidgetFromPosition(const QPoint& position) const;
    //CombatantWidget* getWidgetFromCombatant(BattleDialogModelCombatant* combatant) const;
    void moveRectToPixmap(QGraphicsItem* rectItem, QGraphicsPixmapItem* pixmapItem);
    //BattleDialogModelCombatant* getNextCombatant(BattleDialogModelCombatant* combatant);

    //void getImageForPublishing(QImage& imageForPublishing);

    //void replaceBattleMap();
    //void resizeBattleMap();
    //int widthWindowToBackground(int windowWidth);
    //int widthBackgroundToWindow(int backgroundWidth);

    bool isItemInEffect(QGraphicsPixmapItem* item, QAbstractGraphicsShapeItem* effect);
    void removeEffectsFromItem(QGraphicsPixmapItem* item);
    void applyEffectToItem(QGraphicsPixmapItem* item, QAbstractGraphicsShapeItem* effect);
    void applyPersonalEffectToItem(QGraphicsPixmapItem* item);

    Ui::BattleDialog *ui;
    BattleDialogModel& _model;

    QMap<BattleDialogModelCombatant*, QGraphicsPixmapItem*> _combatantIcons;

    BattleDialogModelCombatant* _selectedCombatant;
    BattleDialogModelCombatant* _contextMenuCombatant;
    bool _mouseDown;
    QPoint _mouseDownPos;

    BattleDialogGraphicsScene* _scene;
    QGraphicsPixmapItem* _background;
    QGraphicsPixmapItem* _activePixmap;
    int _activeScale;
    QGraphicsPixmapItem* _selectedPixmap;
    int _selectedScale;
    QGraphicsPixmapItem* _compassPixmap;

    QTimer* _countdownTimer;
    int _countdown;

    QImage _prescaledBackground;
    QImage _combatantFrame;
    QImage _countdownFrame;
    QSize _targetSize;

    bool _showCountdown;
    int _countdownDuration;
    QColor _countdownColor;

    QRect _rubberBandRect;
    qreal _scale;
};

#endif // BATTLEDIALOG_H
