#include "monsteraction.h"
#include "monsterclassv2.h"
#include <QDomDocument>
#include <QDomElement>

MonsterAction::MonsterAction(int attackBonus, const QString& description, const QString& name, Dice damageDice) :
    _attackBonus(attackBonus),
    _description(description),
    _name(name),
    _damageDice(damageDice)
{
}

MonsterAction::MonsterAction(const QDomElement &element, bool isImport) :
    _attackBonus(0),
    _description(QString()),
    _name(QString()),
    _damageDice(Dice())
{
    Q_UNUSED(isImport);

    _attackBonus = element.firstChildElement(QString("attack_bonus")).text().toInt();
    _description = element.firstChildElement(QString("desc")).text();
    _name = element.firstChildElement(QString("name")).text();
    Dice inputDice = Dice(element.firstChildElement(QString("damage_dice")).text());
    _damageDice = Dice(inputDice.getCount(), inputDice.getType(), element.firstChildElement(QString("damage_bonus")).text().toInt());
}

MonsterAction::MonsterAction(const MonsterAction& other) :
    _attackBonus(other._attackBonus),
    _description(other._description),
    _name(other._name),
    _damageDice(other._damageDice)
{
}

MonsterAction::~MonsterAction()
{
}

QDomElement MonsterAction::outputXML(QDomDocument &doc, QDomElement &element, bool isExport) const
{
    MonsterClass::outputValue(doc, element, isExport, QString("attack_bonus"), QString::number(getAttackBonus()));
    MonsterClass::outputValue(doc, element, isExport, QString("desc"), getDescription());
    MonsterClass::outputValue(doc, element, isExport, QString("name"), getName());
    MonsterClass::outputValue(doc, element, isExport, QString("damage_bonus"), QString::number(getDamageDice().getBonus()));
    Dice outputDice = Dice(getDamageDice().getCount(), getDamageDice().getType(), 0);
    MonsterClass::outputValue(doc, element, isExport, QString("damage_dice"), outputDice.toString());

    return element;
}

bool MonsterAction::hasDiceSummary() const
{
    return((_damageDice.getBonus() != 0) || ((_damageDice.getCount() > 0) && (_damageDice.getType() > 0)));
}

QString MonsterAction::summaryString() const
{
    QString result = _name;

    if(hasDiceSummary())
    {
        result.append(QString(" ("));
        if(_attackBonus >= 0)
            result.append(QString("+"));
        result.append(QString::number(_attackBonus));
        if((_damageDice.getCount() > 0) && (_damageDice.getType() > 0))
        {
            result.append(QString(" / "));
            result.append(_damageDice.toString());
        }
        result.append(QString(")"));
    }

    return result;
}

int MonsterAction::getAttackBonus() const
{
    return _attackBonus;
}

void MonsterAction::setAttackBonus(int attackBonus)
{
    _attackBonus = attackBonus;
}

QString MonsterAction::getDescription() const
{
    return _description;
}

void MonsterAction::setDescription(const QString& description)
{
    _description = description;
}

QString MonsterAction::getName() const
{
    return _name;
}

void MonsterAction::setName(const QString& actionName)
{
    _name = actionName;
}

Dice MonsterAction::getDamageDice() const
{
    return _damageDice;
}

void MonsterAction::setDamageDice(Dice damageDice)
{
    _damageDice = damageDice;
}

MonsterAction &MonsterAction::operator=(const MonsterAction &other)
{
    _attackBonus = other._attackBonus;
    _description = other._description;
    _name = other._name;
    _damageDice = other._damageDice;

    return *this;
}

bool MonsterAction::operator==(const MonsterAction &other) const
{
    return((_attackBonus == other._attackBonus) &&
           (_description == other._description) &&
           (_name == other._name) &&
           (_damageDice == other._damageDice));
}

bool MonsterAction::operator!=(const MonsterAction &other) const
{
    return !(*this == other);
}
