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

void DMHObjectBase::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory)
{
    d->outputXML(doc, parent, targetDirectory);
}

void DMHObjectBase::inputXML(const QDomElement &element)
{
    d->inputXML(element);
}

void DMHObjectBase::postProcessXML(const QDomElement &element)
{
    d->postProcessXML(element);
}

int DMHObjectBase::getID() const
{
    return d->getID();
}

void DMHObjectBase::setBaseId(int baseId)
{
    DMHObjectBase_Private::setBaseId(baseId);
}

void DMHObjectBase::resetBaseId()
{
    DMHObjectBase_Private::resetBaseId();
}

int DMHObjectBase::createId()
{
    return DMHObjectBase_Private::createId();
}

void DMHObjectBase::setID(int id)
{
    d->setID(id);
}

