#ifndef SPELLV2CONVERTER_H
#define SPELLV2CONVERTER_H

#include "spellv2.h"

class Spell;

// Reads a legacy <spell> element through the old Spell parser, then copies all
// fields into the value-bag so the result is interchangeable with a natively
// loaded Spellv2. Used by Spellbook::readSpellbook when it detects an old XML.
class Spellv2Converter : public Spellv2
{
public:
    explicit Spellv2Converter(const QDomElement &element);

private:
    void convertValues(Spell* spell);
};

#endif // SPELLV2CONVERTER_H
