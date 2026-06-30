#ifndef RIBBONTABBATTLE_H
#define RIBBONTABBATTLE_H

#include "ribbonframe.h"
#include <QAction>

namespace Ui {
class RibbonTabBattle;
}

class Party;
class QMenu;

class RibbonTabBattle : public RibbonFrame
{
    Q_OBJECT

public:
    explicit RibbonTabBattle(QWidget *parent = nullptr);
    ~RibbonTabBattle();

    virtual PublishButtonRibbon* getPublishRibbon() override;

public slots:
    void setParty(Party* party);
    void setPartyIcon(const QString& partyIcon);
    void registerPartyIcon(Party* party);
    void removePartyIcon(Party* party);
    void clearPartyIcons();
    void setShowParty(bool showParty);
    void setShowMarkers(bool showMarkers);

    void setShowLiving(bool checked);
    void setShowDead(bool checked);
    void setShowEffects(bool checked);
    void setShowMovement(bool checked);
    void setLairActionsVisible(bool visible);

signals:
    void partySelected(Party* party);
    void partyIconSelected(const QString& partyIcon);

    void showPartyClicked(bool showParty);
    void showMarkersClicked(bool showMarkers);
    void addMarkerClicked();

    void addCharacterClicked();
    void addMonsterClicked();
    void addNPCClicked();
    void addObjectClicked();
    void castSpellClicked();
    void addEffectRadiusClicked();
    void addEffectCubeClicked();
    void addEffectConeClicked();
    void addEffectLineClicked();
    void addEffectSmokeClicked();
    void addEffectFireClicked();
    void addEffectSparksClicked();
    void addEffectLightClicked();
    void duplicateClicked();
    void showLivingClicked(bool checked);
    void showDeadClicked(bool checked);
    void showEffectsClicked(bool checked);
    void showMovementClicked(bool checked);
    void lairActionsClicked();
    void addEventClicked();
    void statisticsClicked();

protected:
    virtual void showEvent(QShowEvent *event) override;

protected slots:
    void selectPartyAction(QAction* action);
    void setPartyButtonIcon(const QIcon &icon);
    void selectEffectAction(QAction* action);

private:
    Ui::RibbonTabBattle *ui;
    QMenu* _partyMenu;
};

class RibbonTabBattle_PartyAction : public QAction
{
    Q_OBJECT

public:
    enum PartyActionType
    {
        PartyActionType_Invalid = -1,
        PartyActionType_Party = 0,
        PartyActionType_Default,
        PartyActionType_Select
    };

    explicit RibbonTabBattle_PartyAction(Party* party, int partyType = PartyActionType_Party, QObject *parent = nullptr);
    virtual ~RibbonTabBattle_PartyAction() override;

    Party* getParty() const;
    int getPartyType() const;

public slots:
    void updateParty();
    void partyDestroyed();

protected:
    Party* _party;
    int _partyType;
};

#endif // RIBBONTABBATTLE_H
