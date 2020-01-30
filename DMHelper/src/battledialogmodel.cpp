#include "battledialogmodel.h"
#include "dmconstants.h"
//#include "combatant.h"
#include "map.h"
//#include "encounterbattle.h"
#include <QDebug>

BattleDialogModel::BattleDialogModel(QObject *parent) :
    CampaignObjectBase(parent),
    //_battle(0),
    _combatants(),
    _effects(),
    _map(nullptr),
    _mapRect(),
    _previousMap(nullptr),
    _previousMapRect(),
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
    _backgroundImage()
{
}

BattleDialogModel::BattleDialogModel(const BattleDialogModel& other, QObject *parent) :
    CampaignObjectBase(parent),
    //_battle(other._battle),
    _combatants(),
    _effects(),
    _map(other._map),
    _mapRect(other._mapRect),
    _previousMap(other._previousMap),
    _previousMapRect(other._previousMapRect),
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

BattleDialogModel::~BattleDialogModel()
{
    qDeleteAll(_combatants);
    qDeleteAll(_effects);
}

void BattleDialogModel::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport)
{
    QDomElement battleElement = doc.createElement( "battle" );

    //battleElement.setAttribute("battleID", _battle->getID());
    battleElement.setAttribute("mapID", _map ? _map->getID().toString() : QUuid().toString());
    battleElement.setAttribute("mapRectX", _mapRect.x());
    battleElement.setAttribute("mapRectY", _mapRect.y());
    battleElement.setAttribute("mapRectWidth", _mapRect.width());
    battleElement.setAttribute("mapRectHeight", _mapRect.height());
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

void BattleDialogModel::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    // TODO: Manager needs to set battle
    // TODO: Manager needs to set map and rect
    // TODO: Manager needs to add combatants
    // TODO: Manager needs to set active combatant

    _background= QColor(element.attribute("backgroundColorR",QString::number(0)).toInt(),
                        element.attribute("backgroundColorG",QString::number(0)).toInt(),
                        element.attribute("backgroundColorB",QString::number(0)).toInt());
    _gridOn = static_cast<bool>(element.attribute("showGrid",QString::number(1)).toInt());
    _gridScale = element.attribute("gridScale",QString::number(0)).toInt();
    _gridOffsetX = element.attribute("gridOffsetX",QString::number(0)).toInt();
    _gridOffsetY = element.attribute("gridOffsetY",QString::number(0)).toInt();
    _showCompass = static_cast<bool>(element.attribute("showCompass",QString::number(0)).toInt());
    _showAlive = static_cast<bool>(element.attribute("showAlive",QString::number(1)).toInt());
    _showDead = static_cast<bool>(element.attribute("showDead",QString::number(0)).toInt());
    _showEffects = static_cast<bool>(element.attribute("showEffects",QString::number(1)).toInt());
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

void BattleDialogModel::setMap(Map* map, const QRect& mapRect)
{
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
}

const QRect& BattleDialogModel::getMapRect() const
{
    return _mapRect;
}

void BattleDialogModel::setMapRect(const QRect& mapRect)
{
    _mapRect = mapRect;
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

QColor BattleDialogModel::getBackgroundColor() const
{
    return _background;
}

void BattleDialogModel::setBackgroundColor(QColor color)
{
    _background = color;
}

bool BattleDialogModel::getGridOn() const
{
    return _gridOn;
}

void BattleDialogModel::setGridOn(bool gridOn)
{
    _gridOn = gridOn;
}

int BattleDialogModel::getGridScale() const
{
    return _gridScale;
}

void BattleDialogModel::setGridScale(int gridScale)
{
    _gridScale = gridScale;
}

int BattleDialogModel::getGridOffsetX() const
{
    return _gridOffsetX;
}

void BattleDialogModel::setGridOffsetX(int gridOffsetX)
{
    _gridOffsetX = gridOffsetX;
}

int BattleDialogModel::getGridOffsetY() const
{
    return _gridOffsetY;
}

void BattleDialogModel::setGridOffsetY(int gridOffsetY)
{
    _gridOffsetY = gridOffsetY;
}

bool BattleDialogModel::getShowCompass() const
{
    return _showCompass;
}

void BattleDialogModel::setShowCompass(bool showCompass)
{
    _showCompass = showCompass;
}

bool BattleDialogModel::getShowAlive() const
{
    return _showAlive;
}

void BattleDialogModel::setShowAlive(bool showAlive)
{
    _showAlive = showAlive;
}

bool BattleDialogModel::getShowDead() const
{
    return _showDead;
}

void BattleDialogModel::setShowDead(bool showDead)
{
    _showDead = showDead;
}

bool BattleDialogModel::getShowEffects() const
{
    return _showEffects;
}

void BattleDialogModel::setShowEffects(bool showEffects)
{
    _showEffects = showEffects;
}

BattleDialogModelCombatant* BattleDialogModel::getActiveCombatant() const
{
    return _activeCombatant;
}

void BattleDialogModel::setActiveCombatant(BattleDialogModelCombatant* activeCombatant)
{
    _activeCombatant = activeCombatant;
}

void BattleDialogModel::setBackgroundImage(QImage backgroundImage)
{
    _backgroundImage = backgroundImage;
}

QImage BattleDialogModel::getBackgroundImage() const
{
    return _backgroundImage;
}

void BattleDialogModel::sortCombatants()
{
    std::sort(_combatants.begin(), _combatants.end(), CompareCombatants);
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
