#include "battledialogmodel.h"
#include "battledialogmodelmonsterbase.h"
#include "dmconstants.h"
#include "campaign.h"
#include "map.h"
#include "grid.h"
#include "layergrid.h"
#include "layerreference.h"
#include "encounterbattle.h"
#include <QDebug>

BattleDialogModel::BattleDialogModel(EncounterBattle* encounter, const QString& name, QObject *parent) :
    CampaignObjectBase(name, parent),
    _encounter(encounter),
    _combatants(),
    _effects(),
    _layerScene(),
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
}

BattleDialogModel::~BattleDialogModel()
{
    qDeleteAll(_combatants);
    qDeleteAll(_effects);
    _layerScene.clearLayers();
}

void BattleDialogModel::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    _background = QColor(element.attribute("backgroundColorR",QString::number(0)).toInt(),
                         element.attribute("backgroundColorG",QString::number(0)).toInt(),
                         element.attribute("backgroundColorB",QString::number(0)).toInt());
    _cameraRect = QRectF(element.attribute("cameraRectX",QString::number(0.0)).toDouble(),
                         element.attribute("cameraRectY",QString::number(0.0)).toDouble(),
                         element.attribute("cameraRectWidth",QString::number(0.0)).toDouble(),
                         element.attribute("cameraRectHeight",QString::number(0.0)).toDouble());
    // TODO: Layers - need this as backwards compability
    /*
    _gridOn = static_cast<bool>(element.attribute("showGrid",QString::number(1)).toInt());
    _gridType = element.attribute("gridType",QString::number(0)).toInt();
    _gridScale = element.attribute("gridScale",QString::number(0)).toInt();
    _gridAngle = element.attribute("gridAngle",QString::number(50)).toInt();
    _gridOffsetX = element.attribute("gridOffsetX",QString::number(0)).toInt();
    _gridOffsetY = element.attribute("gridOffsetY",QString::number(0)).toInt();
    int gridWidth = element.attribute("gridWidth",QString::number(1)).toInt();
    QColor gridColor = element.attribute("gridColor",QString("#000000"));
    _gridPen = QPen(QBrush(gridColor), gridWidth);
    _gridOffsetY = element.attribute("gridOffsetY",QString::number(0)).toInt();
    */
    _showAlive = static_cast<bool>(element.attribute("showAlive",QString::number(1)).toInt());
    _showDead = static_cast<bool>(element.attribute("showDead",QString::number(0)).toInt());
    _showEffects = static_cast<bool>(element.attribute("showEffects",QString::number(1)).toInt());
    _showMovement = static_cast<bool>(element.attribute("showMovement",QString::number(1)).toInt());
    _showLairActions = static_cast<bool>(element.attribute("showLairActions",QString::number(0)).toInt());

    _logger.inputXML(element.firstChildElement("battlelogger"), isImport);

    Campaign* campaign = dynamic_cast<Campaign*>(getParentByType(DMHelper::CampaignType_Campaign));
    if(campaign)
    {
        int mapIdInt = DMH_GLOBAL_INVALID_ID;
        QUuid mapId = parseIdString(element.attribute("mapID"), &mapIdInt);
        Map* battleMap = dynamic_cast<Map*>(campaign->getObjectById(mapId));
        if(battleMap)
        {
            QRect mapRect(element.attribute("mapRectX",QString::number(0)).toInt(),
                          element.attribute("mapRectY",QString::number(0)).toInt(),
                          element.attribute("mapRectWidth",QString::number(0)).toInt(),
                          element.attribute("mapRectHeight",QString::number(0)).toInt());

            setMap(battleMap, mapRect);
        }
    }

    QDomElement layersElement = element.firstChildElement(QString("layerScene"));
    if(!layersElement.isNull())
    {
        _layerScene.inputXML(layersElement, isImport);
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
    }

    _layerScene.postProcessXML(element, isImport);
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

void BattleDialogModel::insertCombatant(int index, BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return;

    _combatants.insert(index, combatant);

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

    emit combatantListChanged();
    emit dirty();
}

