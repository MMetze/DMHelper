#include "dmhnetworkmanager.h"
#include "dmhnetworkmanager_private.h"
#include <QDebug>

DMHNetworkManager::DMHNetworkManager(const DMHLogon& logon, QObject *parent) :
    QObject(parent),
    d(new DMHNetworkManager_Private(logon))
{
    connect(&(*d), SIGNAL(uploadComplete(int, const QString&)), this, SIGNAL(uploadComplete(int, const QString&)));
    connect(&(*d), SIGNAL(downloadStarted(int, const QString&, QNetworkReply*)), this, SIGNAL(downloadStarted(int, const QString&, QNetworkReply*)));
    connect(&(*d), SIGNAL(downloadComplete(int, const QString&, const QByteArray&)), this, SIGNAL(downloadComplete(int, const QString&, const QByteArray&)));
    connect(&(*d), SIGNAL(existsComplete(int, const QString&, const QString&, bool)), this, SIGNAL(existsComplete(int, const QString&, const QString&, bool)));
    connect(&(*d), SIGNAL(createSessionComplete(int, const QString&, const QString&)), this, SIGNAL(createSessionComplete(int, const QString&, const QString&)));
    connect(&(*d), SIGNAL(isOwnerComplete(int, const QString&, const QString&, const QString&, bool)), this, SIGNAL(isOwnerComplete(int, const QString&, const QString&, const QString&, bool)));
    connect(&(*d), SIGNAL(renameSessionComplete(int, const QString&)), this, SIGNAL(renameSessionComplete(int, const QString&)));
    connect(&(*d), SIGNAL(renewSessionComplete(int, const QString&, const QString&)), this, SIGNAL(renewSessionComplete(int, const QString&, const QString&)));
    connect(&(*d), SIGNAL(closeSessionComplete(int, const QString&)), this, SIGNAL(closeSessionComplete(int, const QString&)));
    connect(&(*d), SIGNAL(createUserStarted(int, QNetworkReply*, const QString&, const QString&, const QString&)), this, SIGNAL(createUserStarted(int, QNetworkReply*, const QString&, const QString&, const QString&)));
    connect(&(*d), SIGNAL(createUserComplete(int, const QString&, const QString&, const QString&)), this, SIGNAL(createUserComplete(int, const QString&, const QString&, const QString&)));
    connect(&(*d), SIGNAL(joinSessionStarted(int, QNetworkReply*, const QString&, const QString&)), this, SIGNAL(joinSessionStarted(int, QNetworkReply*, const QString&, const QString&)));
    connect(&(*d), SIGNAL(joinSessionComplete(int, const QString&)), this, SIGNAL(joinSessionComplete(int, const QString&)));
    connect(&(*d), SIGNAL(sessionMembersComplete(int, const QString&, const QString&)), this, SIGNAL(sessionMembersComplete(int, const QString&, const QString&)));
    connect(&(*d), SIGNAL(otherRequestComplete()), this, SIGNAL(otherRequestComplete()));
    connect(&(*d), SIGNAL(messageError(int, const QString&)), this, SIGNAL(messageError(int, const QString&)));
    connect(&(*d), SIGNAL(requestError(int)), this, SIGNAL(requestError(int)));
    connect(&(*d), SIGNAL(DEBUG_message_contents(const QByteArray&)), this, SIGNAL(DEBUG_message_contents(const QByteArray&)));
    connect(&(*d), SIGNAL(DEBUG_response_contents(const QByteArray&)), this, SIGNAL(DEBUG_response_contents(const QByteArray&)));
}

DMHNetworkManager::~DMHNetworkManager()
{
}

DMHNetworkObserver* DMHNetworkManager::registerNetworkObserver(QObject *parent)
{
    return d->registerNetworkObserver(parent);
}

void DMHNetworkManager::uploadPayload(const DMHPayload& payload)
{
    d->uploadPayload(payload);
}

int DMHNetworkManager::uploadFile(const QString& filename)
{
    return d->uploadFile(filename);
}

int DMHNetworkManager::fileExists(const QString& fileMD5)
{
    return d->fileExists(fileMD5);
}

int DMHNetworkManager::uploadData(const QByteArray& data)
{
    return d->uploadData(data);
}

int DMHNetworkManager::downloadFile(const QString& fileMD5)
{
    return d->downloadFile(fileMD5);
}

int DMHNetworkManager::createSession(const QString & sessionName)
{
    return d->createSession(sessionName);
}

int DMHNetworkManager::isSessionOwner(const QString & session)
{
    return d->isSessionOwner(session);
}

int DMHNetworkManager::renameSession(const QString & sessionName, const QString & session)
{
    return d->renameSession(sessionName, session);
}

int DMHNetworkManager::removeSession(const QString & session)
{
    return d->removeSession(session);
}

int DMHNetworkManager::renewSessionInvite(const QString & session)
{
    return d->renewSessionInvite(session);
}

int DMHNetworkManager::closeSession(const QString & session)
{
    return d->closeSession(session);
}

int DMHNetworkManager::getSessionMembers(const QString & session)
{
    return d->getSessionMembers(session);
}

int DMHNetworkManager::createUser(const QString& username, const QString& password, const QString& email, const QString& screenName)
{
    return d->createUser(username, password, email, screenName);
}

int DMHNetworkManager::joinSession(const QString& invite)
{
    return d->joinSession(invite);
}

int DMHNetworkManager::sendMessage(const QString& message, const QString& userId)
{
    return d->sendMessage(message, userId);
}

int DMHNetworkManager::pollMessages()
{
    return d->pollMessages();
}

int DMHNetworkManager::ackMessages()
{
    return d->ackMessages();
}

void DMHNetworkManager::abortRequest(int id)
{
    return d->abortRequest(id);
}

const DMHLogon& DMHNetworkManager::getLogon() const
{
    return d->getLogon();
}

void DMHNetworkManager::setLogon(const DMHLogon& logon)
{
    d->setLogon(logon);
}

QNetworkReply* DMHNetworkManager::getNetworkReply(int requestID)
{
    return d->getNetworkReply(requestID);
}
