#include "battledialogmodel.h"
#include "dmconstants.h"
//#include "combatant.h"
#include "map.h"
//#include "encounterbattle.h"
#include <QDebug>

BattleDialogModel::BattleDialogModel(const QString& name, QObject *parent) :
    CampaignObjectBase(name, parent),
    //_battle(0),
    _combatants(),
    _effects(),
    _map(nullptr),
    _mapRect(),
    _previousMap(nullptr),
    _previousMapRect(),
    _cameraRect(),
    _background(Qt::black),
    _gridOn(true),
    _gridScale(DMHelper::STARTING_GRID_SCALE),
    _gridOffsetX(0),
    _gridOffsetY(0),
    _showCompass(false),
    _showAlive(true),
    _showDead(false),
    _showEffects(true),
    _activeCombatant(nullptr),
    _logger(),
    _backgroundImage()
{
}

/*
BattleDialogModel::BattleDialogModel(const BattleDialogModel& other, QObject *parent) :
    CampaignObjectBase(parent),
    //_battle(other._battle),
    _combatants(),
    _effects(),
    _map(other._map),
    _mapRect(other._mapRect),
    _previousMap(other._previousMap),
    _previousMapRect(other._previousMapRect),
    _cameraRect(other._cameraRect),
    _background(other._background),
    _gridOn(other._gridOn),
    _gridScale(other._gridScale),
    _gridOffsetX(other._gridOffsetX),
    _gridOffsetY(other._gridOffsetY),
    _showCompass(other._showCompass),
    _showAlive(other._showAlive),
    _showDead(other._showDead),
    _showEffects(other._showEffects),
    _activeCombatant(nullptr),
    _logger(other._logger),
    _backgroundImage(other._backgroundImage)
{
    for(int i = 0; i < other._combatants.count(); ++i)
    {
        if(other._combatants.at(i))
        {
            BattleDialogModelCombatant* newCombatant = other._combatants.at(i)->clone();
            _combatants.append(newCombatant);
            if(other._combatants.at(i) == other._activeCombatant)
            {
                _activeCombatant = newCombatant;
            }
        }
    }

    for(int i = 0; i < other._effects.count(); ++i)
    {
        if(other._effects.at(i))
        {
            BattleDialogModelEffect* newEffect = other._effects.at(i)->clone();
            _effects.append(newEffect);
        }
    }
}
*/

BattleDialogModel::~BattleDialogModel()
{
    qDeleteAll(_combatants);
    qDeleteAll(_effects);
}

/*
void BattleDialogModel::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport)
{
    QDomElement battleElement = doc.createElement( "battle" );

    //battleElement.setAttribute("battleID", _battle->getID());
    battleElement.setAttribute("mapID", _map ? _map->getID().toString() : QUuid().toString());
    battleElement.setAttribute("mapRectX", _mapRect.x());
    battleElement.setAttribute("mapRectY", _mapRect.y());
    battleElement.setAttribute("mapRectWidth", _mapRect.width());
    battleElement.setAttribute("mapRectHeight", _mapRect.height());
    battleElement.setAttribute("cameraRectX", _cameraRect.x());
    battleElement.setAttribute("cameraRectY", _cameraRect.y());
    battleElement.setAttribute("cameraRectWidth", _cameraRect.width());
    battleElement.setAttribute("cameraRectHeight", _cameraRect.height());
    battleElement.setAttribute("backgroundColorR", _background.red());
    battleElement.setAttribute("backgroundColorG", _background.green());
    battleElement.setAttribute("backgroundColorB", _background.blue());
    battleElement.setAttribute("background", _mapRect.height());
    battleElement.setAttribute("showGrid", _gridOn);
    battleElement.setAttribute("gridScale", _gridScale);
    battleElement.setAttribute("gridOffsetX", _gridOffsetX);
    battleElement.setAttribute("gridOffsetY", _gridOffsetY);
    battleElement.setAttribute("showCompass", _showCompass);
    battleElement.setAttribute("showAlive", _showAlive);
    battleElement.setAttribute("showDead", _showDead);
    battleElement.setAttribute("showEffects", _showEffects);
    battleElement.setAttribute("activeId", _activeCombatant ? _activeCombatant->getID().toString() : QUuid().toString());

    _logger.outputXML(doc, battleElement, targetDirectory, isExport);

    QDomElement combatantsElement = doc.createElement("combatants");
    for(BattleDialogModelCombatant* combatant : _combatants)
    {
        if(combatant)
            combatant->outputXML(doc, combatantsElement, targetDirectory, isExport);
    }
    battleElement.appendChild(combatantsElement);

    QDomElement effectsElement = doc.createElement("effects");
    for(BattleDialogModelEffect* effect : _effects)
    {
        if(effect)
            effect->outputXML(doc, effectsElement, targetDirectory, isExport);
    }
    battleElement.appendChild(effectsElement);

    parent.appendChild(battleElement);

}
*/

