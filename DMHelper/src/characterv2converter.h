#ifndef CHARACTERV2CONVERTER_H
#define CHARACTERV2CONVERTER_H

#include "characterv2.h"

class Character;
class MonsterClassv2;

class Characterv2Converter : public Characterv2
{
    Q_OBJECT
public:
    explicit Characterv2Converter(const QString& name = QString(), QObject *parent = nullptr);

    virtual void inputXML(const QDomElement &element, bool isImport) override;

protected:
    virtual void convertValues(Character* convertCharacter);

};

#endif // CHARACTERV2CONVERTER_H
