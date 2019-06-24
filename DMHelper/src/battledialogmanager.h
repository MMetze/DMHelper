#ifndef BATTLEDIALOGMANAGER_H
#define BATTLEDIALOGMANAGER_H

#include "battledialogmodel.h"
#include "dmconstants.h"
#include <QObject>
#include <QImage>
#include <QString>

class CampaignObjectBase;
class Campaign;
class Adventure;
class Encounter;
class EncounterBattle;
class Map;
class Combatant;
class BattleDialog;
class NetworkController;

class BattleDialogManager : public QObject
{
    Q_OBJECT
public:
    explicit BattleDialogManager(QWidget *parent = nullptr);
    virtual ~BattleDialogManager();

    QList<BattleDialogModelCombatant*> getLivingMonsters() const;
#ifdef INCLUDE_NETWORK_SUPPORT
    void setNetworkManager(NetworkController* networkManager);
#endif

signals:
    void battleActive(bool active);

    void characterSelected(QUuid characterId);
    void monsterSelected(QString monster);
    void publishImage(QImage image, QColor color);
    void animationStarted(QColor color);
    void animateImage(QImage image);
    void showPublishWindow();

    void dirty();

public slots:
    void loadBattle(EncounterBattle* battleEncounter);
    void deleteBattle(EncounterBattle* battleEncounter);

    void setCampaign(Campaign* campaign);

    void showBattleDialog();
    void startNewBattle(EncounterBattle* battleEncounter);
    void addMonsters();
    void addWave();
    void addCharacter();
    void addNPC();
    void targetResized(const QSize& newSize);

    void setShowOnDeck(bool showOnDeck);
    void setShowCountdown(bool showCountdown);
    void setCountdownDuration(int countdownDuration);

    void cancelPublish();

protected slots:
    void completeBattle();
    void selectBattleMap(Map* preselectedMap, Adventure* relatedAdventure);
    void selectBattleMap();

protected:
    void uploadBattleModel();
    BattleDialog* createBattleDialog(BattleDialogModel* dlgModel);
    Map* selectRelatedMap(Adventure* adventure);
    QList<BattleDialogModelCombatant*> createWaveMonsters(EncounterBattle* battle, int wave);

    BattleDialog* _dlg;
    EncounterBattle* _encounterBattle;
    Campaign* _campaign;
#ifdef INCLUDE_NETWORK_SUPPORT
    NetworkController* _networkManager;
#endif
    QSize _targetSize;

    bool _showOnDeck;
    bool _showCountdown;
    int _countdownDuration;
};

#endif // BATTLEDIALOGMANAGER_H
