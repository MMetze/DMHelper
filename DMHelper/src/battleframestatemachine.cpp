#include "battleframestatemachine.h"
#include "battleframestate.h"
#include <QDebug>

BattleFrameStateMachine::BattleFrameStateMachine(QObject *parent) :
    QObject(parent),
    _stateList(),
    _currentStateIndex(-1),
    _previousStateIndex(-1)
{
}

BattleFrameStateMachine::~BattleFrameStateMachine()
{
    qDeleteAll(_stateList);
}

void BattleFrameStateMachine::addState(BattleFrameState* state)
{
    if(state == nullptr)
    {
        qDebug() << "[BattleFrameStateMachine] ERROR: attemtping to add a NULL state";
        return;
    }

    connect(state, &BattleFrameState::activateState, this, &BattleFrameStateMachine::activateState);
    connect(state, &BattleFrameState::deactivateState, this, &BattleFrameStateMachine::deactivateState);
    connect(state, &BattleFrameState::toggleState, this, &BattleFrameStateMachine::toggleState);
    _stateList.append(state);
}

BattleFrameState* BattleFrameStateMachine::getCurrentState() const
{
    return getState(_currentStateIndex);
}

int BattleFrameStateMachine::getCurrentStateId() const
{
    BattleFrameState* currentState = getCurrentState();
    if(currentState)
        return currentState->getId();
    else
        return -1;
}

BattleFrameState* BattleFrameStateMachine::getPreviousState() const
{
    return getState(_previousStateIndex);
}

int BattleFrameStateMachine::getPreviousStateId() const
{
    BattleFrameState* previousState = getPreviousState();
    if(previousState)
        return previousState->getId();
    else
        return -1;
}

void BattleFrameStateMachine::clear()
{
    _previousStateIndex = -1;
    _currentStateIndex = -1;
    qDeleteAll(_stateList);
}

void BattleFrameStateMachine::reset()
{
    BattleFrameState* previousState = getPreviousState();
    _previousStateIndex = -1;
    _currentStateIndex = _stateList.isEmpty() ? -1 : 0;
    communicateStateChange(previousState, getCurrentState());
}

void BattleFrameStateMachine::activateState(int stateId)
{
    BattleFrameState* currentState = getCurrentState();
    if(currentState == nullptr)
    {
        qDebug() << "[BattleFrameStateMachine] ERROR: State machine is in an invalid state. Unable activated new state: " << stateId;
        return;
    }

    if(currentState->getId() == stateId)
        return;

    int newIndex = getIndexFromId(stateId);
    BattleFrameState* newState = getState(newIndex);
    if(newState == nullptr)
    {
        qDebug() << "[BattleFrameStateMachine] ERROR: unable to find activated state: " << stateId;
        return;
    }

    if(newState->getType() == BattleFrameState::BattleFrameStateType_Transient)
        _previousStateIndex = _currentStateIndex;
    else
        _previousStateIndex = -1;

    _currentStateIndex = newIndex;

    communicateStateChange((_previousStateIndex == -1) ? currentState : nullptr, newState);
}

void BattleFrameStateMachine::deactivateState(int stateId)
{
    BattleFrameState* currentState = getCurrentState();
    if(currentState == nullptr)
    {
        qDebug() << "[BattleFrameStateMachine] ERROR: State machine is in an invalid state. Unable to deactivate state: " << stateId;
        return;
    }

    BattleFrameState* newState = getPreviousState();

    if((stateId != -1) && (currentState->getId() != stateId))
    {
        // The deactivated state is not the current state, check if it's the previous state or skip it.
        if((newState != nullptr) && (newState->getId() == stateId))
        {
            _previousStateIndex = -1;
        }
        return;
    }

    if(newState != nullptr)
    {
        // return to the previous state
        _currentStateIndex = _previousStateIndex;
    }
    else
    {
        int baseIndex = getBaseStateIndex();
        newState = getState(baseIndex);
        if(newState == nullptr)
        {
            qDebug() << "[BattleFrameStateMachine] ERROR: State Machine does not have a base state. Unable to deactivate state.";
            return;
        }

        // return to the base state
        _currentStateIndex = baseIndex;
    }

    _previousStateIndex = -1;
    communicateStateChange(currentState, newState);
}

void BattleFrameStateMachine::toggleState(int stateId)
{
    int toggleIndex = getIndexFromId(stateId);
    if((_currentStateIndex == toggleIndex) || (_previousStateIndex == toggleIndex))
        deactivateState(stateId);
    else
        activateState(stateId);
}

int BattleFrameStateMachine::getBaseStateIndex()
{
    for(int i = 0; i < _stateList.count(); ++i)
    {
        BattleFrameState* state = _stateList.at(i);
        if((state != nullptr) && (state->getType() == BattleFrameState::BattleFrameStateType_Base))
            return i;
    }

    return -1;
}

int BattleFrameStateMachine::getIndexFromId(int stateId)
{
    for(int i = 0; i < _stateList.count(); ++i)
    {
        BattleFrameState* state = _stateList.at(i);
        if((state != nullptr) && (state->getId() == stateId))
            return i;
    }

    return -1;
}

BattleFrameState* BattleFrameStateMachine::getState(int index) const
{
    if((index < 0) || (index >= _stateList.count()))
        return nullptr;

    return _stateList.at(index);
}

void BattleFrameStateMachine::communicateStateChange(BattleFrameState* oldState, BattleFrameState* newState)
{
    if(oldState)
    {
        oldState->leave();
        emit leaveState(oldState);
    }

    if(newState)
    {
        emit enterState(newState);
        newState->enter();
        emit stateChanged(newState->getId());
    }
}
