#include "battledialogmodel.h"
#include "battledialogmodelmonsterbase.h"
#include "dmconstants.h"
#include "campaign.h"
#include "map.h"
#include "grid.h"
#include "layergrid.h"
#include "layertokens.h"
#include "layerreference.h"
#include "encounterbattle.h"
#include <QDebug>

BattleDialogModel::BattleDialogModel(EncounterBattle* encounter, const QString& name, QObject *parent) :
    CampaignObjectBase(name, parent),
    _encounter(encounter),
    _combatants(),
    _effects(),
    _layerScene(this),
    _map(nullptr),
    _mapRect(),
    _previousMap(nullptr),
    _previousMapRect(),
    _cameraRect(),
    _background(Qt::black),
    _showAlive(true),
    _showDead(false),
    _showEffects(true),
    _showMovement(true),
    _showLairActions(false),
    _activeCombatant(nullptr),
    _logger(),
    _backgroundImage()
{
    if(_encounter)
        connect(this, &BattleDialogModel::dirty, _encounter, &EncounterBattle::dirty);

    connect(&_layerScene, &LayerScene::dirty, this, &BattleDialogModel::dirty);
    connect(&_layerScene, &LayerScene::layerScaleChanged, this, &BattleDialogModel::handleScaleChanged);
}

BattleDialogModel::~BattleDialogModel()
{
    //qDeleteAll(_combatants);
    qDeleteAll(_effects);
    _layerScene.clearLayers();
}

