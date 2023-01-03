#ifndef BATTLEDIALOGMODEL_H
#define BATTLEDIALOGMODEL_H

#include "battledialogmodelcombatant.h"
#include "battledialogmodeleffect.h"
#include "battledialoglogger.h"
#include "layerscene.h"
#include <QList>
#include <QRect>
#include <QPen>

class EncounterBattle;
class Map;

class BattleDialogModel : public CampaignObjectBase
{
    Q_OBJECT

public:
    explicit BattleDialogModel(EncounterBattle* encounter, const QString& name = QString(), QObject *parent = nullptr);
    virtual ~BattleDialogModel() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;
    virtual int getObjectType() const override;

    virtual const CampaignObjectBase* getParentByType(int parentType) const override;
    virtual CampaignObjectBase* getParentByType(int parentType) override;

    virtual const CampaignObjectBase* getParentById(const QUuid& id) const override;
    virtual CampaignObjectBase* getParentById(const QUuid& id) override;

    QList<BattleDialogModelCombatant*> getCombatantList() const;
    int getCombatantCount() const;
    BattleDialogModelCombatant* getCombatant(int index) const;
    BattleDialogModelCombatant* getCombatantById(QUuid combatantId) const;
    void insertCombatant(int index, BattleDialogModelCombatant* combatant);
    BattleDialogModelCombatant* removeCombatant(int index);
    void appendCombatant(BattleDialogModelCombatant* combatant);
    void appendCombatants(QList<BattleDialogModelCombatant*> combatants);
    bool isCombatantInList(Combatant* combatant) const;

    QList<BattleDialogModelEffect*> getEffectList() const;
    int getEffectCount() const;
    BattleDialogModelEffect* getEffect(int index) const;
    BattleDialogModelEffect* getEffectById(QUuid effectId) const;
    void insertEffect(int index, BattleDialogModelEffect* effect);
    BattleDialogModelEffect* removeEffect(int index);
    bool removeEffect(BattleDialogModelEffect* effect);
    void appendEffect(BattleDialogModelEffect* effect);

    int getGridScale() const;

    Map* getMap() const;
    const QRect& getMapRect() const;
    bool isMapChanged() const;
    const QRect& getPreviousMapRect() const;
    Map* getPreviousMap() const;
    QRectF getCameraRect() const;
    QColor getBackgroundColor() const;
    bool getShowAlive() const;
    bool getShowDead() const;
    bool getShowEffects() const;
    bool getShowMovement() const;
    bool getShowLairActions() const;
    const BattleDialogLogger& getLogger() const;
    BattleDialogModelCombatant* getActiveCombatant() const;
    int getActiveCombatantIndex() const;
    QImage getBackgroundImage() const;

    LayerScene& getLayerScene();
    const LayerScene& getLayerScene() const;

public slots:
    void setMap(Map* map, const QRect& mapRect);
    void setMapRect(const QRect& mapRect);
    void setCameraRect(const QRectF& rect);
    void setBackgroundColor(const QColor& color);
    void setShowAlive(bool showAlive);
    void setShowDead(bool showDead);
    void setShowEffects(bool showEffects);
    void setShowMovement(bool showMovement);
    void setShowLairActions(bool showLairActions);
    void setActiveCombatant(BattleDialogModelCombatant* activeCombatant);
    void setBackgroundImage(QImage backgroundImage);
    void sortCombatants();

signals:
    void mapChanged(Map* map);
    void mapRectChanged(const QRect& mapRect);
    void cameraRectChanged(const QRectF& rect);
    void backgroundColorChanged(const QColor& color);
    void showAliveChanged(bool showAlive);
    void showDeadChanged(bool showDead);
    void showEffectsChanged(bool showEffects);
    void showMovementChanged(bool showMovement);
    void showLairActionsChanged(bool showLairActions);
    void combatantListChanged();
    void effectListChanged();
    void activeCombatantChanged(BattleDialogModelCombatant* activeCombatant);
    void initiativeOrderChanged();
    void backgroundImageChanged(QImage backgroundImage);

protected slots:
    void mapDestroyed(QObject *obj);
    void characterDestroyed(const QUuid& destroyedId);

protected:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;

private:
    static bool CompareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b);

    // Encounter
    EncounterBattle* _encounter;

    // Battle content values
    QList<BattleDialogModelCombatant*> _combatants;
    QList<BattleDialogModelEffect*> _effects;

    // Visualization values
    LayerScene _layerScene;
    Map* _map;
    QRect _mapRect;
    Map* _previousMap;
    QRect _previousMapRect;

    QRectF _cameraRect;

    QColor _background;

    bool _showAlive;
    bool _showDead;
    bool _showEffects;
    bool _showMovement;
    bool _showLairActions;

    BattleDialogModelCombatant* _activeCombatant;

    BattleDialogLogger _logger;

    QImage _backgroundImage;
};

#endif // BATTLEDIALOGMODEL_H
