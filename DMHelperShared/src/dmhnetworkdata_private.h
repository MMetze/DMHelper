#ifndef DMHNETWORKDATA_PRIVATE_H
#define DMHNETWORKDATA_PRIVATE_H

#include <QString>
#include <QDomElement>
#include "dmhpayload.h"

class QByteArray;

class DMHNetworkData_Private_Base
{
public:
    DMHNetworkData_Private_Base();
    virtual ~DMHNetworkData_Private_Base() {}

    virtual bool readChildElement(QDomElement element, const QString& childName, QString& stringOut);
    virtual bool readChildElement(QDomElement element, const QString& childName, QByteArray& baOut);

    // From DMHNetworkData
    virtual bool isValid();

protected:
    bool _valid;

};




class DMHNetworkData_Private : public DMHNetworkData_Private_Base
{
public:
    DMHNetworkData_Private();
    virtual ~DMHNetworkData_Private() {}

private:

};





class DMHNetworkData_DM_Push_Private : public DMHNetworkData_Private_Base
{
public:
    explicit DMHNetworkData_DM_Push_Private(QDomElement data);
    virtual ~DMHNetworkData_DM_Push_Private() override {}

protected:

};




class DMHNetworkData_Payload_Private : public DMHNetworkData_Private_Base
{
public:
    explicit DMHNetworkData_Payload_Private(QDomElement data);
    virtual ~DMHNetworkData_Payload_Private() override {}

    const DMHPayload& getPayload() const;
    QString getTimestamp() const;

protected:
    DMHPayload _payload;
    QString _timestamp;
};





class DMHNetworkData_Raw_Private : public DMHNetworkData_Private_Base
{
public:
    explicit DMHNetworkData_Raw_Private(QDomElement data);
    virtual ~DMHNetworkData_Raw_Private() override {}

    QString getName() const;
    QString getMD5() const;
    QString getUuid() const;
    QByteArray getData() const;

protected:
    QString _name;
    QString _md5;
    QString _uuid;
    QByteArray _data;
};



class DMHNetworkData_CreateUser_Private : public DMHNetworkData_Private_Base
{
public:
    explicit DMHNetworkData_CreateUser_Private(QDomElement data);
    virtual ~DMHNetworkData_CreateUser_Private() override {}

    QString getUsername() const;
    QString getUserId() const;
    QString getEmail() const;

protected:
    QString _username;
    QString _userId;
    QString _email;
};





class DMHNetworkData_Upload_Private : public DMHNetworkData_Private_Base
{
public:
    explicit DMHNetworkData_Upload_Private(QDomElement data);
    virtual ~DMHNetworkData_Upload_Private() override {}

    QString getName() const;
    QString getMD5() const;
    QString getUuid() const;

protected:
    QString _name;
    QString _md5;
    QString _uuid;
};




class DMHNetworkData_Exists_Private : public DMHNetworkData_Private_Base
{
public:
    explicit DMHNetworkData_Exists_Private(QDomElement data);
    virtual ~DMHNetworkData_Exists_Private() override {}

    bool exists() const;
    QString getName() const;
    QString getMD5() const;
    QString getUuid() const;

protected:
    QString _name;
    QString _md5;
    QString _uuid;
};





class DMHNetworkData_IsOwner_Private : public DMHNetworkData_Private_Base
{
public:
    explicit DMHNetworkData_IsOwner_Private(QDomElement data);
    virtual ~DMHNetworkData_IsOwner_Private() override {}

    QString getSession() const;
    QString getUser() const;
    QString getSessionName() const;
    QString getInvite() const;
    bool isOwner() const;

protected:
    QString _session;
    QString _user;
    QString _sessionName;
    QString _invite;
    bool _isOwner;
};




class DMHNetworkData_CreateSession_Private : public DMHNetworkData_Private_Base
{
public:
    explicit DMHNetworkData_CreateSession_Private(QDomElement data);
    virtual ~DMHNetworkData_CreateSession_Private() override {}

    QString getSession() const;
    QString getInvite() const;

protected:
    QString _session;
    QString _invite;
};




class DMHNetworkData_RenameSession_Private : public DMHNetworkData_Private_Base
{
public:
    explicit DMHNetworkData_RenameSession_Private(QDomElement data);
    virtual ~DMHNetworkData_RenameSession_Private() override {}

    QString getName() const;

protected:
    QString _name;
};




class DMHNetworkData_RenewSession_Private : public DMHNetworkData_Private_Base
{
public:
    explicit DMHNetworkData_RenewSession_Private(QDomElement data);
    virtual ~DMHNetworkData_RenewSession_Private() override {}

    QString getSession() const;
    QString getInvite() const;

protected:
    QString _session;
    QString _invite;
};




class DMHNetworkData_CloseSession_Private : public DMHNetworkData_Private_Base
{
public:
    explicit DMHNetworkData_CloseSession_Private(QDomElement data);
    virtual ~DMHNetworkData_CloseSession_Private() override {}

    QString getSession() const;

protected:
    QString _session;
};




class DMHNetworkData_SessionMembers_Private : public DMHNetworkData_Private_Base
{
public:
    explicit DMHNetworkData_SessionMembers_Private(QDomElement data);
    virtual ~DMHNetworkData_SessionMembers_Private() override {}

    QString getSession() const;
    QString getMembers() const;

protected:
    QString _session;
    QString _members;
};





class DMHNetworkData_JoinSession_Private : public DMHNetworkData_Private_Base
{
public:
    explicit DMHNetworkData_JoinSession_Private(QDomElement data);
    virtual ~DMHNetworkData_JoinSession_Private() override {}

    QString getSession() const;

protected:
    QString _session;
};






class DMHNetworkData_Message_Private : public DMHNetworkData_Private_Base
{
public:
    explicit DMHNetworkData_Message_Private(QDomElement data);
    virtual ~DMHNetworkData_Message_Private() override {}

    QString getData() const;

protected:
    QString _data;
};







class DMHNetworkData_UserInfo_Private : public DMHNetworkData_Private_Base
{
public:
    explicit DMHNetworkData_UserInfo_Private(QDomElement data);
    virtual ~DMHNetworkData_UserInfo_Private() override {}

    QString getUserId() const;
    QString getUsername() const;
    QString getMail() const;
    QString getSurname() const;
    QString getForename() const;
    bool getDisabled() const;

protected:
    QString _userId;
    QString _username;
    QString _mail;
    QString _surname;
    QString _forename;
    bool _disabled;
};


#endif // DMHNETWORKDATA_PRIVATE_H