void BattleDialogModel::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    _background = QColor(element.attribute("backgroundColorR", QString::number(0)).toInt(),
                         element.attribute("backgroundColorG", QString::number(0)).toInt(),
                         element.attribute("backgroundColorB", QString::number(0)).toInt());
    _cameraRect = QRectF(element.attribute("cameraRectX", QString::number(0.0)).toDouble(),
                         element.attribute("cameraRectY", QString::number(0.0)).toDouble(),
                         element.attribute("cameraRectWidth", QString::number(0.0)).toDouble(),
                         element.attribute("cameraRectHeight", QString::number(0.0)).toDouble());
    // TODO: Layers - need this as backwards compability
    int gridScale = element.attribute("gridScale", QString::number(DMHelper::STARTING_GRID_SCALE)).toInt();
    _layerScene.setScale(gridScale);

    _showAlive = static_cast<bool>(element.attribute("showAlive", QString::number(1)).toInt());
    _showDead = static_cast<bool>(element.attribute("showDead", QString::number(0)).toInt());
    _showEffects = static_cast<bool>(element.attribute("showEffects", QString::number(1)).toInt());
    _showMovement = static_cast<bool>(element.attribute("showMovement", QString::number(1)).toInt());
    _showLairActions = static_cast<bool>(element.attribute("showLairActions", QString::number(0)).toInt());

    _logger.inputXML(element.firstChildElement("battlelogger"), isImport);

    Campaign* campaign = dynamic_cast<Campaign*>(getParentByType(DMHelper::CampaignType_Campaign));
    if(campaign)
    {
        int mapIdInt = DMH_GLOBAL_INVALID_ID;
        QUuid mapId = parseIdString(element.attribute("mapID"), &mapIdInt);
        Map* battleMap = dynamic_cast<Map*>(campaign->getObjectById(mapId));
        if(battleMap)
        {
            QRect mapRect(element.attribute("mapRectX", QString::number(0)).toInt(),
                          element.attribute("mapRectY", QString::number(0)).toInt(),
                          element.attribute("mapRectWidth", QString::number(0)).toInt(),
                          element.attribute("mapRectHeight", QString::number(0)).toInt());

            setMap(battleMap, mapRect);
        }
    }

    QDomElement layersElement = element.firstChildElement(QString("layer-scene"));
    if(!layersElement.isNull())
    {
        _layerScene.inputXML(layersElement, isImport);

        // Set the model for any token layers
        for(int i = 0; i < _layerScene.layerCount(); ++i)
        {
            Layer* layer = _layerScene.layerAt(i);
            if((layer) && (layer->getType() == DMHelper::LayerType_Tokens))
            {
                LayerTokens* tokenLayer = dynamic_cast<LayerTokens*>(layer);
                if(tokenLayer)
                    tokenLayer->setModel(this);
            }
        }
        _layerScene.postProcessXML(layersElement, isImport);
    }
    else
    {
        if(_map)
        {
            for(int i = 0; i < _map->getLayerScene().layerCount(); ++i)
            {
                Layer* layer = _map->getLayerScene().layerAt(i);
                if(layer)
                    _layerScene.appendLayer(new LayerReference(_map, layer, layer->getOrder()));
            }
        }

        int gridScale = element.attribute("gridScale", QString::number(DMHelper::STARTING_GRID_SCALE)).toInt();
        _layerScene.setScale(gridScale);

        //use the grid Scale to seed the layer scale, if the grid is on, read the rest and create a grid layer
        LayerGrid* gridLayer = nullptr;
        bool gridOn = static_cast<bool>(element.attribute("showGrid",QString::number(1)).toInt());
        if(gridOn)
        {
            gridLayer = new LayerGrid();
            gridLayer->inputXML(element, isImport);
            gridLayer->setName(QString("grid"));
        }

        LayerTokens* tokenLayer = new LayerTokens(this);
        tokenLayer->inputXML(element, isImport);
        tokenLayer->setName(QString("tokens"));
        tokenLayer->postProcessXML(campaign, element, isImport);

        int fowPosition = _layerScene.getLayerIndex(_layerScene.getFirst(DMHelper::LayerType_Fow));
        if(fowPosition == -1)
        {
            if(gridLayer)
                _layerScene.appendLayer(gridLayer);
            _layerScene.appendLayer(tokenLayer);
        }
        else
        {
            _layerScene.insertLayer(fowPosition, tokenLayer);
            if(gridLayer)
                _layerScene.insertLayer(fowPosition, gridLayer);
        }
    }

    // Todo: Layers - include active ID
    QUuid activeId(element.attribute("activeId"));

    if(!activeId.isNull())
    {
        QList<Layer*> tokenLayers = _layerScene.getLayers(DMHelper::LayerType_Tokens);
        foreach(Layer* layer, tokenLayers)
        {
            LayerTokens* tokenLayer = dynamic_cast<LayerTokens*>(layer);
            if(tokenLayer)
            {
                QList<BattleDialogModelCombatant*> combatants = tokenLayer->getCombatants();
                foreach(BattleDialogModelCombatant* combatant, combatants)
                {
                    if(combatant->getID() == activeId)
                        setActiveCombatant(combatant);
                }
            }
        }
    }

    // Re-establish links to linked items
    QList<Layer*> tokenLayers = _layerScene.getLayers(DMHelper::LayerType_Tokens);
    foreach(Layer* layer, tokenLayers)
    {
        LayerTokens* tokenLayer = dynamic_cast<LayerTokens*>(layer);
        if(tokenLayer)
        {
            QList<BattleDialogModelCombatant*> combatants = tokenLayer->getCombatants();
            foreach(BattleDialogModelCombatant* combatant, combatants)
            {
                if(!combatant->getLinkedID().isNull())
                    combatant->setLinkedObject(tokenLayer->getObjectById(combatant->getLinkedID()));
            }

            QList<BattleDialogModelEffect*> effects = tokenLayer->getEffects();
            foreach(BattleDialogModelEffect* effect, effects)
            {
                if(!effect->getLinkedID().isNull())
                    effect->setLinkedObject(tokenLayer->getObjectById(effect->getLinkedID()));
            }
        }
    }
}

void BattleDialogModel::copyValues(const CampaignObjectBase* other)
{
    const BattleDialogModel* otherModel = dynamic_cast<const BattleDialogModel*>(other);
    if(!otherModel)
        return;

    _background = otherModel->_background;
    _cameraRect = otherModel->_cameraRect;
    _showAlive = otherModel->_showAlive;
    _showDead = otherModel->_showDead;
    _showEffects = otherModel->_showEffects;
    _showMovement = otherModel->_showMovement;
    _showLairActions = otherModel->_showLairActions;

    _logger = otherModel->_logger;

    _layerScene.copyValues(&otherModel->_layerScene);

    CampaignObjectBase::copyValues(other);
}

int BattleDialogModel::getObjectType() const
{
    return DMHelper::CampaignType_BattleContent;
}

