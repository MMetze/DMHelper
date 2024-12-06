#ifndef MONSTERCLASSV2_H
#define MONSTERCLASSV2_H

#include "templateobject.h"
#include <QObject>

class QDomElement;

class MonsterClassv2 : public QObject, public TemplateObject
{
    Q_OBJECT
public:
    explicit MonsterClassv2(const QString& name, QObject *parent = nullptr);
    explicit MonsterClassv2(const QDomElement &element, bool isImport, QObject *parent = nullptr);

signals:
};

#endif // MONSTERCLASSV2_H
