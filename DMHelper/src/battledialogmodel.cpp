#include "battledialogmodel.h"
#include "dmconstants.h"
#include "map.h"
#include "grid.h"
#include <QDebug>

BattleDialogModel::BattleDialogModel(const QString& name, QObject *parent) :
    CampaignObjectBase(name, parent),
    _combatants(),
    _effects(),
    _map(nullptr),
    _mapRect(),
    _previousMap(nullptr),
    _previousMapRect(),
    _cameraRect(),
    _background(Qt::black),
    _gridOn(true),
    _gridType(Grid::GridType_Square),
    _gridScale(DMHelper::STARTING_GRID_SCALE),
    _gridAngle(50),
    _gridOffsetX(0),
    _gridOffsetY(0),
    _showCompass(false),
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
    _gridOn = static_cast<bool>(element.attribute("showGrid",QString::number(1)).toInt());
    _gridType = element.attribute("gridType",QString::number(0)).toInt();
    _gridScale = element.attribute("gridScale",QString::number(0)).toInt();
    _gridAngle = element.attribute("gridAngle",QString::number(50)).toInt();
    _gridOffsetX = element.attribute("gridOffsetX",QString::number(0)).toInt();
    _gridOffsetY = element.attribute("gridOffsetY",QString::number(0)).toInt();
    // TODO: possibly re-enable compass at some point
    //_showCompass = static_cast<bool>(element.attribute("showCompass",QString::number(0)).toInt());
    _showCompass = false;
    _showAlive = static_cast<bool>(element.attribute("showAlive",QString::number(1)).toInt());
    _showDead = static_cast<bool>(element.attribute("showDead",QString::number(0)).toInt());
    _showEffects = static_cast<bool>(element.attribute("showEffects",QString::number(1)).toInt());
    _showMovement = static_cast<bool>(element.attribute("showMovement",QString::number(1)).toInt());
    _showLairActions = static_cast<bool>(element.attribute("showLairActions",QString::number(0)).toInt());

    _logger.inputXML(element.firstChildElement("battlelogger"), isImport);
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
    emit combatantListChanged();
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

    emit combatantListChanged();

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

    emit combatantListChanged();
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
}

BattleDialogModelEffect* BattleDialogModel::removeEffect(int index)
{
    BattleDialogModelEffect* result = nullptr;
    if((index >= 0) && (index < _effects.count()))
        result = _effects.takeAt(index);

    emit effectListChanged();
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
    return result;
}

