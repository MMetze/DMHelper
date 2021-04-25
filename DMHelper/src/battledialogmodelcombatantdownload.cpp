#include "battledialogmodelcombatantdownload.h"
#include <QDomElement>
#include <QDebug>

BattleDialogModelCombatantDownload::BattleDialogModelCombatantDownload(QObject *parent) :
    BattleDialogModelCombatant(QString(), parent),
    _combatantId(),
    _sizeFactor(1.0),
    _conditions(0),
    _speed(30)
{
}

void BattleDialogModelCombatantDownload::inputXML(const QDomElement &element, bool isImport)
{
    // Ignored
    // element.setAttribute("type", getCombatantType());

    // element.setAttribute("combatantId", getCombatant() ? getCombatant()->getID().toString() : QUuid().toString());
    _combatantId = QUuid(element.attribute("combatantId", QUuid().toString()));
    // element.setAttribute("sizeFactor", getSizeFactor());
    _sizeFactor = element.attribute("sizeFactor", QString::number(0)).toInt();
    // element.setAttribute("conditions", getConditions());
    setConditions(element.attribute("conditions", QString::number(0)).toInt());
    // element.setAttribute("name", getName());
    setName(element.attribute("name"));

    // element.setAttribute("initiative", _initiative);
    // element.setAttribute("positionX", _position.x());
    // element.setAttribute("positionY", _position.y());
    // element.setAttribute("isShown", _isShown);
    /*
     Covers
     _initiative = element.attribute("initiative",QString::number(0)).toInt();
     _position = QPointF(element.attribute("positionX",QString::number(0)).toDouble(),
                         element.attribute("positionY",QString::number(0)).toDouble());
     _isShown = static_cast<bool>(element.attribute("isShown",QString::number(1)).toInt());
     _isKnown = static_cast<bool>(element.attribute("isKnown",QString::number(1)).toInt());
     */
    BattleDialogModelCombatant::inputXML(element, true);
}

int BattleDialogModelCombatantDownload::getCombatantType() const
{
    return DMHelper::CombatantType_Download;
}

BattleDialogModelCombatant* BattleDialogModelCombatantDownload::clone() const
{
    return nullptr;
}

qreal BattleDialogModelCombatantDownload::getSizeFactor() const
{
    return _sizeFactor;
}

int BattleDialogModelCombatantDownload::getSizeCategory() const
{
    return DMHelper::CombatantSize_Unknown;
}

int BattleDialogModelCombatantDownload::getStrength() const
{
    return 10;
}

int BattleDialogModelCombatantDownload::getDexterity() const
{
    return 10;
}

int BattleDialogModelCombatantDownload::getConstitution() const
{
    return 10;
}

int BattleDialogModelCombatantDownload::getIntelligence() const
{
    return 10;
}

int BattleDialogModelCombatantDownload::getWisdom() const
{
    return 10;
}

int BattleDialogModelCombatantDownload::getCharisma() const
{
    return 10;
}

int BattleDialogModelCombatantDownload::getSkillModifier(Combatant::Skills skill) const
{
    Q_UNUSED(skill);
    return 0;
}

int BattleDialogModelCombatantDownload::getConditions() const
{
    return _conditions;
}

bool BattleDialogModelCombatantDownload::hasCondition(Combatant::Condition condition) const
{
    return ((_conditions & condition) != 0);
}

int BattleDialogModelCombatantDownload::getSpeed() const
{
    return _speed;
}

int BattleDialogModelCombatantDownload::getArmorClass() const
{
    return 10;
}

int BattleDialogModelCombatantDownload::getHitPoints() const
{
    return 1;
}

void BattleDialogModelCombatantDownload::setHitPoints(int hitPoints)
{
    Q_UNUSED(hitPoints);
}

QPixmap BattleDialogModelCombatantDownload::getIconPixmap(DMHelper::PixmapSize iconSize) const
{
    // TODO: need this?
    Q_UNUSED(iconSize);
    return QPixmap();
}

void BattleDialogModelCombatantDownload::setConditions(int conditions)
{
    if(_conditions != conditions)
        _conditions = conditions;
}

void BattleDialogModelCombatantDownload::applyConditions(int conditions)
{
    if((_conditions & conditions) != conditions)
        _conditions |= conditions;
}

void BattleDialogModelCombatantDownload::removeConditions(int conditions)
{
    if((_conditions & ~conditions) != _conditions)
        _conditions &= ~conditions;
}
