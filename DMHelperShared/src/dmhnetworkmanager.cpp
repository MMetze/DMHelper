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
    connect(&(*d), SIGNAL(otherRequestComplete()), this, SIGNAL(otherRequestComplete()));
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