const CampaignObjectBase* BattleDialogModel::getParentByType(int parentType) const
{
    if(parentType == DMHelper::CampaignType_BattleContent)
        return this;

    if(!_encounter)
        return nullptr;

    if(parentType == _encounter->getObjectType())
        return _encounter;

    return _encounter->getParentByType(parentType);
}

CampaignObjectBase* BattleDialogModel::getParentByType(int parentType)
{
    if(parentType == DMHelper::CampaignType_BattleContent)
        return this;

    if(!_encounter)
        return nullptr;

    if(parentType == _encounter->getObjectType())
        return _encounter;

    return _encounter->getParentByType(parentType);
}

const CampaignObjectBase* BattleDialogModel::getParentById(const QUuid& id) const
{
    if(id == getID())
        return this;

    if(!_encounter)
        return nullptr;

    if(id == _encounter->getID())
        return _encounter;

    return _encounter->getParentById(id);
}

CampaignObjectBase* BattleDialogModel::getParentById(const QUuid& id)
{
    if(id == getID())
        return this;

    if(!_encounter)
        return nullptr;

    if(id == _encounter->getID())
        return _encounter;

    return _encounter->getParentById(id);
}

QGraphicsItem* BattleDialogModel::getObjectItem(BattleDialogModelObject* object) const
{
    BattleDialogModelCombatant* combatant = dynamic_cast<BattleDialogModelCombatant*>(object);
    BattleDialogModelEffect* effect = dynamic_cast<BattleDialogModelEffect*>(object);

    QList<Layer*> tokenLayers = _layerScene.getLayers(DMHelper::LayerType_Tokens);
    foreach(Layer* layer, tokenLayers)
    {
        LayerTokens* tokenLayer = dynamic_cast<LayerTokens*>(layer);
        if(tokenLayer)
        {
            QGraphicsItem* item = nullptr;
            if(combatant)
                item = tokenLayer->getCombatantItem(combatant);
            else if(effect)
                item = tokenLayer->getEffectItem(effect);

            if(item)
                return item;
        }
    }

    return nullptr;
}

QList<BattleDialogModelCombatant*> BattleDialogModel::getCombatantList() const
{
    return _combatants;
}

int BattleDialogModel::getCombatantCount() const
{
    return _combatants.count();
}

BattleDialogModelCombatant* BattleDialogModel::getCombatant(int index) const
{
    if((index < 0) || (index >= _combatants.count()))
        return nullptr;
    else
        return _combatants.at(index);
}

BattleDialogModelCombatant* BattleDialogModel::getCombatantById(QUuid combatantId) const
{
    for(BattleDialogModelCombatant* combatant : _combatants)
    {
        if((combatant) && (combatant->getCombatant()) && (combatant->getCombatant()->getID() == combatantId))
            return combatant;
    }

    return nullptr;
}

void BattleDialogModel::moveCombatant(int fromIndex, int toIndex)
{
    if((fromIndex < 0) || (fromIndex >= _combatants.size()) || (toIndex < 0) || (toIndex >= _combatants.size()))
    {
        qDebug() << "[Battle Dialog Model] ERROR: move from or to an invalid index! from: " << fromIndex << ", to: " << toIndex;
        return;
    }

    _combatants.move(fromIndex, toIndex);

    emit combatantListChanged();
    emit dirty();
}

void BattleDialogModel::removeCombatant(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return;

    QList<Layer*> tokenLayers = _layerScene.getLayers(DMHelper::LayerType_Tokens);
    for(Layer* layer : tokenLayers)
    {
        LayerTokens* tokenLayer = dynamic_cast<LayerTokens*>(layer);
        if((tokenLayer) && (tokenLayer->containsCombatant(combatant)))
        {
            tokenLayer->removeCombatant(combatant);
            delete combatant;
            return;
        }
    }
}

void BattleDialogModel::appendCombatant(BattleDialogModelCombatant* combatant, LayerTokens* targetLayer)
{
    if(!combatant)
        return;

    LayerTokens* layer = (targetLayer ? targetLayer : dynamic_cast<LayerTokens*>(_layerScene.getPriority(DMHelper::LayerType_Tokens)));
    if(!layer)
        return;

    layer->addCombatant(combatant);
}

