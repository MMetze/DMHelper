#include "dmhnetworkdata.h"
#include "dmhnetworkdata_private.h"
#include <QDomElement>
#include <QString>

DMHNetworkData::DMHNetworkData() :
    d(new DMHNetworkData_Private())
{
}

DMHNetworkData::~DMHNetworkData()
{
}



DMHNetworkData_Payload::DMHNetworkData_Payload(QDomElement data) :
    DMHNetworkData(),
    d(new DMHNetworkData_Payload_Private(data))
{
}

DMHNetworkData_Payload::~DMHNetworkData_Payload()
{
}

bool DMHNetworkData_Payload::isValid()
{
    return d->isValid();
}

const DMHPayload& DMHNetworkData_Payload::getPayload() const
{
    return d->getPayload();
}

QString DMHNetworkData_Payload::getTimestamp() const
{
    return d->getTimestamp();
}



DMHNetworkData_Raw::DMHNetworkData_Raw(QDomElement data) :
    DMHNetworkData(),
    d(new DMHNetworkData_Raw_Private(data))
{
}

DMHNetworkData_Raw::~DMHNetworkData_Raw()
{
}

bool DMHNetworkData_Raw::isValid()
{
    return d->isValid();
}

QString DMHNetworkData_Raw::getName() const
{
    return d->getName();
}

QString DMHNetworkData_Raw::getId() const
{
    return d->getId();
}

QByteArray DMHNetworkData_Raw::getData() const
{
    return d->getData();
}





DMHNetworkData_Upload::DMHNetworkData_Upload(QDomElement data) :
    DMHNetworkData(),
    d(new DMHNetworkData_Upload_Private(data))
{
}

DMHNetworkData_Upload::~DMHNetworkData_Upload()
{
}

bool DMHNetworkData_Upload::isValid()
{
    return d->isValid();
}

QString DMHNetworkData_Upload::getName() const
{
    return d->getName();
}

QString DMHNetworkData_Upload::getId() const
{
    return d->getId();
}




DMHNetworkData_Exists::DMHNetworkData_Exists(QDomElement data) :
    DMHNetworkData(),
    d(new DMHNetworkData_Exists_Private(data))
{
}

DMHNetworkData_Exists::~DMHNetworkData_Exists()
{
}

bool DMHNetworkData_Exists::isValid()
{
    return d->isValid();
}

bool DMHNetworkData_Exists::exists() const
{
    return d->exists();
}

QString DMHNetworkData_Exists::getName() const
{
    return d->getName();
}

QString DMHNetworkData_Exists::getMD5() const
{
    return d->getMD5();
}
