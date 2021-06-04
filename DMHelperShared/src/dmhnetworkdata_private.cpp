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

QString DMHNetworkData_Private_Base::readChildElement(QDomElement element, const QString& childName)
{
    if(!element.isNull())
    {
        QDomElement childElement = element.firstChildElement(childName);
        if(!childElement.isNull())
            return childElement.text();
    }

    return QString();
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




DMHNetworkData_DM_Push_Private::DMHNetworkData_DM_Push_Private(QDomElement data) :
    DMHNetworkData_Private_Base()
{
    Q_UNUSED(data);
    _valid = true;
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
    _md5(),
    _uuid(),
    _data()
{
    QDomElement nodeElement = data.firstChildElement(QString("node"));
    if(!nodeElement.isNull())
    {
        readChildElement(nodeElement, QString("name"), _name);
        readChildElement(nodeElement, QString("md5"), _md5);
        readChildElement(nodeElement, QString("ID"), _uuid);
        readChildElement(nodeElement, QString("data"), _data);

        _valid = !_data.isEmpty();
        /*
        if((readChildElement(nodeElement, QString("name"), _name)) &&
           (readChildElement(nodeElement, QString("md5"), _md5)) &&
           (readChildElement(nodeElement, QString("ID"), _uuid)) &&
           (readChildElement(nodeElement, QString("data"), _data)))
            _valid = true;
            */
    }
}

QString DMHNetworkData_Raw_Private::getName() const
{
    return _name;
}

QString DMHNetworkData_Raw_Private::getMD5() const
{
    return _md5;
}

QString DMHNetworkData_Raw_Private::getUuid() const
{
    return _uuid;
}

QByteArray DMHNetworkData_Raw_Private::getData() const
{
    return _data;
}






DMHNetworkData_Upload_Private::DMHNetworkData_Upload_Private(QDomElement data) :
    DMHNetworkData_Private_Base(),
    _name(),
    _md5(),
    _uuid()
{
    QDomElement nodeElement = data.firstChildElement(QString("node"));
    if(!nodeElement.isNull())
    {
        if((readChildElement(nodeElement, QString("name"), _name)) &&
           (readChildElement(nodeElement, QString("md5"), _md5)) &&
           (readChildElement(nodeElement, QString("id"), _uuid)))
            _valid = true;
    }
}

QString DMHNetworkData_Upload_Private::getName() const
{
    return _name;
}

QString DMHNetworkData_Upload_Private::getMD5() const
{
    return _md5;
}

QString DMHNetworkData_Upload_Private::getUuid() const
{
    return _uuid;
}




DMHNetworkData_Exists_Private::DMHNetworkData_Exists_Private(QDomElement data) :
    DMHNetworkData_Private_Base(),
    _name(),
    _md5(),
    _uuid()
{
    QDomElement nodeElement = data.firstChildElement(QString("node"));
    if(!nodeElement.isNull())
    {
        if((readChildElement(nodeElement, QString("md5"), _md5)) &&
           (readChildElement(nodeElement, QString("ID"), _uuid)) &&
           (readChildElement(nodeElement, QString("name"), _name)))
            _valid = true;
    }
    else
    {
        _valid = true; // empty / exists=false response
    }
}

bool DMHNetworkData_Exists_Private::exists() const
{
    return ((!_md5.isEmpty() && (!_uuid.isEmpty())));
}

QString DMHNetworkData_Exists_Private::getName() const
{
    return _name;
}

QString DMHNetworkData_Exists_Private::getMD5() const
{
    return _md5;
}

QString DMHNetworkData_Exists_Private::getUuid() const
{
    return _uuid;
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
       (readChildElement(data, QString("isowner"), isOwner)))
    {
        _isOwner = static_cast<bool>(isOwner.toInt());
        if((_isOwner) &&
           ((!readChildElement(data, QString("name"), _sessionName)) ||
           (!readChildElement(data, QString("code"), _invite))))
                return; // If isOwner comes back true, the session name and code should also be part of the response

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





DMHNetworkData_SessionGeneral_Private::DMHNetworkData_SessionGeneral_Private(QDomElement data) :
    DMHNetworkData_Private_Base()
{
    Q_UNUSED(data);
    _valid = true;
}







DMHNetworkData_CreateUser_Private::DMHNetworkData_CreateUser_Private(QDomElement data) :
    DMHNetworkData_Private_Base(),
    _username(),
    _userId(),
    _email()
{
    if((readChildElement(data, QString("username"), _username)) &&
       (readChildElement(data, QString("user"), _userId)) &&
       (readChildElement(data, QString("email"), _email)))
        _valid = true;
}

QString DMHNetworkData_CreateUser_Private::getUsername() const
{
    return _username;
}

QString DMHNetworkData_CreateUser_Private::getUserId() const
{
    return _userId;
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




DMHNetworkData_SimpleMessage_Private::DMHNetworkData_SimpleMessage_Private(QDomElement data) :
    DMHNetworkData_Private_Base(),
    _data()
{
    _data = data.text();
    _valid = true;
}

QString DMHNetworkData_SimpleMessage_Private::getData() const
{
    return _data;
}






DMHNetworkData_Message_Private::DMHNetworkData_Message_Private(QDomElement data) :
    DMHNetworkData_Private_Base(),
    _messages()
{
    _valid = true;
    QDomElement nodeElement = data.firstChildElement(QString("node"));
    while(!nodeElement.isNull())
    {
        _messages.append(DMHMessage(readChildElement(nodeElement, QString("ID")),
                                    readChildElement(nodeElement, QString("sender")),
                                    readChildElement(nodeElement, QString("body")),
                                    static_cast<bool>(readChildElement(nodeElement, QString("polled")).toInt()),
                                    readChildElement(nodeElement, QString("timestamp"))));

        nodeElement = nodeElement.nextSiblingElement(QString("node"));
    }

    // <ID>BD04EDC8-FAF9-4EF9-8563-61AA18B8CAC8</ID>\n   <sender>NewUser</sender>\n   <body>Joining session: NewUser</body>\n   <polled>0</polled>\n   <timestamp>2021-05-30 12:21:09</timestamp>
    // <node>\n   <ID>BD04EDC8-FAF9-4EF9-8563-61AA18B8CAC8</ID>\n   <sender>NewUser</sender>\n   <body>Joining session: NewUser</body>\n   <polled>1</polled>\n   <timestamp>2021-05-30 12:21:09</timestamp>\n  </node>\n  <node>\n   <ID>73BD6FB0-7DDA-4A01-A367-F5A95CE77A45</ID>\n   <sender>NewUser</sender>\n   <body>Joining session: NewUser</body>\n   <polled>0</polled>\n   <timestamp>2021-05-30 12:23:51</timestamp>\n  </node>\n  <node>\n   <ID>DC26831F-707A-4604-994E-C01C99DD4630</ID>\n   <sender>NewUser</sender>\n   <body>Joining session: NewUser</body>\n   <polled>0</polled>\n   <timestamp>2021-05-30 12:23:53</timestamp>\n  </node>\n  <node>\n   <ID>7FC1B8B8-E289-46F3-847B-72F4436C8FEE</ID>\n   <sender>NewUser</sender>\n   <body>Joining session: NewUser</body>\n   <polled>0</polled>\n   <timestamp>2021-05-30 12:24:06</timestamp>\n  </node>\n  <node>\n   <ID>0991FD94-D0D1-4D3A-BE27-40F4E42472F0</ID>\n   <sender>NewUser</sender>\n   <body>Joining session: NewUser</body>\n   <polled>0</polled>\n   <timestamp>2021-05-30 12:24:10</timestamp>\n  </node>
    //_data = data.text();
    _valid = true;
}

QList<DMHMessage> DMHNetworkData_Message_Private::getMessages() const
{
    return _messages;
}





DMHNetworkData_UserInfo_Private::DMHNetworkData_UserInfo_Private(QDomElement data) :
    DMHNetworkData_Private_Base(),
    _userId(),
    _username(),
    _mail(),
    _surname(),
    _forename(),
    _disabled(false)
{

    QString disabled;
    if((readChildElement(data, QString("ID"), _userId)) &&
       (readChildElement(data, QString("username"), _username)) &&
       (readChildElement(data, QString("mail"), _mail)) &&
       (readChildElement(data, QString("surname"), _surname)) &&
       (readChildElement(data, QString("forename"), _forename)) &&
       (readChildElement(data, QString("disabled"), disabled)))
    {
         _disabled = static_cast<bool>(disabled.toInt());
        _valid = true;
    }
}

QString DMHNetworkData_UserInfo_Private::getUserId() const
{
    return _userId;
}

QString DMHNetworkData_UserInfo_Private::getUsername() const
{
    return _username;
}

QString DMHNetworkData_UserInfo_Private::getMail() const
{
    return _mail;
}

QString DMHNetworkData_UserInfo_Private::getSurname() const
{
    return _surname;
}

QString DMHNetworkData_UserInfo_Private::getForename() const
{
    return _forename;
}

bool DMHNetworkData_UserInfo_Private::getDisabled() const
{
    return _disabled;
}


