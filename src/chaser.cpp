#include "chaser.h"
#include "combatant.h"
#include "monster.h"
#include "dmconstants.h"
#include "chaserwidget.h"
#include <QGraphicsTextItem>
#include <QGraphicsPixmapItem>
#include <QTextDocument>

Chaser::Chaser(int position) :
    _position(position)
{
}

Chaser::Chaser(const Chaser &other) :
    _position(other._position)
{
}

Chaser::~Chaser()
{
}

int Chaser::getPosition() const
{
    return _position;
}

void Chaser::setPosition(int position)
{
    _position = position;
}

void Chaser::update()
{
}

ChaserCombatant::ChaserCombatant(QGraphicsTextItem* textItem, QGraphicsPixmapItem* pixmapItem, Combatant* combatant, int position, int pursuer) :
    Chaser(position),
    _dashCount(0),
    _speed(0),
    _exhaustion(0),
    _deficit(0),
    _pursuer(pursuer),
    _textItem(textItem),
    _pixmapItem(pixmapItem),
    _combatant(combatant),
    _widget(NULL)
{
    if(_combatant)
    {
        _dashCount = 3 + Combatant::getAbilityMod(combatant->getConstitution());
        _speed = _combatant->getSpeed();
        if(_combatant->getType() == DMHelper::CombatantType_Monster)
        {
            _combatant->setInitiative(Dice::d20() + Combatant::getAbilityMod(_combatant->getDexterity()));
        }
    }

    if(_textItem)
        _textItem->setY(position);

    if(_pixmapItem)
        _pixmapItem->setY(position);
}

ChaserCombatant::ChaserCombatant(const ChaserCombatant &other) :
    Chaser(other),
    _dashCount(other._dashCount),
    _speed(other._speed),
    _exhaustion(other._exhaustion),
    _deficit(other._deficit),
    _pursuer(other._pursuer),
    _textItem(other._textItem),
    _combatant(other._combatant),
    _widget(other._widget)
{
}

ChaserCombatant::~ChaserCombatant()
{
}

void ChaserCombatant::setPosition(int position)
{
    Chaser::setPosition(position);
}

QString ChaserCombatant::getName() const
{
    if(_combatant)
    {
        QString name = _combatant->getName();
        if(name.isEmpty())
        {
            Monster* monster = dynamic_cast<Monster*>(_combatant);
            if(monster)
            {
                name = monster->getMonsterClassName();
            }
        }

        return name;
    }
    else
    {
        return QString();
    }
}

int ChaserCombatant::getSpeed() const
{
    if(_exhaustion >= 5)
        return 0;
    else if(_exhaustion >= 2)
        return _speed / 2;
    else
        return _speed;
}

void ChaserCombatant::setSpeed(int speed)
{
    _speed = speed;
    refreshWidget();
}

int ChaserCombatant::getInitiative() const
{
    if(_combatant)
        return _combatant->getInitiative();
    else
        return 0;
}

void ChaserCombatant::setInitiative(int initiative)
{
    if(_combatant)
    {
        _combatant->setInitiative(initiative);
        refreshWidget();
    }
}

int ChaserCombatant::getX() const
{
    if(_textItem)
    {
        return _textItem->x() + _textItem->document()->size().width() / 2;
    }
    else
    {
        return -1;
    }
}

int ChaserCombatant::getWidth() const
{
    if(_textItem)
    {
        return _textItem->document()->idealWidth();
    }
    else
    {
        return -1;
    }
}

int ChaserCombatant::getDashCount() const
{
    return _dashCount;
}

void ChaserCombatant::useDash()
{
    setDashCount(_dashCount - 1);
}

void ChaserCombatant::setDashCount(int dashCount)
{
    _dashCount = dashCount;
    refreshWidget();
}

int ChaserCombatant::getExhaustion() const
{
    return _exhaustion;
}

void ChaserCombatant::addExhaustion()
{
    setExhaustion(_exhaustion + 1);
}

void ChaserCombatant::setExhaustion(int exhaustion)
{
    _exhaustion = exhaustion;
    refreshWidget();
}

int ChaserCombatant::getDeficit() const
{
    return _deficit;
}

