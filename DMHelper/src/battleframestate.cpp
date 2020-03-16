#include "battleframestate.h"
#include "dmconstants.h"
#include <QPixmap>
#include <QDebug>

BattleFrameState::BattleFrameState(int stateId, BattleFrameStateType stateType, QObject *parent) :
    QObject(parent),
    _stateId(stateId),
    _stateType(stateType),
    _cursor()
{
}

BattleFrameState::BattleFrameState(int stateId, BattleFrameStateType stateType, const QPixmap& cursorPixmap, int hotX, int hotY, QObject *parent) :
    QObject(parent),
    _stateId(stateId),
    _stateType(stateType),
    _cursor(cursorPixmap.scaled(DMHelper::CURSOR_SIZE, DMHelper::CURSOR_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation),
            hotX * DMHelper::CURSOR_SIZE / cursorPixmap.width(),
            hotY * DMHelper::CURSOR_SIZE / cursorPixmap.height())
{
}

BattleFrameState::~BattleFrameState()
{
}

bool BattleFrameState::isValid() const
{
    return _stateType != BattleFrameStateType_Invalid;
}

int BattleFrameState::getId() const
{
    return _stateId;
}

BattleFrameState::BattleFrameStateType BattleFrameState::getType() const
{
    return _stateType;
}

const QCursor& BattleFrameState::getCursor() const
{
    return _cursor;
}

void BattleFrameState::activate()
{
    emit activateState(getId());
}

void BattleFrameState::deactivate()
{
    emit deactivateState(getId());
}

void BattleFrameState::toggle()
{
    emit toggleState(getId());
}

void BattleFrameState::enter()
{
    qDebug() << "[BattleFrameState] State entered: " << getId();
    emit stateChanged(true);
}

void BattleFrameState::leave()
{
    qDebug() << "[BattleFrameState] State left: " << getId();
    emit stateChanged(false);
}

void BattleFrameState::setCursor(const QCursor& cursor)
{
    if(_cursor != cursor)
    {
        _cursor = cursor;
        emit cursorChanged(_cursor);
    }
}