void BattleDialogModel::appendCombatantToList(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return;

    _combatants.append(combatant);

    // For a character addition, connect to the destroyed signal
    if((combatant->getCombatantType() == DMHelper::CombatantType_Character) && (combatant->getCombatant()))
        connect(combatant->getCombatant(), &CampaignObjectBase::campaignObjectDestroyed, this, &BattleDialogModel::characterDestroyed);

    // For a monster addition, also react to image changes
    if((combatant->getCombatantType() == DMHelper::CombatantType_Monster))
    {
        BattleDialogModelMonsterBase* monster = dynamic_cast<BattleDialogModelMonsterBase*>(combatant);
        if(monster)
            connect(monster, &BattleDialogModelMonsterBase::imageChanged, this, &BattleDialogModel::combatantListChanged);
    }

    emit combatantAdded(combatant);
    emit combatantListChanged();
    emit dirty();
}

void BattleDialogModel::removeCombatantFromList(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return;

    if(!_combatants.removeOne(combatant))
        return;

    if(combatant->getCombatantType() == DMHelper::CombatantType_Monster)
    {
        BattleDialogModelMonsterBase* monster = dynamic_cast<BattleDialogModelMonsterBase*>(combatant);
        if(monster)
            disconnect(monster, &BattleDialogModelMonsterBase::imageChanged, this, &BattleDialogModel::combatantListChanged);
    }

    emit combatantRemoved(combatant);
    emit combatantListChanged();
    emit dirty();
}

bool BattleDialogModel::isCombatantInList(Combatant* combatant) const
{
    if(!combatant)
        return false;

    if(_combatants.isEmpty())
        return false;

    QListIterator<BattleDialogModelCombatant*> it(_combatants);
    while(it.hasNext())
    {
        BattleDialogModelCombatant* listCombatant = it.next();
        if((listCombatant) && (listCombatant->getCombatant()) && (listCombatant->getCombatant()->getID() == combatant->getID()))
            return true;
    }

    return false;
}

QList<BattleDialogModelEffect*> BattleDialogModel::getEffectList() const
{
    return _effects;
}

int BattleDialogModel::getEffectCount() const
{
    return _effects.count();
}

BattleDialogModelEffect* BattleDialogModel::getEffect(int index) const
{
    if((index < 0) || (index >= _effects.count()))
        return nullptr;
    else
        return _effects.at(index);
}

BattleDialogModelEffect* BattleDialogModel::getEffectById(QUuid effectId) const
{
    for(BattleDialogModelEffect* effect : _effects)
    {
        if((effect) && (effect->getID() == effectId))
            return effect;
    }

    return nullptr;
}

void BattleDialogModel::removeEffect(BattleDialogModelEffect* effect)
{
    if(!effect)
        return;

    QList<Layer*> tokenLayers = _layerScene.getLayers(DMHelper::LayerType_Tokens);
    for(Layer* layer : tokenLayers)
    {
        LayerTokens* tokenLayer = dynamic_cast<LayerTokens*>(layer);
        if((tokenLayer) && (tokenLayer->containsEffect(effect)))
        {
            tokenLayer->removeEffect(effect);
            delete effect;
            return;
        }
    }
}

void BattleDialogModel::appendEffect(BattleDialogModelEffect* effect)
{
    if(!effect)
        return;

    LayerTokens* layer = dynamic_cast<LayerTokens*>(_layerScene.getPriority(DMHelper::LayerType_Tokens));
    if(!layer)
        return;

    layer->addEffect(effect);
}

void BattleDialogModel::appendEffectToList(BattleDialogModelEffect* effect)
{
    if(!effect)
        return;

    _effects.append(effect);

    emit effectListChanged();
    emit dirty();
}

void BattleDialogModel::removeEffectFromList(BattleDialogModelEffect* effect)
{
    if(!effect)
        return;

    if(!_effects.removeOne(effect))
        return;

    emit effectListChanged();
    emit dirty();
}

int BattleDialogModel::getGridScale() const
{
    // TODO: Layers - need to make this complete
    //if(!_map)
    //    return 1;

//    LayerGrid* layer = dynamic_cast<LayerGrid*>(_map->getLayerScene().getPriority(DMHelper::LayerType_Grid));
//    return layer ? layer->getConfig().getGridScale() : DMHelper::STARTING_GRID_SCALE;
    return _layerScene.getScale();
}

Map* BattleDialogModel::getMap() const
{
    return _map;
}

