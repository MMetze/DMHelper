#ifndef BATTLERENDERER_H
#define BATTLERENDERER_H

#include "campaignobjectrenderer.h"
#include "encounterbattle.h"
#include <QPixmap>

class BattleDialogGraphicsScene;
class BattleDialogModelCombatant;
class QGraphicsPixmapItem;

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
    virtual void stopRendering() override;
    virtual void targetResized(const QSize& newSize) override;
    virtual void setRotation(int rotation) override;

protected:
    void getImageForPublishing(QImage& imageForPublishing);
    void createCombatantIcon(BattleDialogModelCombatant* combatant);

    EncounterBattle& _battle;
    QGraphicsPixmapItem* _background;
    BattleDialogGraphicsScene* _scene;
    QSize _targetSize;
    int _rotation;
};

#endif // BATTLERENDERER_H
