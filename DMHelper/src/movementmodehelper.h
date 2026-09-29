#ifndef MOVEMENTMODEHELPER_H
#define MOVEMENTMODEHELPER_H

#include <QList>
#include <QString>

class BattleDialogModelCombatant;

class MovementModeHelper
{
public:
    struct MovementModeValue
    {
        QString key;
        QString label;
        int speedFt;
    };

    static constexpr int CustomMovementMinSpeed = 1;
    static constexpr int CustomMovementMaxSpeed = 1000;

    static QList<MovementModeValue> getMovementModes(BattleDialogModelCombatant* combatant);
    static bool hasMovementMode(const QList<MovementModeValue>& modes, const QString& key);
    static QString effectiveMovementModeKey(BattleDialogModelCombatant* combatant, const QList<MovementModeValue>& modes);
    static int effectiveMovementSpeedFt(BattleDialogModelCombatant* combatant);
    static QList<MovementModeValue> intersectMovementModes(const QList<BattleDialogModelCombatant*>& combatants);

private:
    static QString normalizeMovementModeKey(const QString& value);
    static QString movementModeDisplayName(const QString& key);
    static QList<MovementModeValue> parseMovementModes(const QString& movementText);
    static QString movementStringForCombatant(BattleDialogModelCombatant* combatant);
    static QString defaultMovementModeKey(const QList<MovementModeValue>& modes);
};

#endif // MOVEMENTMODEHELPER_H
