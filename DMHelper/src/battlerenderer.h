#ifndef BATTLERENDERER_H
#define BATTLERENDERER_H

#include "campaignobjectrenderer.h"
#include "encounterbattle.h"
#include <QPixmap>
#include <QMap>

class BattleDialogGraphicsScene;
class BattleDialogModelCombatant;
class QGraphicsPixmapItem;
class QGraphicsItem;
class UnselectedPixmap;

class BattleRenderer : public CampaignObjectRenderer
{
    Q_OBJECT
public:
    explicit BattleRenderer(EncounterBattle& battle, QPixmap background, QObject *parent = nullptr);
    virtual ~BattleRenderer() override;

    enum BattleRendererItemChild
    {
        BattleRendererItemChild_Base = 0,
        BattleRendererItemChild_Selection,
        BattleRendererItemChild_AreaEffect,
        BattleRendererItemChild_Area,
        BattleRendererItemChild_PersonalEffect
    };

public slots:
    // From CampaignObjectRenderer
    virtual void startRendering() override;
    virtual void refreshRender() override;
    virtual void stopRendering() override;
    virtual void targetResized(const QSize& newSize) override;
    virtual void setRotation(int rotation) override;

    virtual void publishWindowMouseDown(const QPointF& position) override;
    virtual void publishWindowMouseMove(const QPointF& position) override;
    virtual void publishWindowMouseRelease(const QPointF& position) override;

    void updateModel();

protected:
    void getImageForPublishing(QImage& imageForPublishing);
    UnselectedPixmap* createCombatantIcon(BattleDialogModelCombatant* combatant);

    EncounterBattle& _battle;
    QGraphicsPixmapItem* _background;
    BattleDialogGraphicsScene* _scene;
    QMap<BattleDialogModelCombatant*, QGraphicsPixmapItem*> _combatantIcons;
    QSize _targetSize;
    int _rotation;

    BattleDialogModelCombatant* _selectedCombatant;
    QGraphicsItem* _selectedItem;

    QSizeF _scaledSceneSize;
    QRectF _targetRect;

    QGraphicsPixmapItem* _activePixmap;

};

#endif // BATTLERENDERER_H
