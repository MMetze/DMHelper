#include "spellv2converter.h"
#include "spell.h"
#include <QDomElement>

Spellv2Converter::Spellv2Converter(const QDomElement &element) :
    Spellv2{QString(), nullptr}
{
    Spell* legacy = new Spell(element, false);

    beginBatchChanges();
    convertValues(legacy);
    endBatchChanges();

    delete legacy;
}

void Spellv2Converter::convertValues(Spell* spell)
{
    if(!spell)
        return;

    setName(spell->getName());
    setLevel(spell->getLevel());
    setSchool(spell->getSchool());
    setTime(spell->getTime());
    setRange(spell->getRange());
    setComponents(spell->getComponents());
    setDuration(spell->getDuration());
    setClasses(spell->getClasses());
    setDescription(spell->getDescription());
    setRitual(spell->isRitual());
    setRolls(spell->getRolls());

    setEffectType(spell->getEffectType());
    setEffectShapeActive(spell->getEffectShapeActive());
    setEffectSize(spell->getEffectSize());
    setEffectColor(spell->getEffectColor());
    setEffectToken(spell->getEffectToken());
    setEffectTokenRotation(spell->getEffectTokenRotation());
    setEffectConditionList(spell->getEffectConditionList());
}