void BattleDialogModel::appendEffect(BattleDialogModelEffect* effect)
{
    if(!effect)
        return;

    _effects.append(effect);
    emit effectListChanged();
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

bool BattleDialogModel::getGridOn() const
{
    return _gridOn;
}

int BattleDialogModel::getGridType() const
{
    return _gridType;
}

int BattleDialogModel::getGridScale() const
{
    return _gridScale;
}

int BattleDialogModel::getGridAngle() const
{
    return _gridAngle;
}

int BattleDialogModel::getGridOffsetX() const
{
    return _gridOffsetX;
}

int BattleDialogModel::getGridOffsetY() const
{
    return _gridOffsetY;
}

bool BattleDialogModel::getShowCompass() const
{
    return _showCompass;
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

    if((_map) && (_previousMap != _map))
    {
        qDebug() << "[Battle Dialog Model] new map set in model: " << _map->getFileName() << " and setting all contents to the middle.";
        for(BattleDialogModelCombatant* combatant : _combatants)
        {
            if(combatant)
            {
                combatant->setPosition(QPointF(_mapRect.x() + _mapRect.width() / 2, _mapRect.y() + _mapRect.height() / 2));
            }
        }
    }

    emit mapChanged(_map);
}

void BattleDialogModel::setMapRect(const QRect& mapRect)
{
    if(_mapRect != mapRect)
    {
        _mapRect = mapRect;
        emit mapRectChanged(_mapRect);
    }
}

void BattleDialogModel::setCameraRect(const QRectF& rect)
{
    if(_cameraRect != rect)
    {
        _cameraRect = rect;
        emit cameraRectChanged(_cameraRect);
    }
}

void BattleDialogModel::setBackgroundColor(const QColor& color)
{
    if(_background != color)
    {
        _background = color;
        emit backgroundColorChanged(color);
    }
}

void BattleDialogModel::setGridOn(bool gridOn)
{
    if(_gridOn != gridOn)
    {
        _gridOn = gridOn;
        emit gridOnChanged(_gridOn);
    }
}

void BattleDialogModel::setGridType(int gridType)
{
    if(_gridType != gridType)
    {
        _gridType = gridType;
        emit gridTypeChanged(_gridType);
    }
}

void BattleDialogModel::setGridScale(int gridScale)
{
    if(_gridScale != gridScale)
    {
        _gridScale = gridScale;
        emit gridScaleChanged(_gridScale);
    }
}

void BattleDialogModel::setGridAngle(int gridAngle)
{
    if(_gridAngle != gridAngle)
    {
        _gridAngle = gridAngle;
        emit gridAngleChanged(_gridAngle);
    }
}

void BattleDialogModel::setGridOffsetX(int gridOffsetX)
{
    if(_gridOffsetX != gridOffsetX)
    {
        _gridOffsetX = gridOffsetX;
        emit gridOffsetXChanged(_gridOffsetX);
    }
}

void BattleDialogModel::setGridOffsetY(int gridOffsetY)
{
    if(_gridOffsetY != gridOffsetY)
    {
        _gridOffsetY = gridOffsetY;
        emit gridOffsetYChanged(_gridOffsetY);
    }
}

void BattleDialogModel::setShowCompass(bool showCompass)
{
    if(_showCompass != showCompass)
    {
        _showCompass = showCompass;
        emit showCompassChanged(_showCompass);
    }
}

void BattleDialogModel::setShowAlive(bool showAlive)
{
    if(_showAlive != showAlive)
    {
        _showAlive = showAlive;
        emit showAliveChanged(_showAlive);
    }
}

void BattleDialogModel::setShowDead(bool showDead)
{
    if(_showDead != showDead)
    {
        _showDead = showDead;
        emit showDeadChanged(_showDead);
    }
}

void BattleDialogModel::setShowEffects(bool showEffects)
{
    if(_showEffects != showEffects)
    {
        _showEffects = showEffects;
        emit showEffectsChanged(_showEffects);
    }
}

void BattleDialogModel::setShowMovement(bool showMovement)
{
    if(_showMovement != showMovement)
    {
        _showMovement = showMovement;
        emit showMovementChanged(_showMovement);
    }
}

void BattleDialogModel::setShowLairActions(bool showLairActions)
{
    if(_showLairActions != showLairActions)
    {
        _showLairActions = showLairActions;
        emit showLairActionsChanged(_showLairActions);
    }
}

void BattleDialogModel::setActiveCombatant(BattleDialogModelCombatant* activeCombatant)
{
    if(_activeCombatant != activeCombatant)
    {
        _activeCombatant = activeCombatant;
        emit activeCombatantChanged(_activeCombatant);
    }
}

void BattleDialogModel::setBackgroundImage(QImage backgroundImage)
{
    if(_backgroundImage != backgroundImage)
    {
        _backgroundImage = backgroundImage;
        emit backgroundImageChanged(_backgroundImage);
    }
}

void BattleDialogModel::sortCombatants()
{
    std::sort(_combatants.begin(), _combatants.end(), CompareCombatants);
}

void BattleDialogModel::mapDestroyed(QObject *obj)
{
    Q_UNUSED(obj);
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
    element.setAttribute("showGrid", _gridOn);
    element.setAttribute("gridType", _gridType);
    element.setAttribute("gridScale", _gridScale);
    element.setAttribute("gridAngle", _gridAngle);
    element.setAttribute("gridOffsetX", _gridOffsetX);
    element.setAttribute("gridOffsetY", _gridOffsetY);
    element.setAttribute("showCompass", _showCompass);
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
