#include "battledialogmodelinitiativeevent.h"
#include <QDomElement>

BattleDialogModelInitiativeEvent::BattleDialogModelInitiativeEvent(const QString& name, int initiative, QObject* parent) :
    BattleDialogModelCombatant(name, parent)
{
    setInitiative(initiative);
}

BattleDialogModelInitiativeEvent::~BattleDialogModelInitiativeEvent()
{
}

void BattleDialogModelInitiativeEvent::inputXML(const QDomElement& element, bool isImport)
{
    BattleDialogModelCombatant::inputXML(element, isImport);
}

void BattleDialogModelInitiativeEvent::copyValues(const CampaignObjectBase* other)
{
    BattleDialogModelCombatant::copyValues(other);
}

int BattleDialogModelInitiativeEvent::getCombatantType() const
{
    return DMHelper::CombatantType_InitiativeEvent;
}

BattleDialogModelCombatant* BattleDialogModelInitiativeEvent::clone() const
{
    BattleDialogModelInitiativeEvent* result = new BattleDialogModelInitiativeEvent(getName(), getInitiative());
    result->copyValues(this);
    return result;
}

qreal BattleDialogModelInitiativeEvent::getSizeFactor() const
{
    return 1.0;
}

int BattleDialogModelInitiativeEvent::getSizeCategory() const
{
    return DMHelper::CombatantSize_Unknown;
}

int BattleDialogModelInitiativeEvent::getStrength() const { return 0; }
int BattleDialogModelInitiativeEvent::getDexterity() const { return 0; }
int BattleDialogModelInitiativeEvent::getConstitution() const { return 0; }
int BattleDialogModelInitiativeEvent::getIntelligence() const { return 0; }
int BattleDialogModelInitiativeEvent::getWisdom() const { return 0; }
int BattleDialogModelInitiativeEvent::getCharisma() const { return 0; }

int BattleDialogModelInitiativeEvent::getSkillModifier(Combatant::Skills skill) const
{
    Q_UNUSED(skill);
    return 0;
}

QStringList BattleDialogModelInitiativeEvent::getConditionList() const
{
    return QStringList();
}

bool BattleDialogModelInitiativeEvent::hasConditionId(const QString& conditionId) const
{
    Q_UNUSED(conditionId);
    return false;
}

int BattleDialogModelInitiativeEvent::getSpeed() const { return 0; }
int BattleDialogModelInitiativeEvent::getArmorClass() const { return 0; }

int BattleDialogModelInitiativeEvent::getHitPoints() const
{
    // Always report a positive HP so BattleFrame::getNextCombatant doesn't
    // skip us as "dead". The active toggle is wired through getKnown().
    return 1;
}

void BattleDialogModelInitiativeEvent::setHitPoints(int hitPoints)
{
    Q_UNUSED(hitPoints);
}

QPixmap BattleDialogModelInitiativeEvent::getIconPixmap(DMHelper::PixmapSize iconSize) const
{
    Q_UNUSED(iconSize);
    return QPixmap();
}

void BattleDialogModelInitiativeEvent::setConditionList(const QStringList& conditions) { Q_UNUSED(conditions); }
void BattleDialogModelInitiativeEvent::addConditionId(const QString& conditionId) { Q_UNUSED(conditionId); }
void BattleDialogModelInitiativeEvent::removeConditionId(const QString& conditionId) { Q_UNUSED(conditionId); }
void BattleDialogModelInitiativeEvent::clearConditions() {}

QDomElement BattleDialogModelInitiativeEvent::createOutputXML(QDomDocument& doc)
{
    return doc.createElement("initiativeevent");
}

void BattleDialogModelInitiativeEvent::internalOutputXML(QDomDocument& doc, QDomElement& element, QDir& targetDirectory, bool isExport)
{
    BattleDialogModelCombatant::internalOutputXML(doc, element, targetDirectory, isExport);
}
