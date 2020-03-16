#ifndef BATTLEFRAMESTATEMACHINE_H
#define BATTLEFRAMESTATEMACHINE_H

#include <QObject>
#include <QList>

class BattleFrameState;

class BattleFrameStateMachine : public QObject
{
    Q_OBJECT
public:
    explicit BattleFrameStateMachine(QObject *parent = nullptr);
    virtual ~BattleFrameStateMachine();

    // The base mode is with all specific modes deactivated
    // Persistent modes are active until deactivated
    // Transient modes are active until used, then fall back to the previous persistent mode

    /*
    enum BattleFrameStateType
    {
        BattleFrameStateType_CombatantEdit = 0,
        BattleFrameStateType_ZoomSelect,
        BattleFrameStateType_CameraSelect,
        BattleFrameStateType_CameraEdit,
        BattleFrameStateType_Distance,
        BattleFrameStateType_Pointer,
        BattleFrameStateType_FoWSelect,
        BattleFrameStateType_FoWEdit,
    };
    */

    void addState(BattleFrameState* state);

    BattleFrameState* getCurrentState() const;
    int getCurrentStateId() const;
    BattleFrameState* getPreviousState() const;
    int getPreviousStateId() const;

signals:
    void enterState(BattleFrameState* state);
    void leaveState(BattleFrameState* state);
    void stateChanged(int stateId);

public slots:
    void clear();
    void reset();
    void activateState(int stateId);
    void deactivateState(int stateId = -1);
    void toggleState(int stateId);

private:

    int getBaseStateIndex();
    int getIndexFromId(int stateId);
    BattleFrameState* getState(int index) const;
    void communicateStateChange(BattleFrameState* oldState, BattleFrameState* newState);

    QList<BattleFrameState*> _stateList;
    int _currentStateIndex;
    int _previousStateIndex;

    //BattleFrameStateType _currentState;
    //BattleFrameStateType _previousState;
};

#endif // BATTLEFRAMESTATEMACHINE_H
