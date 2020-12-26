#include "dmhobjectbase_private.h"
#include "dmhobjectbase.h"
#include <QtGlobal>
#include <QDebug>

//int DMHObjectBase_Private::_id_global = DMH_GLOBAL_BASE_ID;

DMHObjectBase_Private::DMHObjectBase_Private() :
    _id(QUuid::createUuid()),
    _intId(DMH_GLOBAL_BASE_ID)
{
}

DMHObjectBase_Private::DMHObjectBase_Private(const DMHObjectBase_Private& other) :
    _id(other._id),
    _intId(other._intId)
{
}

DMHObjectBase_Private::~DMHObjectBase_Private()
{
}

QDomElement DMHObjectBase_Private::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport)
{
    Q_UNUSED(doc);
    Q_UNUSED(parent);
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    //parent.setAttribute( "_baseID", getID().toString() );
    return QDomElement();
}

void DMHObjectBase_Private::inputXML(const QDomElement &element, bool isImport)
{
    if(element.hasAttribute(QString("_baseID")))
    {
        QUuid newId = QUuid(element.attribute(QString("_baseID")));
        if(newId.isNull())
        {
            _intId = element.attribute(QString("_baseID")).toInt();
        }
        else
        {
            _id = newId;
            _intId = DMH_GLOBAL_BASE_ID;
        }
    }
    else if(!isImport)
    {
        qDebug() << "[DMHObjectBase_Private] WARNING: Object read from file without base ID. Default ID is created, risk of ID duplication!";
    }
}

QUuid DMHObjectBase_Private::getID() const
{
    return _id;
}

int DMHObjectBase_Private::getIntID() const
{
    return _intId;
}

void DMHObjectBase_Private::setID(QUuid id)
{
    _id = id;
}

void DMHObjectBase_Private::setIntID(int id)
{
    _intId = id;
}

/*
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
*/

void DMHObjectBase_Private::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    element.setAttribute("_baseID", getID().toString());
}
