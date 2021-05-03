#ifndef DMHNETWORKDATA_H
#define DMHNETWORKDATA_H

#include "dmhglobal.h"
#include "dmhpayload.h"
#include <QDomElement>
#include <memory>

class QString;
class DMHNetworkData_Private;
class DMHNetworkData_DM_Push_Private;
class DMHNetworkData_Payload_Private;
class DMHNetworkData_Raw_Private;
class DMHNetworkData_Upload_Private;
class DMHNetworkData_Exists_Private;
class DMHNetworkData_IsOwner_Private;
class DMHNetworkData_CreateSession_Private;
class DMHNetworkData_RenameSession_Private;
class DMHNetworkData_RenewSession_Private;
class DMHNetworkData_CloseSession_Private;
class DMHNetworkData_SessionMembers_Private;
class DMHNetworkData_CreateUser_Private;
class DMHNetworkData_JoinSession_Private;
class DMHNetworkData_Message_Private;
class DMHNetworkData_UserInfo_Private;


//class DMHSHARED_EXPORT DMHNetworkData
class DMHNetworkData
{
public:
    DMHNetworkData();
    virtual ~DMHNetworkData();

    virtual bool isValid() = 0;

private:
    // No copy constructor allowed
    DMHNetworkData(const DMHNetworkData& other);

    std::unique_ptr<DMHNetworkData_Private> d;
};



//class DMHSHARED_EXPORT DMHNetworkData_Payload : public DMHNetworkData
class DMHNetworkData_DM_Push : public DMHNetworkData
{
public:
    explicit DMHNetworkData_DM_Push(QDomElement data);
    virtual ~DMHNetworkData_DM_Push() override;

    // From DMHNetworkData
    virtual bool isValid() override;

private:
    // No copy constructor allowed
    DMHNetworkData_DM_Push(const DMHNetworkData_DM_Push& other);

    std::unique_ptr<DMHNetworkData_DM_Push_Private> d;
};



//class DMHSHARED_EXPORT DMHNetworkData_Payload : public DMHNetworkData
class DMHNetworkData_Payload : public DMHNetworkData
{
public:
    explicit DMHNetworkData_Payload(QDomElement data);
    virtual ~DMHNetworkData_Payload() override;

    // From DMHNetworkData
    virtual bool isValid() override;

    const DMHPayload& getPayload() const;
    QString getTimestamp() const;

private:
    // No copy constructor allowed
    DMHNetworkData_Payload(const DMHNetworkData_Payload& other);

    std::unique_ptr<DMHNetworkData_Payload_Private> d;
};



//class DMHSHARED_EXPORT DMHNetworkData_Raw : public DMHNetworkData
class DMHNetworkData_Raw : public DMHNetworkData
{
public:
    explicit DMHNetworkData_Raw(QDomElement data);
    virtual ~DMHNetworkData_Raw() override;

    // From DMHNetworkData
    virtual bool isValid() override;

    QString getName() const;
    QString getMD5() const;
    QString getUuid() const;
    QByteArray getData() const;

private:
    // No copy constructor allowed
    DMHNetworkData_Raw(const DMHNetworkData_Raw& other);

    std::unique_ptr<DMHNetworkData_Raw_Private> d;
};





//class DMHSHARED_EXPORT DMHNetworkData_Upload : public DMHNetworkData
class DMHNetworkData_Upload : public DMHNetworkData
{
public:
    explicit DMHNetworkData_Upload(QDomElement data);
    virtual ~DMHNetworkData_Upload() override;

    // From DMHNetworkData
    virtual bool isValid() override;

    QString getName() const;
    QString getMD5() const;
    QString getUuid() const;

private:
    // No copy constructor allowed
    DMHNetworkData_Upload(const DMHNetworkData_Upload& other);

    std::unique_ptr<DMHNetworkData_Upload_Private> d;
};






//class DMHSHARED_EXPORT DMHNetworkData_Exists : public DMHNetworkData
class DMHNetworkData_Exists : public DMHNetworkData
{
public:
    explicit DMHNetworkData_Exists(QDomElement data);
    virtual ~DMHNetworkData_Exists() override;

    // From DMHNetworkData
    virtual bool isValid() override;

    bool exists() const;
    QString getName() const;
    QString getMD5() const;
    QString getUuid() const;

private:
    // No copy constructor allowed
    DMHNetworkData_Exists(const DMHNetworkData_Exists& other);

    std::unique_ptr<DMHNetworkData_Exists_Private> d;
};






class DMHNetworkData_IsOwner : public DMHNetworkData
{
public:
    explicit DMHNetworkData_IsOwner(QDomElement data);
    virtual ~DMHNetworkData_IsOwner() override;

    // From DMHNetworkData
    virtual bool isValid() override;

    QString getSession() const;
    QString getUser() const;
    QString getSessionName() const;
    QString getInvite() const;
    bool isOwner() const;

private:
    // No copy constructor allowed
    DMHNetworkData_IsOwner(const DMHNetworkData_IsOwner& other);

