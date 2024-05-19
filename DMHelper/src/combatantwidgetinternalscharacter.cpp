#include "combatantwidgetinternalscharacter.h"
#include "battledialogmodelcharacter.h"
#include "combatantwidgetcharacter.h"
#include "characterv2.h"
#include <QLineEdit>
#include <QMouseEvent>
#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>
#include <QDebug>

CombatantWidgetInternalsCharacter::CombatantWidgetInternalsCharacter(BattleDialogModelCharacter* character, CombatantWidgetCharacter *parent) :
    CombatantWidgetInternals(parent),
    _widgetParent(parent),
    _character(character)
{
    if(_widgetParent)
        _widgetParent->setInternals(this);
}

BattleDialogModelCombatant* CombatantWidgetInternalsCharacter::getCombatant()
{
    return _character;
}

QFrame* CombatantWidgetInternalsCharacter::getFrame()
{
    return _widgetParent;
}

int CombatantWidgetInternalsCharacter::getInitiative() const
{
    return _character->getInitiative();
}

bool CombatantWidgetInternalsCharacter::isShown()
{
    if(_character)
        return _character->getShown();
    else
        return true;
}

bool CombatantWidgetInternalsCharacter::isKnown()
{
    if(_character)
        return _character->getKnown();
    else
        return true;
}

void CombatantWidgetInternalsCharacter::updateImage()
{
    updateData();
}

void CombatantWidgetInternalsCharacter::updateData()
{
    if(_widgetParent)
        _widgetParent->updateData();
}

void CombatantWidgetInternalsCharacter::setInitiative(int initiative)
{
    Q_UNUSED(initiative);

    if(!_widgetParent || !_character)
        return;

    if(initiative != _character->getInitiative())
        _character->setInitiative(initiative);

    updateData();
}

void CombatantWidgetInternalsCharacter::setHitPoints(int hp)
{
    Q_UNUSED(hp);

    if(!_widgetParent || !_character)
        return;

    if(hp != _character->getHitPoints())
        _character->setHitPoints(hp);

    updateData();
}

void CombatantWidgetInternalsCharacter::executeDoubleClick()
{
    if((_character) && (_character->getCharacter()))
        emit clicked(_character->getCharacter()->getID());
    else
        qDebug() << "[Widget Character Internal] no valid character found!";
}
