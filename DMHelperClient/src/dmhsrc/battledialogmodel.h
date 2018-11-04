#ifndef BATTLEDIALOGMODEL_H
#define BATTLEDIALOGMODEL_H

#include "dmhobjectbase.h"
#include "battledialogmodelcombatant.h"
#include "battledialogmodeleffect.h"
#include <QList>
#include <QRect>

//class EncounterBattle;
class Map;


class BattleDialogModel : public DMHObjectBase
{
    Q_OBJECT

public:
    explicit BattleDialogModel(QObject *parent = nullptr);
    BattleDialogModel(const BattleDialogModel& other, QObject *parent = nullptr);
    virtual ~BattleDialogModel();

    // From CampaignObjectBase
    //virtual void outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory);
    virtual void inputXML(const QDomElement &element);

    /*
    // Local
    EncounterBattle* getBattle() const;
    void setBattle(EncounterBattle* battle);
    */

    QList<BattleDialogModelCombatant*> getCombatantList() const;
    int getCombatantCount() const;
    BattleDialogModelCombatant* getCombatant(int index) const;
    BattleDialogModelCombatant* getCombatantById(int combatantId) const;
    void insertCombatant(int index, BattleDialogModelCombatant* combatant);
    BattleDialogModelCombatant* removeCombatant(int index);
    void appendCombatant(BattleDialogModelCombatant* combatant);
    void appendCombatants(QList<BattleDialogModelCombatant*> combatants);

    QList<BattleDialogModelEffect*> getEffectList() const;
    int getEffectCount() const;
    BattleDialogModelEffect* getEffect(int index) const;
    BattleDialogModelEffect* getEffectById(int effectId) const;
    void insertEffect(int index, BattleDialogModelEffect* effect);
    BattleDialogModelEffect* removeEffect(int index);
    bool removeEffect(BattleDialogModelEffect* effect);
    void appendEffect(BattleDialogModelEffect* effect);

    /*
    Map* getMap() const;
    void setMap(Map* map, const QRect& mapRect);
    */

    const QRect& getMapRect() const;
    void setMapRect(const QRect& mapRect);

    //bool isMapChanged() const;
    const QRect& getPreviousMapRect() const;
    //Map* getPreviousMap() const;

    bool getGridOn() const;
    void setGridOn(bool gridOn);

    int getGridScale() const;
    void setGridScale(int gridScale);

    int getGridOffsetX() const;
    void setGridOffsetX(int gridOffsetX);

    int getGridOffsetY() const;
    void setGridOffsetY(int gridOffsetY);

    bool getShowCompass() const;
    void setShowCompass(bool showCompass);

    bool getShowAlive() const;
    void setShowAlive(bool showAlive);

    bool getShowDead() const;
    void setShowDead(bool showDead);

    bool getShowEffects() const;
    void setShowEffects(bool showEffects);

    BattleDialogModelCombatant* getActiveCombatant() const;
    void setActiveCombatant(BattleDialogModelCombatant* activeCombatant);

    //void sortCombatants();

private:
    //static bool CompareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b);

    // Battle content values
    //EncounterBattle* _battle;

    QList<BattleDialogModelCombatant*> _combatants;
    QList<BattleDialogModelEffect*> _effects;

    // Visualization values
    //Map* _map;
    QRect _mapRect;
    //Map* _previousMap;
    QRect _previousMapRect;

    bool _gridOn;
    int _gridScale;
    int _gridOffsetX;
    int _gridOffsetY;

    bool _showCompass;
    bool _showAlive;
    bool _showDead;
    bool _showEffects;

    BattleDialogModelCombatant* _activeCombatant;
};

#endif // BATTLEDIALOGMODEL_H