void BattleDialogModel::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    // TODO: Manager needs to set battle
    // TODO: Manager needs to set map and rect
    // TODO: Manager needs to add combatants
    // TODO: Manager needs to set active combatant

    _background = QColor(element.attribute("backgroundColorR",QString::number(0)).toInt(),
                         element.attribute("backgroundColorG",QString::number(0)).toInt(),
                         element.attribute("backgroundColorB",QString::number(0)).toInt());
    _cameraRect = QRectF(element.attribute("cameraRectX",QString::number(0.0)).toDouble(),
                         element.attribute("cameraRectY",QString::number(0.0)).toDouble(),
                         element.attribute("cameraRectWidth",QString::number(0.0)).toDouble(),
                         element.attribute("cameraRectHeight",QString::number(0.0)).toDouble());
    _gridOn = static_cast<bool>(element.attribute("showGrid",QString::number(1)).toInt());
    _gridScale = element.attribute("gridScale",QString::number(0)).toInt();
    _gridOffsetX = element.attribute("gridOffsetX",QString::number(0)).toInt();
    _gridOffsetY = element.attribute("gridOffsetY",QString::number(0)).toInt();
    _showCompass = static_cast<bool>(element.attribute("showCompass",QString::number(0)).toInt());
    _showAlive = static_cast<bool>(element.attribute("showAlive",QString::number(1)).toInt());
    _showDead = static_cast<bool>(element.attribute("showDead",QString::number(0)).toInt());
    _showEffects = static_cast<bool>(element.attribute("showEffects",QString::number(1)).toInt());

    _logger.inputXML(element.firstChildElement("battlelogger"), isImport);
}

/*
EncounterBattle* BattleDialogModel::getBattle() const
{
    return _battle;
}

void BattleDialogModel::setBattle(EncounterBattle* battle)
{
    // TODO: Check whether any combatants should be deleted if required??
    _battle = battle;
}
*/

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
    _combatants.insert(index, combatant);
}

BattleDialogModelCombatant* BattleDialogModel::removeCombatant(int index)
{
    if((index < 0) || (index >= _combatants.count()))
        return nullptr;
    else
        return _combatants.takeAt(index);
}

void BattleDialogModel::appendCombatant(BattleDialogModelCombatant* combatant)
{
    _combatants.append(combatant);
}

void BattleDialogModel::appendCombatants(QList<BattleDialogModelCombatant*> combatants)
{
    _combatants.append(combatants);
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
    _effects.insert(index, effect);
}

BattleDialogModelEffect* BattleDialogModel::removeEffect(int index)
{
    if((index < 0) || (index >= _effects.count()))
        return nullptr;
    else
        return _effects.takeAt(index);
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
    {
        qDebug() << "[Battle Dialog Model] ERROR: Unable to remove effect " << effect;
    }

    return result;
}

void BattleDialogModel::appendEffect(BattleDialogModelEffect* effect)
{
    _effects.append(effect);
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

int BattleDialogModel::getGridScale() const
{
    return _gridScale;
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

const BattleDialogLogger& BattleDialogModel::getLogger() const
{
    return _logger;
}

BattleDialogModelCombatant* BattleDialogModel::getActiveCombatant() const
{
    return _activeCombatant;
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

void BattleDialogModel::setBackgroundColor(QColor color)
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

void BattleDialogModel::setGridScale(int gridScale)
{
    if(_gridScale != gridScale)
    {
        _gridScale = gridScale;
        emit gridScaleChanged(_gridScale);
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
    element.setAttribute("gridScale", _gridScale);
    element.setAttribute("gridOffsetX", _gridOffsetX);
    element.setAttribute("gridOffsetY", _gridOffsetY);
    element.setAttribute("showCompass", _showCompass);
    element.setAttribute("showAlive", _showAlive);
    element.setAttribute("showDead", _showDead);
    element.setAttribute("showEffects", _showEffects);
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
