#include "dmhobjectbase.h"
#include "dmhobjectbase_private.h"
#include <QDebug>

DMHObjectBase::DMHObjectBase(QObject *parent) :
    QObject(parent),
    d(new DMHObjectBase_Private())
{
}

DMHObjectBase::DMHObjectBase(const DMHObjectBase& other) :
    QObject(other.parent()),
    d(new DMHObjectBase_Private(*(other.d)))
{
    qDebug() << "[DMHObjectBase] WARNING: Object copied - this is a highly questionable action leading to duplicate IDs! ID: " << getID();
}

DMHObjectBase::~DMHObjectBase()
{
}

QDomElement DMHObjectBase::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport)
{
    return d->outputXML(doc, parent, targetDirectory, isExport);
}

void DMHObjectBase::inputXML(const QDomElement &element, bool isImport)
{
    d->inputXML(element, isImport);
}

void DMHObjectBase::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    d->internalOutputXML(doc, element, targetDirectory, isExport);
}

QUuid DMHObjectBase::getID() const
{
    return d->getID();
}

int DMHObjectBase::getIntID() const
{
    return d->getIntID();
}

void DMHObjectBase::setID(QUuid id)
{
    d->setID(id);
}

void DMHObjectBase::setIntID(int id)
{
    d->setIntID(id);
}

void DMHObjectBase::renewID()
{
    QUuid oldId = getID();
    d->renewID();
    qDebug() << "[DMHObjectBase] WARNING: Object " << objectName() << " reset from ID " << oldId << " to " << getID();
}
