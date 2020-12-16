#include "widgetcharacterinternal.h"
#include "dmconstants.h"
#include "battledialogmodel.h"
#include "battledialogmodelcharacter.h"
#include "widgetcharacter.h"
#include "character.h"
#include <QLineEdit>
#include <QMouseEvent>
#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>
#include <QDebug>

WidgetCharacterInternal::WidgetCharacterInternal(BattleDialogModelCharacter* character, WidgetCharacter *parent) :
    WidgetCombatantBase(parent),
    _widgetParent(parent),
    _character(character)
{
    if(_widgetParent)
        _widgetParent->setInternals(this);
}

BattleDialogModelCombatant* WidgetCharacterInternal::getCombatant()
{
    return _character;
}

QFrame* WidgetCharacterInternal::getFrame()
{
    return _widgetParent;
}

int WidgetCharacterInternal::getInitiative() const
{
    return _character->getInitiative();
}

bool WidgetCharacterInternal::isShown()
{
    if(_character)
        return _character->getShown();
    else
        return true;
}

bool WidgetCharacterInternal::isKnown()
{
    if(_character)
        return _character->getKnown();
    else
        return true;
}

void WidgetCharacterInternal::updateData()
{
    if(_widgetParent)
        _widgetParent->updateData();
}

void WidgetCharacterInternal::setInitiative(int initiative)
{
    Q_UNUSED(initiative);

    if(!_widgetParent || !_character)
        return;

    if(initiative != _character->getInitiative())
        _character->setInitiative(initiative);

    updateData();
}

void WidgetCharacterInternal::setHitPoints(int hp)
{
    Q_UNUSED(hp);

    if(!_widgetParent || !_character)
        return;

    if(hp != _character->getHitPoints())
        _character->setHitPoints(hp);

    updateData();
}

void WidgetCharacterInternal::executeDoubleClick()
{
    if((_character) && (_character->getCharacter()))
        emit clicked(_character->getCharacter()->getID());
    else
        qDebug() << "[Widget Character Internal] no valid character found!";
}
