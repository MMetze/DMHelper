#include "rulefactory.h"
#include "ruleinitiative2e.h"
#include "ruleinitiative5e.h"
#include "ruleinitiativegroup.h"
#include "ruleinitiativegroupmonsters.h"

RuleFactory::RuleFactory(QObject *parent)
    : QObject{parent}
{}

RuleInitiative* RuleFactory::createRuleInitiative(const QString& ruleInitiativeType, QObject *parent)
{
    if(ruleInitiativeType == RuleInitiative5e::InitiativeType)
        return new RuleInitiative5e(parent);

    if(ruleInitiativeType == RuleInitiativeGroup::InitiativeType)
        return new RuleInitiativeGroup(parent);

    if(ruleInitiativeType == RuleInitiativeGroupMonsters::InitiativeType)
        return new RuleInitiativeGroupMonsters(parent);

    if(ruleInitiativeType == RuleInitiative2e::InitiativeType)
        return new RuleInitiative2e(parent);

    return nullptr;
}

QString RuleFactory::getRuleInitiativeDefault()
{
    return RuleInitiative5e::InitiativeType;
}

QStringList RuleFactory::getRuleInitiativeNames()
{
    QStringList result;

    result << RuleInitiative5e::InitiativeType << RuleInitiative5e::InitiativeDescription
           << RuleInitiative2e::InitiativeType << RuleInitiative2e::InitiativeDescription
           << RuleInitiativeGroup::InitiativeType << RuleInitiativeGroup::InitiativeDescription
           << RuleInitiativeGroupMonsters::InitiativeType << RuleInitiativeGroupMonsters::InitiativeDescription;

    return result;
}
