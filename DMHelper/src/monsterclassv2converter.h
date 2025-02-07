#ifndef MONSTERCLASSV2CONVERTER_H
#define MONSTERCLASSV2CONVERTER_H

#include "monsterclassv2.h"

class MonsterClass;
class MonsterAction;

class MonsterClassv2Converter : public MonsterClassv2
{
public:
    explicit MonsterClassv2Converter(const QDomElement &element);

private:
    void convertValues(MonsterClass* monsterClass);
    void convertList(const QString& listName, const QList<MonsterAction>& actionList);
};

#endif // MONSTERCLASSV2CONVERTER_H
