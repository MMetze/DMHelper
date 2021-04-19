#ifndef BATTLERENDERER_H
#define BATTLERENDERER_H

#include "campaignobjectrenderer.h"
#include "encounterbattle.h"

class BattleDialogGraphicsScene;

class BattleRenderer : public CampaignObjectRenderer
{
    Q_OBJECT
public:
    explicit BattleRenderer(EncounterBattle& battle, QObject *parent = nullptr);
    virtual ~BattleRenderer() override;

public slots:
    // From CampaignObjectRenderer
    virtual void startRendering() override;
    virtual void stopRendering() override;
    virtual void targetResized(const QSize& newSize) override;
    virtual void setRotation(int rotation) override;

private:
    EncounterBattle& _battle;
    BattleDialogGraphicsScene* _scene;
};

#endif // BATTLERENDERER_H
