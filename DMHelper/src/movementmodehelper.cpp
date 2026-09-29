#include "movementmodehelper.h"
#include "battledialogmodelcombatant.h"
#include "battledialogmodelmonsterbase.h"
#include "characterv2.h"
#include "monster.h"
#include "monsterclassv2.h"
#include <QRegularExpression>
#include <QSet>

namespace
{
    static const QRegularExpression FIRST_NUMBER_RE(QStringLiteral("(\\d+)"));
}

QString MovementModeHelper::normalizeMovementModeKey(const QString& value)
{
    QString key = value.trimmed().toLower();
    if(key.isEmpty())
        return QString();

    key.remove(QRegularExpression(QStringLiteral("[^a-z\\- ]")));
    return key.simplified();
}

QString MovementModeHelper::movementModeDisplayName(const QString& key)
{
    if(key == QStringLiteral("walk"))
        return QStringLiteral("Walk");
    if(key == QStringLiteral("custom"))
        return QStringLiteral("Custom");
    if(key.isEmpty())
        return QStringLiteral("Walk");

    QString result = key;
    result[0] = result.at(0).toUpper();
    return result;
}

QList<MovementModeHelper::MovementModeValue> MovementModeHelper::parseMovementModes(const QString& movementText)
{
    QList<MovementModeValue> result;
    QSet<QString> addedKeys;

    const QStringList segments = movementText.split(',', Qt::SkipEmptyParts);
    for(const QString& segment : segments)
    {
        const QString trimmed = segment.trimmed();
        if(trimmed.isEmpty())
            continue;

        const QRegularExpressionMatch numberMatch = FIRST_NUMBER_RE.match(trimmed);
        if(!numberMatch.hasMatch())
            continue;

        const int speedFt = numberMatch.captured(1).toInt();
        if(speedFt <= 0)
            continue;

        const QString prefix = normalizeMovementModeKey(trimmed.left(numberMatch.capturedStart(1)));
        const QString suffix = normalizeMovementModeKey(trimmed.mid(numberMatch.capturedEnd(1)));
        const QString key = !prefix.isEmpty() ? prefix : (!suffix.isEmpty() ? suffix : QStringLiteral("walk"));
        if(addedKeys.contains(key))
            continue;

        MovementModeValue mode;
        mode.key = key;
        mode.label = movementModeDisplayName(key);
        mode.speedFt = speedFt;
        result.append(mode);
        addedKeys.insert(key);
    }

    return result;
}

QString MovementModeHelper::movementStringForCombatant(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return QString();

    if(BattleDialogModelMonsterBase* monsterBase = dynamic_cast<BattleDialogModelMonsterBase*>(combatant))
    {
        MonsterClassv2* monsterClass = monsterBase->getMonsterClass();
        if(monsterClass)
            return monsterClass->getStringValue(QStringLiteral("speed"));
    }

    Combatant* sourceCombatant = combatant->getCombatant();
    if(Characterv2* character = dynamic_cast<Characterv2*>(sourceCombatant))
        return character->getMovementString();
    if(Monster* monster = dynamic_cast<Monster*>(sourceCombatant))
        return monster->getMovementString();

    return QString::number(combatant->getSpeed());
}

QList<MovementModeHelper::MovementModeValue> MovementModeHelper::getMovementModes(BattleDialogModelCombatant* combatant)
{
    QList<MovementModeValue> modes = parseMovementModes(movementStringForCombatant(combatant));
    if(modes.isEmpty() && combatant)
    {
        const int speedFt = combatant->getSpeed();
        if(speedFt > 0)
        {
            MovementModeValue mode;
            mode.key = QStringLiteral("walk");
            mode.label = QStringLiteral("Walk");
            mode.speedFt = speedFt;
            modes.append(mode);
        }
    }

    return modes;
}

bool MovementModeHelper::hasMovementMode(const QList<MovementModeValue>& modes, const QString& key)
{
    for(const MovementModeValue& mode : modes)
    {
        if(mode.key == key)
            return true;
    }

    return false;
}

QString MovementModeHelper::defaultMovementModeKey(const QList<MovementModeValue>& modes)
{
    if(hasMovementMode(modes, QStringLiteral("walk")))
        return QStringLiteral("walk");

    return modes.isEmpty() ? QString() : modes.first().key;
}

QString MovementModeHelper::effectiveMovementModeKey(BattleDialogModelCombatant* combatant, const QList<MovementModeValue>& modes)
{
    if(!combatant)
        return QString();

    const QString selectedMode = combatant->getSelectedMovementMode().trimmed().toLower();
    if((selectedMode == QStringLiteral("custom")) && (combatant->getCustomMovementSpeedFt() > 0))
        return selectedMode;
    if(!selectedMode.isEmpty() && hasMovementMode(modes, selectedMode))
        return selectedMode;

    return defaultMovementModeKey(modes);
}

int MovementModeHelper::effectiveMovementSpeedFt(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return 0;

    const QList<MovementModeValue> modes = getMovementModes(combatant);
    const QString modeKey = effectiveMovementModeKey(combatant, modes);
    if(modeKey == QStringLiteral("custom"))
        return combatant->getCustomMovementSpeedFt();

    for(const MovementModeValue& mode : modes)
    {
        if(mode.key == modeKey)
            return mode.speedFt;
    }

    return combatant->getSpeed();
}

QList<MovementModeHelper::MovementModeValue> MovementModeHelper::intersectMovementModes(const QList<BattleDialogModelCombatant*>& combatants)
{
    QList<MovementModeValue> commonModes;
    if(combatants.isEmpty())
        return commonModes;

    commonModes = getMovementModes(combatants.first());
    for(int i = 1; i < combatants.count(); ++i)
    {
        const QList<MovementModeValue> currentModes = getMovementModes(combatants.at(i));
        QSet<QString> currentKeys;
        for(const MovementModeValue& mode : currentModes)
            currentKeys.insert(mode.key);

        for(int modeIndex = commonModes.count() - 1; modeIndex >= 0; --modeIndex)
        {
            if(!currentKeys.contains(commonModes.at(modeIndex).key))
                commonModes.removeAt(modeIndex);
        }
    }

    return commonModes;
}
