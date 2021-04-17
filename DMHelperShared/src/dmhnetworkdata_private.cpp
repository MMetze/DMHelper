#include "dmhnetworkdata_private.h"
#include <QByteArray>
#include <QTextStream>
#include <QDebug>


DMHNetworkData_Private_Base::DMHNetworkData_Private_Base() :
    _valid(false)
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

bool DMHNetworkData_Private_Base::isValid()
{
    return _valid;
}




DMHNetworkData_Private::DMHNetworkData_Private() :
    DMHNetworkData_Private_Base()
{
}




DMHNetworkData_Payload_Private::DMHNetworkData_Payload_Private(QDomElement data) :
    DMHNetworkData_Private_Base(),
    _payload(),
    _timestamp()
{
    _payload.fromString(data.text());
    /*
    QString dataText = data.text();

    QString payloadString;
    if(readChildElement(data, QString("data"), payloadString))
        _payload.fromString(payloadString);
    else
        _valid = false;
        */

    if(readChildElement(data, QString("last"), _timestamp))
        _valid = true;
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
    _data()
{
    QDomElement nodeElement = data.firstChildElement(QString("node"));
    if(!nodeElement.isNull())
    {
        if((readChildElement(nodeElement, QString("name"), _name)) &&
           (readChildElement(nodeElement, QString("ID"), _id)) &&
           (readChildElement(nodeElement, QString("data"), _data)))
            _valid = true;
    }
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
    _id()
{
    QDomElement nodeElement = data.firstChildElement(QString("node"));
    if(!nodeElement.isNull())
    {
        if((readChildElement(nodeElement, QString("name"), _name)) &&
           (readChildElement(nodeElement, QString("id"), _id)))
            _valid = true;
    }
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
    _md5()
{
    QDomElement nodeElement = data.firstChildElement(QString("node"));
    if(!nodeElement.isNull())
    {
        if((readChildElement(nodeElement, QString("ID"), _md5)) &&
           (readChildElement(nodeElement, QString("name"), _name)))
            _valid = true;
    }
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





DMHNetworkData_IsOwner_Private::DMHNetworkData_IsOwner_Private(QDomElement data) :
    DMHNetworkData_Private_Base(),
    _session(),
    _user(),
    _sessionName(),
    _invite(),
    _isOwner(false)
{
    QString isOwner;
    if((readChildElement(data, QString("session"), _session)) &&
       (readChildElement(data, QString("user"), _user)) &&
       (readChildElement(data, QString("name"), _sessionName)) &&
       (readChildElement(data, QString("isowner"), isOwner)) &&
       (readChildElement(data, QString("code"), _invite)))
    {
        _isOwner = static_cast<bool>(isOwner.toInt());
        _valid = true;
    }
}

QString DMHNetworkData_IsOwner_Private::getSession() const
{
    return _session;
}

QString DMHNetworkData_IsOwner_Private::getUser() const
{
    return _user;
}

QString DMHNetworkData_IsOwner_Private::getSessionName() const
{
    return _sessionName;
}

QString DMHNetworkData_IsOwner_Private::getInvite() const
{
    return _invite;
}

bool DMHNetworkData_IsOwner_Private::isOwner() const
{
    return _isOwner;
}





DMHNetworkData_CreateSession_Private::DMHNetworkData_CreateSession_Private(QDomElement data) :
    DMHNetworkData_Private_Base(),
    _session(),
    _invite()
{
    if((readChildElement(data, QString("session"), _session)) &&
       (readChildElement(data, QString("code"), _invite)))
        _valid = true;
}

QString DMHNetworkData_CreateSession_Private::getSession() const
{
    return _session;
}

QString DMHNetworkData_CreateSession_Private::getInvite() const
{
    return _invite;
}





DMHNetworkData_RenameSession_Private::DMHNetworkData_RenameSession_Private(QDomElement data) :
    DMHNetworkData_Private_Base(),
    _name()
{
    _name = data.text();
    _valid = !_name.isEmpty();
}

QString DMHNetworkData_RenameSession_Private::getName() const
{
    return _name;
}





DMHNetworkData_RenewSession_Private::DMHNetworkData_RenewSession_Private(QDomElement data) :
    DMHNetworkData_Private_Base(),
    _session(),
    _invite()
{
    if((readChildElement(data, QString("session"), _session)) &&
       (readChildElement(data, QString("code"), _invite)))
        _valid = true;
}

QString DMHNetworkData_RenewSession_Private::getSession() const
{
    return _session;
}

QString DMHNetworkData_RenewSession_Private::getInvite() const
{
    return _invite;
}





DMHNetworkData_CloseSession_Private::DMHNetworkData_CloseSession_Private(QDomElement data) :
    DMHNetworkData_Private_Base(),
    _session()
{
    if(readChildElement(data, QString("session"), _session))
        _valid = true;
}

QString DMHNetworkData_CloseSession_Private::getSession() const
{
    return _session;
}




DMHNetworkData_SessionMembers_Private::DMHNetworkData_SessionMembers_Private(QDomElement data) :
    DMHNetworkData_Private_Base(),
    _session(),
    _members()
{
    if(readChildElement(data, QString("session"), _session))
    {
        QDomElement membersElement = data.firstChildElement("members");
        if(!membersElement.isNull())
        {
            QTextStream stream(&_members);
            membersElement.save(stream, 0);
            _valid = true;
        }
    }
}

QString DMHNetworkData_SessionMembers_Private::getSession() const
{
    return _session;
}

QString DMHNetworkData_SessionMembers_Private::getMembers() const
{
    return _members;
}





DMHNetworkData_CreateUser_Private::DMHNetworkData_CreateUser_Private(QDomElement data) :
    DMHNetworkData_Private_Base(),
    _username(),
    _email()
{
    if((readChildElement(data, QString("username"), _username)) &&
       (readChildElement(data, QString("email"), _email)))
        _valid = true;
}

QString DMHNetworkData_CreateUser_Private::getUsername() const
{
    return _username;
}

QString DMHNetworkData_CreateUser_Private::getEmail() const
{
    return _email;
}



DMHNetworkData_JoinSession_Private::DMHNetworkData_JoinSession_Private(QDomElement data) :
    DMHNetworkData_Private_Base(),
    _session()
{
    _session = data.text();
    _valid = !_session.isEmpty();
}

QString DMHNetworkData_JoinSession_Private::getSession() const
{
    return _session;
}
