#include "dmhnetworkdata_private.h"
#include <QByteArray>

DMHNetworkData_Private_Base::DMHNetworkData_Private_Base()
{
}

DMHNetworkData_Private_Base::~DMHNetworkData_Private_Base()
{
}

bool DMHNetworkData_Private_Base::readChildElement(QDomElement element, const QString& childName, QString& stringOut)
{
    if(element.isNull())
        return false;

    QDomElement childElement = element.firstChildElement(childName);
    if(!childElement.isNull())
        stringOut = childElement.text();

    return !childElement.isNull();
}

bool DMHNetworkData_Private_Base::readChildElement(QDomElement element, const QString& childName, QByteArray& baOut)
{
    if(element.isNull())
        return false;

    QDomElement childElement = element.firstChildElement(childName);
    if(!childElement.isNull())
    {
        QString baString = childElement.text();
        QByteArray base64data = baString.toUtf8();
        baOut = QByteArray::fromBase64(base64data);
    }

    return !childElement.isNull();
}




DMHNetworkData_Private::DMHNetworkData_Private() :
    DMHNetworkData_Private_Base()
{
}

DMHNetworkData_Private::~DMHNetworkData_Private()
{
}




DMHNetworkData_Payload_Private::DMHNetworkData_Payload_Private(QDomElement data) :
    DMHNetworkData_Private_Base(),
    _payload(),
    _timestamp(),
    _valid(true)
{
    QString payloadString;
    if(readChildElement(data, QString("payload"), payloadString))
        _payload.fromString(payloadString);
    else
        _valid = false;

    if(!readChildElement(data, QString("last"), _timestamp))
        _valid = false;
}

DMHNetworkData_Payload_Private::~DMHNetworkData_Payload_Private()
{
}

bool DMHNetworkData_Payload_Private::isValid()
{
    return _valid;
}

const DMHPayload& DMHNetworkData_Payload_Private::getPayload() const
{
    return _payload;
}

QString DMHNetworkData_Payload_Private::getTimestamp() const
{
    return _timestamp;
}




DMHNetworkData_Raw_Private::DMHNetworkData_Raw_Private(QDomElement data) :
    DMHNetworkData_Private_Base(),
    _name(),
    _id(),
    _data(),
    _valid(true)
{
    QDomElement nodeElement = data.firstChildElement(QString("node"));
    if(nodeElement.isNull())
    {
        _valid = false;
    }
    else
    {
        if(!readChildElement(nodeElement, QString("name"), _name))
            _valid = false;

        if(!readChildElement(nodeElement, QString("ID"), _id))
            _valid = false;

        if(!readChildElement(nodeElement, QString("data"), _data))
            _valid = false;
    }
}

DMHNetworkData_Raw_Private::~DMHNetworkData_Raw_Private()
{
}

bool DMHNetworkData_Raw_Private::isValid()
{
    return _valid;
}

QString DMHNetworkData_Raw_Private::getName() const
{
    return _name;
}

QString DMHNetworkData_Raw_Private::getId() const
{
    return _id;
}

QByteArray DMHNetworkData_Raw_Private::getData() const
{
    return _data;
}






DMHNetworkData_Upload_Private::DMHNetworkData_Upload_Private(QDomElement data) :
    DMHNetworkData_Private_Base(),
    _name(),
    _id(),
    _valid(true)
{
    QDomElement nodeElement = data.firstChildElement(QString("node"));
    if(nodeElement.isNull())
    {
        _valid = false;
    }
    else
    {
        if(!readChildElement(nodeElement, QString("name"), _name))
            _valid = false;

        if(!readChildElement(nodeElement, QString("id"), _id))
            _valid = false;
    }
}

DMHNetworkData_Upload_Private::~DMHNetworkData_Upload_Private()
{
}

bool DMHNetworkData_Upload_Private::isValid()
{
    return _valid;
}

QString DMHNetworkData_Upload_Private::getName() const
{
    return _name;
}

QString DMHNetworkData_Upload_Private::getId() const
{
    return _id;
}




DMHNetworkData_Exists_Private::DMHNetworkData_Exists_Private(QDomElement data) :
    DMHNetworkData_Private_Base(),
    _name(),
    _md5(),
    _valid(true)
{
    QDomElement nodeElement = data.firstChildElement(QString("node"));
    if(nodeElement.isNull())
    {
        _valid = false;
    }
    else
    {
        readChildElement(nodeElement, QString("ID"), _md5);
        readChildElement(nodeElement, QString("name"), _name);
    }
}

DMHNetworkData_Exists_Private::~DMHNetworkData_Exists_Private()
{
}

bool DMHNetworkData_Exists_Private::isValid()
{
    return _valid;
}

bool DMHNetworkData_Exists_Private::exists() const
{
    return !_md5.isEmpty();
}

QString DMHNetworkData_Exists_Private::getName() const
{
    return _name;
}

QString DMHNetworkData_Exists_Private::getMD5() const
{
    return _md5;
}
