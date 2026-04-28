#ifndef PERROUNDRESOURCE_H
#define PERROUNDRESOURCE_H

#include <QString>
#include <QHash>
#include <QVariant>

class QDomDocument;
class QDomElement;

// PerRoundResource — descriptor for a counter-style resource attached to a
// MonsterClassv2 (legendary actions, recharge abilities, etc.). The
// per-instance current value lives on BattleDialogModelCombatant; this struct
// only describes the resource definition (name, max, recharge schedule).
//
// Values are stored in the TemplateObject value-bag as list entries
// (QHash<QString, QVariant>) under a single list key (typically
// "perRoundResources") with the keys defined in the static accessors below.
struct PerRoundResource
{
    // Recharge schedule — encoded as a string in the value-bag for forward
    // compatibility with rule-specific dice expressions (e.g. "5-6").
    static const char* RECHARGE_TURN;       // "turn"      — every combatant turn
    static const char* RECHARGE_ROUND;      // "round"     — every combat round
    static const char* RECHARGE_SHORT_REST; // "shortRest"
    static const char* RECHARGE_LONG_REST;  // "longRest"

    // Hash keys (used inside the QHash<QString, QVariant> entry)
    static const char* KEY_NAME;
    static const char* KEY_MAX;
    static const char* KEY_RECHARGE;

    QString name;
    int     max;
    QString recharge;

    PerRoundResource();
    PerRoundResource(const QString& name, int max, const QString& recharge);

    // Round-trip with the value-bag representation.
    QHash<QString, QVariant> toHash() const;
    static PerRoundResource fromHash(const QHash<QString, QVariant>& hash);

    // Round-trip with XML (for per-instance counter persistence on combatants).
    QDomElement writeXML(QDomDocument& doc, const QString& elementName, int currentValue) const;
    static PerRoundResource readXML(const QDomElement& element, int* outCurrentValue = nullptr);
};

#endif // PERROUNDRESOURCE_H