const QRect& BattleDialogModel::getMapRect() const
{
    return _mapRect;
}

const QRect& BattleDialogModel::getPreviousMapRect() const
{
    return _previousMapRect;
}

Map* BattleDialogModel::getPreviousMap() const
{
    return _previousMap;
}

QRectF BattleDialogModel::getCameraRect() const
{
    return _cameraRect;
}

QColor BattleDialogModel::getBackgroundColor() const
{
    return _background;
}

bool BattleDialogModel::getShowAlive() const
{
    return _showAlive;
}

bool BattleDialogModel::getShowDead() const
{
    return _showDead;
}

bool BattleDialogModel::getShowEffects() const
{
    return _showEffects;
}

bool BattleDialogModel::getShowMovement() const
{
    return _showMovement;
}

bool BattleDialogModel::getShowLairActions() const
{
    return _showLairActions;
}

const BattleDialogLogger& BattleDialogModel::getLogger() const
{
    return _logger;
}

BattleDialogModelCombatant* BattleDialogModel::getActiveCombatant() const
{
    return _activeCombatant;
}

int BattleDialogModel::getActiveCombatantIndex() const
{
    return _activeCombatant ? _combatants.indexOf(_activeCombatant) : -1;
}

QImage BattleDialogModel::getBackgroundImage() const
{
    return _backgroundImage;
}

LayerScene& BattleDialogModel::getLayerScene()
{
    return _layerScene;
}

const LayerScene& BattleDialogModel::getLayerScene() const
{
    return _layerScene;
}

void BattleDialogModel::setMap(Map* map, const QRect& mapRect)
{
    if(_map == map)
        return;

    _previousMap = _map;
    _map = map;

    disconnect(_previousMap, &CampaignObjectBase::campaignObjectDestroyed, this, &BattleDialogModel::mapDestroyed);
    connect(_map, &CampaignObjectBase::campaignObjectDestroyed, this, &BattleDialogModel::mapDestroyed);

    _previousMapRect = _mapRect;
    _mapRect = mapRect;

    if((_map) && (_previousMap != _map) && (_combatants.count() > 0))
    {
        _map->initialize();

        // TODO: Layers


        QRect mapRect(QPoint(), _map->getBackgroundImage().size());
        qDebug() << "[Battle Dialog Model] new map set in model: " << _map->getFileName() << " and setting all contents outside the map to the middle.";
        for(BattleDialogModelCombatant* combatant : _combatants)
        {
            if((combatant) && (!mapRect.contains(combatant->getPosition().toPoint())))
                combatant->setPosition(QPointF(_mapRect.x() + _mapRect.width() / 2, _mapRect.y() + _mapRect.height() / 2));
        }
    }

    emit mapChanged(_map);
    emit dirty();
}

void BattleDialogModel::setMapRect(const QRect& mapRect)
{
    if(_mapRect != mapRect)
    {
        _mapRect = mapRect;
        emit mapRectChanged(_mapRect);
        //emit dirty();
    }
}

void BattleDialogModel::setCameraRect(const QRectF& rect)
{
    if(_cameraRect != rect)
    {
        _cameraRect = rect;
        emit cameraRectChanged(_cameraRect);
        emit dirty();
    }
}

void BattleDialogModel::setBackgroundColor(const QColor& color)
{
    if(_background != color)
    {
        _background = color;
        emit backgroundColorChanged(color);
        emit dirty();
    }
}

void BattleDialogModel::setShowAlive(bool showAlive)
{
    if(_showAlive != showAlive)
    {
        _showAlive = showAlive;
        emit showAliveChanged(_showAlive);
        emit dirty();
    }
}

void BattleDialogModel::setShowDead(bool showDead)
{
    if(_showDead != showDead)
    {
        _showDead = showDead;
        emit showDeadChanged(_showDead);
        emit dirty();
    }
}

void BattleDialogModel::setShowEffects(bool showEffects)
{
    if(_showEffects != showEffects)
    {
        _showEffects = showEffects;
        emit showEffectsChanged(_showEffects);
        emit dirty();
    }
}

void BattleDialogModel::setShowMovement(bool showMovement)
{
    if(_showMovement != showMovement)
    {
        _showMovement = showMovement;
        emit showMovementChanged(_showMovement);
        emit dirty();
    }
}