void ChaserCombatant::reduceDeficit(int delta)
{
    setDeficit(_deficit - delta);
}

void ChaserCombatant::setDeficit(int deficit)
{
    _deficit = deficit;
    refreshWidget();
}

bool ChaserCombatant::isPursuer() const
{
    return _pursuer;
}

void ChaserCombatant::setPursuer(bool pursuer)
{
    _pursuer = pursuer;
    refreshWidget();
}

QGraphicsTextItem* ChaserCombatant::getTextItem() const
{
    return _textItem;
}

void ChaserCombatant::setTextItem(QGraphicsTextItem* textItem)
{
    _textItem = textItem;
    refreshWidget();
}

Combatant* ChaserCombatant::getCombatant() const
{
    return _combatant;
}

void ChaserCombatant::setCombatant(Combatant* combatant)
{
    _combatant = combatant;
    refreshWidget();
}

void ChaserCombatant::update()
{
    if((!_textItem) || (!_pixmapItem))
        return;

    if(_textItem->y() < _position)
    {
        int newPos = qMin((int)_textItem->y() + 3, _position);

        _textItem->setY(newPos);
        _pixmapItem->setY(newPos);

        refreshWidget();
    }
}

ChaserWidget* ChaserCombatant::getWidget() const
{
    return _widget;
}

void ChaserCombatant::setWidget(ChaserWidget* widget)
{
    _widget = widget;
    refreshWidget();
}

void ChaserCombatant::refreshWidget()
{
    if(_widget)
    {
        _widget->readData();
    }
}

ChaserLocalCombatant::ChaserLocalCombatant(QGraphicsTextItem* textItem, Combatant* combatant, int position, int pursuer) :
    ChaserCombatant(textItem, 0, combatant, position, pursuer)
{
}

ChaserLocalCombatant::~ChaserLocalCombatant()
{
    delete _combatant;
}

ChaserComplication::ChaserComplication() :
    Chaser(),
    _textItem(0),
    _data(),
    _active(true)
{
}

ChaserComplication::ChaserComplication(QGraphicsTextItem* textItem, const ComplicationData& data, bool active, int position) :
    Chaser(position),
    _textItem(textItem),
    _data(data),
    _active(active)
{
    _textItem->setFlags(QGraphicsItem::ItemIsSelectable);
    _textItem->setY(position + _data._length);
}

ChaserComplication::ChaserComplication(const ChaserComplication &other) :
    Chaser(other),
    _textItem(other._textItem),
    _data(other._data),
    _active(other._active)
{
}

ChaserComplication::~ChaserComplication()
{
}

void ChaserComplication::setPosition(int position)
{
    Chaser::setPosition(position);
    if(_textItem)
        _textItem->setY(position + _data._length);
}

QString ChaserComplication::getName() const
{
    return _data._title;
}

int ChaserComplication::getSpeed() const
{
    return 0;
}

int ChaserComplication::getInitiative() const
{
    return 0;
}

int ChaserComplication::getX() const
{
    if(_textItem)
    {
        return _textItem->x();
    }
    else
    {
        return -1;
    }
}

int ChaserComplication::getWidth() const
{
    if(_textItem)
    {
        return _textItem->document()->idealWidth();
    }
    else
    {
        return -1;
    }
}

QGraphicsTextItem* ChaserComplication::getTextItem() const
{
    return _textItem;
}

void ChaserComplication::setTextItem(QGraphicsTextItem* textItem)
{
    if(textItem)
    {
        textItem->setFlags(QGraphicsItem::ItemIsSelectable);
    }
    _textItem = textItem;
}

const ComplicationData& ChaserComplication::getData() const
{
    return _data;
}

void ChaserComplication::setComplicationData(const ComplicationData& data)
{
    _data = data;
}

bool ChaserComplication::isActive() const
{
    return _active;
}

void ChaserComplication::setActive(bool active)
{
    _active = active;
    if(_textItem)
    {
        if(_active)
        {
            _textItem->setDefaultTextColor(QColor(Qt::black));
        }
        else
        {
            _textItem->setDefaultTextColor(QColor(Qt::gray));
        }
    }
}