BattleDialogModelCombatant* BattleDialogModel::removeCombatant(int index)
{
    BattleDialogModelCombatant* removedCombatant = nullptr;
    if((index >= 0) && (index < _combatants.count()))
    {
        removedCombatant = _combatants.takeAt(index);
        if(_activeCombatant == removedCombatant)
            _activeCombatant = nullptr;
    }

    if((removedCombatant) && (removedCombatant->getCombatantType() == DMHelper::CombatantType_Monster))
    {
        BattleDialogModelMonsterBase* monster = dynamic_cast<BattleDialogModelMonsterBase*>(removedCombatant);
        if(monster)
            disconnect(monster, &BattleDialogModelMonsterBase::imageChanged, this, &BattleDialogModel::combatantListChanged);
    }

    emit combatantListChanged();
    emit dirty();

    return removedCombatant;
}

void BattleDialogModel::appendCombatant(BattleDialogModelCombatant* combatant)
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

    emit combatantListChanged();
    emit dirty();
}

void BattleDialogModel::appendCombatants(QList<BattleDialogModelCombatant*> combatants)
{
    for(BattleDialogModelCombatant* combatant : combatants)
        appendCombatant(combatant);
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

void BattleDialogModel::insertEffect(int index, BattleDialogModelEffect* effect)
{
    if(!effect)
        return;

    _effects.insert(index, effect);
    emit effectListChanged();
    emit dirty();
}

BattleDialogModelEffect* BattleDialogModel::removeEffect(int index)
{
    BattleDialogModelEffect* result = nullptr;
    if((index >= 0) && (index < _effects.count()))
        result = _effects.takeAt(index);

    emit effectListChanged();
    emit dirty();
    return result;
}

bool BattleDialogModel::removeEffect(BattleDialogModelEffect* effect)
{
    if(!effect)
    {
        qDebug() << "[Battle Dialog Model] ERROR: attempted to remove NULL effect!";
        return false;
    }

    bool result = _effects.removeOne(effect);

    if(!result)
        qDebug() << "[Battle Dialog Model] ERROR: Unable to remove effect " << effect;

    emit effectListChanged();
    emit dirty();
    return result;
}

void BattleDialogModel::appendEffect(BattleDialogModelEffect* effect)
{
    if(!effect)
        return;

    _effects.append(effect);
    emit effectListChanged();
    emit dirty();
}

int BattleDialogModel::getGridScale() const
{
    // TODO: Layers - need to make this complete
    if(!_map)
        return 1;

    LayerGrid* layer = dynamic_cast<LayerGrid*>(_map->getLayerScene().getPriority(DMHelper::LayerType_Grid));
    return layer ? layer->getConfig().getGridScale() : DMHelper::STARTING_GRID_SCALE;
}

Map* BattleDialogModel::getMap() const
{
    return _map;
}

const QRect& BattleDialogModel::getMapRect() const
{
    return _mapRect;
}

bool BattleDialogModel::isMapChanged() const
{
    return (_map != _previousMap);
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

    disconnect(_previousMap, &QObject::destroyed, this, &BattleDialogModel::mapDestroyed);
    connect(_map, &QObject::destroyed, this, &BattleDialogModel::mapDestroyed);

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
        emit dirty();
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

void BattleDialogModel::mapDestroyed(QObject *obj)
{
    Q_UNUSED(obj);
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
            removeCombatant(i);
            return;
        }
    }
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

    QDomElement combatantsElement = doc.createElement("combatants");
    for(BattleDialogModelCombatant* combatant : _combatants)
    {
        if(combatant)
            combatant->outputXML(doc, combatantsElement, targetDirectory, isExport);
    }
    element.appendChild(combatantsElement);

    QDomElement effectsElement = doc.createElement("effects");
    for(BattleDialogModelEffect* effect : _effects)
    {
        if(effect)
            effect->outputXML(doc, effectsElement, targetDirectory, isExport);
    }
    element.appendChild(effectsElement);

    // TODO: Layers - need a layer for markers and tokens
    _layerScene.outputXML(doc, element, targetDirectory, isExport);
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
