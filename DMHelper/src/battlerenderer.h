#ifndef BATTLERENDERER_H
#define BATTLERENDERER_H

#include "campaignobjectrenderer.h"
#include "encounterbattle.h"
#include <QPixmap>

class BattleDialogGraphicsScene;

class BattleRenderer : public CampaignObjectRenderer
{
    Q_OBJECT
public:
    explicit BattleRenderer(EncounterBattle& battle, QPixmap background, QObject *parent = nullptr);
    virtual ~BattleRenderer() override;

public slots:
    // From CampaignObjectRenderer
    virtual void startRendering() override;
    virtual void stopRendering() override;
    virtual void targetResized(const QSize& newSize) override;
    virtual void setRotation(int rotation) override;

protected:
    void getImageForPublishing(QImage& imageForPublishing);

    EncounterBattle& _battle;
    QPixmap _background;
    BattleDialogGraphicsScene* _scene;
    QSize _targetSize;
    int _rotation;
};

#endif // BATTLERENDERER_H
