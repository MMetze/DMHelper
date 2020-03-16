#ifndef BATTLEFRAMESTATE_H
#define BATTLEFRAMESTATE_H

#include <QObject>
#include <QCursor>
#include "battleframestatemachine.h"

class BattleFrameState : public QObject
{
    Q_OBJECT
public:

    enum BattleFrameStateType
    {
        BattleFrameStateType_Invalid = -1,
        BattleFrameStateType_Base = 0,
        BattleFrameStateType_Persistent,
        BattleFrameStateType_Transient,
    };

    BattleFrameState(int stateId, BattleFrameStateType stateType, QObject *parent = nullptr);
    // Note: if cursorPixmap gets scaled, the hotX and hotY will also get equivalently scaled
    BattleFrameState(int stateId, BattleFrameStateType stateType, const QPixmap& cursorPixmap, int hotX = -1, int hotY = -1, QObject *parent = nullptr);
    virtual ~BattleFrameState();

    bool isValid() const;

    int getId() const;
    BattleFrameStateType getType() const;
    const QCursor& getCursor() const;

signals:
    void activateState(int id);
    void deactivateState(int id);
    void toggleState(int id);

    void stateChanged(bool activated);

    void cursorChanged(const QCursor& cursor);

public slots:
    void activate();
    void deactivate();
    void toggle();
    void enter();
    void leave();

    void setCursor(const QCursor& cursor);

protected:

    int _stateId;
    BattleFrameStateType _stateType;
    QCursor _cursor;

};

#endif // BATTLEFRAMESTATE_H
