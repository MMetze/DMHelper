#include "dmhobjectbase_private.h"
#include "dmhobjectbase.h"
#include <QtGlobal>
#include <QDebug>

int DMHObjectBase_Private::_id_global = DMH_GLOBAL_BASE_ID;

DMHObjectBase_Private::DMHObjectBase_Private() :
    _id(createId())
{
}

DMHObjectBase_Private::DMHObjectBase_Private(const DMHObjectBase_Private& other) :
    _id(other._id)
{
}

DMHObjectBase_Private::~DMHObjectBase_Private()
{
}

void DMHObjectBase_Private::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory)
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);

    parent.setAttribute( "_baseID", getID() );
}

void DMHObjectBase_Private::inputXML(const QDomElement &element)
{
    if(element.hasAttribute(QString("_baseID")))
        _id = element.attribute( QString("_baseID") ).toInt();
    else
        qDebug() << "[DMHObjectBase_Private] WARNING: Object read from file without base ID. Default ID is created, risk of ID duplication!";
}

void DMHObjectBase_Private::postProcessXML(const QDomElement &element)
{
    Q_UNUSED(element);
}

int DMHObjectBase_Private::getID() const
{
    return _id;
}

void DMHObjectBase_Private::setID(int id)
{
    _id = id;
}

void DMHObjectBase_Private::setBaseId(int baseId)
{
    _id_global = baseId;
}

void DMHObjectBase_Private::resetBaseId()
{
    _id_global = DMH_GLOBAL_BASE_ID;
}

int DMHObjectBase_Private::createId()
{
    return _id_global++;
}
