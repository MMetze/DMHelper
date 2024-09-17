#ifndef CHARACTERV2CONVERTER_H
#define CHARACTERV2CONVERTER_H

#include "characterv2.h"

class Character;

class Characterv2Converter : public Characterv2
{
    Q_OBJECT
public:
    explicit Characterv2Converter();

    virtual void inputXML(const QDomElement &element, bool isImport) override;

protected:
    virtual void convertValues(Character* convertCharacter);

};

#endif // CHARACTERV2CONVERTER_H
