#include "battledialogmodelcombatantgroup.h"
#include <QDomDocument>

BattleDialogModelCombatantGroup::BattleDialogModelCombatantGroup(const QString& name, QObject *parent) :
    QObject(parent),
    _id(QUuid::createUuid()),
    _name(name),
    _initiative(0),
    _collapsed(false)
{
}

BattleDialogModelCombatantGroup::BattleDialogModelCombatantGroup(const QUuid& id, const QString& name, int initiative, bool collapsed, QObject *parent) :
    QObject(parent),
    _id(id),
    _name(name),
    _initiative(initiative),
    _collapsed(collapsed)
{
}

BattleDialogModelCombatantGroup::~BattleDialogModelCombatantGroup()
{
}

QUuid BattleDialogModelCombatantGroup::getId() const
{
    return _id;
}

QString BattleDialogModelCombatantGroup::getName() const
{
    return _name;
}

void BattleDialogModelCombatantGroup::setName(const QString& name)
{
    if(_name != name)
    {
        _name = name;
        emit groupChanged();
        emit dirty();
    }
}

int BattleDialogModelCombatantGroup::getInitiative() const
{
    return _initiative;
}

void BattleDialogModelCombatantGroup::setInitiative(int initiative)
{
    if(_initiative != initiative)
    {
        _initiative = initiative;
        emit groupChanged();
        emit dirty();
    }
}

bool BattleDialogModelCombatantGroup::isCollapsed() const
{
    return _collapsed;
}

void BattleDialogModelCombatantGroup::setCollapsed(bool collapsed)
{
    if(_collapsed != collapsed)
    {
        _collapsed = collapsed;
        emit groupChanged();
        emit dirty();
    }
}

QDomElement BattleDialogModelCombatantGroup::createOutputXML(QDomDocument &doc)
{
    QDomElement element = doc.createElement("combatantgroup");
    element.setAttribute("id", _id.toString());
    element.setAttribute("name", _name);
    element.setAttribute("initiative", _initiative);
    element.setAttribute("collapsed", static_cast<int>(_collapsed));
    return element;
}

void BattleDialogModelCombatantGroup::inputXML(const QDomElement &element)
{
    _id = QUuid(element.attribute("id"));
    _name = element.attribute("name");
    _initiative = element.attribute("initiative", QString::number(0)).toInt();
    _collapsed = static_cast<bool>(element.attribute("collapsed", QString::number(0)).toInt());
}
