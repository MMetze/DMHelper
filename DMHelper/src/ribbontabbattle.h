#ifndef RIBBONTABBATTLE_H
#define RIBBONTABBATTLE_H

#include "ribbonframe.h"

namespace Ui {
class RibbonTabBattle;
}

class RibbonTabBattle : public RibbonFrame
{
    Q_OBJECT

public:
    explicit RibbonTabBattle(QWidget *parent = nullptr);
    ~RibbonTabBattle();

    virtual PublishButtonRibbon* getPublishRibbon() override;

public slots:
    void setShowLiving(bool checked);
    void setShowDead(bool checked);
    void setShowEffects(bool checked);
    void setShowMovement(bool checked);
    void setFowMovement(bool checked);
    void setLairActions(bool checked);

signals:
    void newMapClicked();
    void reloadMapClicked();
    void addCharacterClicked();
    void addMonsterClicked();
    void addNPCClicked();
    void addObjectClicked();
    void castSpellClicked();
    void addEffectRadiusClicked();
    void addEffectCubeClicked();
    void addEffectConeClicked();
    void addEffectLineClicked();
    void showLivingClicked(bool checked);
    void showDeadClicked(bool checked);
    void showEffectsClicked(bool checked);
    void showMovementClicked(bool checked);
    void fowMovementClicked(bool checked);
    void lairActionsClicked(bool checked);
    void statisticsClicked();

protected:
    virtual void showEvent(QShowEvent *event) override;

private:
    Ui::RibbonTabBattle *ui;
};

#endif // RIBBONTABBATTLE_H