    std::unique_ptr<DMHNetworkData_IsOwner_Private> d;
};




class DMHNetworkData_CreateSession : public DMHNetworkData
{
public:
    explicit DMHNetworkData_CreateSession(QDomElement data);
    virtual ~DMHNetworkData_CreateSession() override;

    // From DMHNetworkData
    virtual bool isValid() override;

    QString getSession() const;
    QString getInvite() const;

private:
    // No copy constructor allowed
    DMHNetworkData_CreateSession(const DMHNetworkData_CreateSession& other);

    std::unique_ptr<DMHNetworkData_CreateSession_Private> d;
};




class DMHNetworkData_RenameSession : public DMHNetworkData
{
public:
    explicit DMHNetworkData_RenameSession(QDomElement data);
    virtual ~DMHNetworkData_RenameSession() override;

    // From DMHNetworkData
    virtual bool isValid() override;

    QString getName() const;

private:
    // No copy constructor allowed
    DMHNetworkData_RenameSession(const DMHNetworkData_RenameSession& other);

    std::unique_ptr<DMHNetworkData_RenameSession_Private> d;
};



class DMHNetworkData_RenewSession : public DMHNetworkData
{
public:
    explicit DMHNetworkData_RenewSession(QDomElement data);
    virtual ~DMHNetworkData_RenewSession() override;

    // From DMHNetworkData
    virtual bool isValid() override;

    QString getSession() const;
    QString getInvite() const;

private:
    // No copy constructor allowed
    DMHNetworkData_RenewSession(const DMHNetworkData_RenewSession& other);

    std::unique_ptr<DMHNetworkData_RenewSession_Private> d;
};




class DMHNetworkData_CloseSession : public DMHNetworkData
{
public:
    explicit DMHNetworkData_CloseSession(QDomElement data);
    virtual ~DMHNetworkData_CloseSession() override;

    // From DMHNetworkData
    virtual bool isValid() override;

    QString getSession() const;

private:
    // No copy constructor allowed
    DMHNetworkData_CloseSession(const DMHNetworkData_CloseSession& other);

    std::unique_ptr<DMHNetworkData_CloseSession_Private> d;
};




class DMHNetworkData_SessionMembers : public DMHNetworkData
{
public:
    explicit DMHNetworkData_SessionMembers(QDomElement data);
    virtual ~DMHNetworkData_SessionMembers() override;

    // From DMHNetworkData
    virtual bool isValid() override;

    QString getSession() const;
    QString getMembers() const;

private:
    // No copy constructor allowed
    DMHNetworkData_SessionMembers(const DMHNetworkData_SessionMembers& other);

    std::unique_ptr<DMHNetworkData_SessionMembers_Private> d;
};






class DMHNetworkData_CreateUser : public DMHNetworkData
{
public:
    explicit DMHNetworkData_CreateUser(QDomElement data);
    virtual ~DMHNetworkData_CreateUser() override;

    // From DMHNetworkData
    virtual bool isValid() override;

    QString getUsername() const;
    QString getUserId() const;
    QString getEmail() const;

private:
    // No copy constructor allowed
    DMHNetworkData_CreateUser(const DMHNetworkData_CreateUser& other);

    std::unique_ptr<DMHNetworkData_CreateUser_Private> d;
};









class DMHNetworkData_JoinSession : public DMHNetworkData
{
public:
    explicit DMHNetworkData_JoinSession(QDomElement data);
    virtual ~DMHNetworkData_JoinSession() override;

    // From DMHNetworkData
    virtual bool isValid() override;

    QString getSession() const;

private:
    // No copy constructor allowed
    DMHNetworkData_JoinSession(const DMHNetworkData_JoinSession& other);

    std::unique_ptr<DMHNetworkData_JoinSession_Private> d;
};








class DMHNetworkData_Message : public DMHNetworkData
{
public:
    explicit DMHNetworkData_Message(QDomElement data);
    virtual ~DMHNetworkData_Message() override;

    // From DMHNetworkData
    virtual bool isValid() override;

    QString getData() const;

private:
    // No copy constructor allowed
    DMHNetworkData_Message(const DMHNetworkData_Message& other);

    std::unique_ptr<DMHNetworkData_Message_Private> d;
};









class DMHNetworkData_UserInfo : public DMHNetworkData
{
public:
    explicit DMHNetworkData_UserInfo(QDomElement data);
    virtual ~DMHNetworkData_UserInfo() override;

    // From DMHNetworkData
    virtual bool isValid() override;

    QString getUserId() const;
    QString getUsername() const;
    QString getMail() const;
    QString getSurname() const;
    QString getForename() const;
    bool getDisabled() const;

private:
    // No copy constructor allowed
    DMHNetworkData_UserInfo(const DMHNetworkData_UserInfo& other);

    std::unique_ptr<DMHNetworkData_UserInfo_Private> d;
};


#endif // DMHNETWORKDATA_H