void BattleDialogModel::setShowLairActions(bool showLairActions)
{
    if(_showLairActions != showLairActions)
    {
        _showLairActions = showLairActions;
        emit showLairActionsChanged(_showLairActions);
        emit dirty();
    }
}

void BattleDialogModel::setActiveCombatant(BattleDialogModelCombatant* activeCombatant)
{
    if(_activeCombatant != activeCombatant)
    {
        _activeCombatant = activeCombatant;
        emit activeCombatantChanged(_activeCombatant);
        emit dirty();
    }
}

void BattleDialogModel::setBackgroundImage(QImage backgroundImage)
{
    if(_backgroundImage != backgroundImage)
    {
        _backgroundImage = backgroundImage;
        emit backgroundImageChanged(_backgroundImage);
        emit dirty();
    }
}

void BattleDialogModel::sortCombatants()
{
    std::sort(_combatants.begin(), _combatants.end(), CompareCombatants);
    emit initiativeOrderChanged();
    emit dirty();
}

void BattleDialogModel::mapDestroyed(const QUuid& id)
{
    Q_UNUSED(id);
    // TODO: Layers
    setMap(nullptr, QRect());
}

void BattleDialogModel::characterDestroyed(const QUuid& destroyedId)
{
    for(int i = 0; i < _combatants.count(); ++i)
    {
        BattleDialogModelCombatant* combatant = _combatants.at(i);
        if((combatant) &&
           (combatant->getCombatantType() == DMHelper::CombatantType_Character) &&
           (combatant->getCombatant()) &&
           (combatant->getCombatant()->getID() == destroyedId))
        {
            removeCombatant(combatant);
            return;
        }
    }
}

void BattleDialogModel::handleScaleChanged(Layer* layer)
{
    if(!layer)
        return;

    Layer* selectedLayer = _layerScene.getSelectedLayer();
    if(!selectedLayer)
        return;

    Layer* nearestLayer = _layerScene.getNearest(selectedLayer, DMHelper::LayerType_Grid);
    if(nearestLayer != layer)
        return;

    LayerGrid* gridLayer = dynamic_cast<LayerGrid*>(nearestLayer);
    if(gridLayer)
        emit gridScaleChanged(gridLayer->getConfig());
}

QDomElement BattleDialogModel::createOutputXML(QDomDocument &doc)
{
    return doc.createElement("battle");
}

void BattleDialogModel::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("mapID", _map ? _map->getID().toString() : QUuid().toString());
    element.setAttribute("mapRectX", _mapRect.x());
    element.setAttribute("mapRectY", _mapRect.y());
    element.setAttribute("mapRectWidth", _mapRect.width());
    element.setAttribute("mapRectHeight", _mapRect.height());
    element.setAttribute("cameraRectX", _cameraRect.x());
    element.setAttribute("cameraRectY", _cameraRect.y());
    element.setAttribute("cameraRectWidth", _cameraRect.width());
    element.setAttribute("cameraRectHeight", _cameraRect.height());
    element.setAttribute("backgroundColorR", _background.red());
    element.setAttribute("backgroundColorG", _background.green());
    element.setAttribute("backgroundColorB", _background.blue());
    element.setAttribute("background", _mapRect.height());
    element.setAttribute("showAlive", _showAlive);
    element.setAttribute("showDead", _showDead);
    element.setAttribute("showEffects", _showEffects);
    element.setAttribute("showMovement", _showMovement);
    element.setAttribute("showLairActions", _showLairActions);
    element.setAttribute("activeId", _activeCombatant ? _activeCombatant->getID().toString() : QUuid().toString());

    _logger.outputXML(doc, element, targetDirectory, isExport);

    CampaignObjectBase::internalOutputXML(doc, element, targetDirectory, isExport);
}

bool BattleDialogModel::belongsToObject(QDomElement& element)
{
    Q_UNUSED(element);

    // Don't auto-input any child objects of the battle. The battle will handle this itself.
    return true;
}

bool BattleDialogModel::CompareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b)
{
    if((!a)||(!b))
        return false;

    if(a->getInitiative() == b->getInitiative())
    {
        return a->getDexterity() > b->getDexterity();
    }
    else
    {
        return a->getInitiative() > b->getInitiative();
    }
}
