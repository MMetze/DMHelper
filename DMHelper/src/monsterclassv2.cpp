#include "monsterclassv2.h"
#include "monsterfactory.h"

MonsterClassv2::MonsterClassv2(const QString& name, QObject *parent) :
    QObject{parent},
    TemplateObject{MonsterFactory::Instance()}
{
}

MonsterClassv2::MonsterClassv2(const QDomElement &element, bool isImport, QObject *parent) :
    QObject{parent},
    TemplateObject{MonsterFactory::Instance()}
{
}
