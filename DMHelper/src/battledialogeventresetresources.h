#ifndef BATTLEDIALOGEVENTRESETRESOURCES_H
#define BATTLEDIALOGEVENTRESETRESOURCES_H

#include "battledialogevent.h"
#include <QString>

// BattleDialogEventResetResources — fired by BattleFrame when per-round
// resources should be reset on combatants. The "scope" string identifies
// which recharge schedules should fire (one of PerRoundResource::RECHARGE_*).
// CombatantTemplateFrame instances listen and reset matching resources on
// their backing combatant.
class BattleDialogEventResetResources : public BattleDialogEvent
{
    Q_OBJECT
public:
    explicit BattleDialogEventResetResources(const QString& scope = QString());
    explicit BattleDialogEventResetResources(const QDomElement& element);
    BattleDialogEventResetResources(const BattleDialogEventResetResources& other);
    virtual ~BattleDialogEventResetResources() override;

    virtual int getType() const override;
    virtual BattleDialogEvent* clone() override;

    virtual QDomElement outputXML(QDomElement &element, bool isExport) override;

    QString getScope() const;
    void setScope(const QString& scope);

private:
    QString _scope;
};

#endif // BATTLEDIALOGEVENTRESETRESOURCES_H
