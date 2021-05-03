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



DMHNetworkData_DM_Push::DMHNetworkData_DM_Push(QDomElement data) :
    DMHNetworkData(),
    d(new DMHNetworkData_DM_Push_Private(data))
{
}

DMHNetworkData_DM_Push::~DMHNetworkData_DM_Push()
{
}

bool DMHNetworkData_DM_Push::isValid()
{
    return d->isValid();
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

QString DMHNetworkData_Raw::getMD5() const
{
    return d->getMD5();
}

QString DMHNetworkData_Raw::getUuid() const
{
    return d->getUuid();
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

QString DMHNetworkData_Upload::getMD5() const
{
    return d->getMD5();
}

QString DMHNetworkData_Upload::getUuid() const
{
    return d->getUuid();
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

QString DMHNetworkData_Exists::getUuid() const
{
    return d->getUuid();
}




DMHNetworkData_IsOwner::DMHNetworkData_IsOwner(QDomElement data) :
    DMHNetworkData(),
    d(new DMHNetworkData_IsOwner_Private(data))
{
}

DMHNetworkData_IsOwner::~DMHNetworkData_IsOwner()
{
}

bool DMHNetworkData_IsOwner::isValid()
{
    return d->isValid();
}

QString DMHNetworkData_IsOwner::getSession() const
{
    return d->getSession();
}

QString DMHNetworkData_IsOwner::getUser() const
{
    return d->getUser();
}

QString DMHNetworkData_IsOwner::getSessionName() const
{
    return d->getSessionName();
}

QString DMHNetworkData_IsOwner::getInvite() const
{
    return d->getInvite();
}

bool DMHNetworkData_IsOwner::isOwner() const
{
    return d->isOwner();
}





DMHNetworkData_CreateSession::DMHNetworkData_CreateSession(QDomElement data) :
    DMHNetworkData(),
    d(new DMHNetworkData_CreateSession_Private(data))
{
}

DMHNetworkData_CreateSession::~DMHNetworkData_CreateSession()
{
}

bool DMHNetworkData_CreateSession::isValid()
{
    return d->isValid();
}

QString DMHNetworkData_CreateSession::getSession() const
{
    return d->getSession();
}

QString DMHNetworkData_CreateSession::getInvite() const
{
    return d->getInvite();
}




DMHNetworkData_RenameSession::DMHNetworkData_RenameSession(QDomElement data) :
    DMHNetworkData(),
    d(new DMHNetworkData_RenameSession_Private(data))
{
}

DMHNetworkData_RenameSession::~DMHNetworkData_RenameSession()
{
}

bool DMHNetworkData_RenameSession::isValid()
{
    return d->isValid();
}

QString DMHNetworkData_RenameSession::getName() const
{
    return d->getName();
}






DMHNetworkData_RenewSession::DMHNetworkData_RenewSession(QDomElement data) :
    DMHNetworkData(),
    d(new DMHNetworkData_RenewSession_Private(data))
{
}

DMHNetworkData_RenewSession::~DMHNetworkData_RenewSession()
{
}

bool DMHNetworkData_RenewSession::isValid()
{
    return d->isValid();
}

QString DMHNetworkData_RenewSession::getSession() const
{
    return d->getSession();
}

QString DMHNetworkData_RenewSession::getInvite() const
{
    return d->getInvite();
}






DMHNetworkData_CloseSession::DMHNetworkData_CloseSession(QDomElement data) :
    DMHNetworkData(),
    d(new DMHNetworkData_CloseSession_Private(data))
{
}

DMHNetworkData_CloseSession::~DMHNetworkData_CloseSession()
{
}

bool DMHNetworkData_CloseSession::isValid()
{
    return d->isValid();
}

QString DMHNetworkData_CloseSession::getSession() const
{
    return d->getSession();
}




DMHNetworkData_SessionMembers::DMHNetworkData_SessionMembers(QDomElement data) :
    DMHNetworkData(),
    d(new DMHNetworkData_SessionMembers_Private(data))
{
}

DMHNetworkData_SessionMembers::~DMHNetworkData_SessionMembers()
{
}

bool DMHNetworkData_SessionMembers::isValid()
{
    return d->isValid();
}

QString DMHNetworkData_SessionMembers::getSession() const
{
    return d->getSession();
}

QString DMHNetworkData_SessionMembers::getMembers() const
{
    return d->getMembers();
}






DMHNetworkData_SessionGeneral::DMHNetworkData_SessionGeneral(QDomElement data) :
    DMHNetworkData(),
    d(new DMHNetworkData_SessionGeneral_Private(data))
{
}

DMHNetworkData_SessionGeneral::~DMHNetworkData_SessionGeneral()
{
}

bool DMHNetworkData_SessionGeneral::isValid()
{
    return d->isValid();
}






DMHNetworkData_CreateUser::DMHNetworkData_CreateUser(QDomElement data) :
    DMHNetworkData(),
    d(new DMHNetworkData_CreateUser_Private(data))
{
}

DMHNetworkData_CreateUser::~DMHNetworkData_CreateUser()
{
}

bool DMHNetworkData_CreateUser::isValid()
{
    return d->isValid();
}

QString DMHNetworkData_CreateUser::getUsername() const
{
    return d->getUsername();
}

QString DMHNetworkData_CreateUser::getUserId() const
{
    return d->getUserId();
}

QString DMHNetworkData_CreateUser::getEmail() const
{
    return d->getEmail();
}





DMHNetworkData_JoinSession::DMHNetworkData_JoinSession(QDomElement data) :
    DMHNetworkData(),
    d(new DMHNetworkData_JoinSession_Private(data))
{
}

DMHNetworkData_JoinSession::~DMHNetworkData_JoinSession()
{
}

bool DMHNetworkData_JoinSession::isValid()
{
    return d->isValid();
}

QString DMHNetworkData_JoinSession::getSession() const
{
    return d->getSession();
}





DMHNetworkData_Message::DMHNetworkData_Message(QDomElement data) :
    DMHNetworkData(),
    d(new DMHNetworkData_Message_Private(data))
{
}

DMHNetworkData_Message::~DMHNetworkData_Message()
{
}

bool DMHNetworkData_Message::isValid()
{
    return d->isValid();
}

QString DMHNetworkData_Message::getData() const
{
    return d->getData();
}


DMHNetworkData_UserInfo::DMHNetworkData_UserInfo(QDomElement data) :
    DMHNetworkData(),
    d(new DMHNetworkData_UserInfo_Private(data))
{
}

DMHNetworkData_UserInfo::~DMHNetworkData_UserInfo()
{
}

bool DMHNetworkData_UserInfo::isValid()
{
    return d->isValid();
}

QString DMHNetworkData_UserInfo::getUserId() const
{
    return d->getUserId();
}

QString DMHNetworkData_UserInfo::getUsername() const
{
    return d->getUsername();
}

QString DMHNetworkData_UserInfo::getMail() const
{
    return d->getMail();
}

QString DMHNetworkData_UserInfo::getSurname() const
{
    return d->getSurname();
}

QString DMHNetworkData_UserInfo::getForename() const
{
    return d->getForename();
}

bool DMHNetworkData_UserInfo::getDisabled() const
{
    return d->getDisabled();
}

