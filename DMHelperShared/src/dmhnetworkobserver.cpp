#include "dmhnetworkobserver.h"
#include "dmhnetworkobserver_private.h"

DMHNetworkObserver::DMHNetworkObserver(const DMHLogon& logon, QObject *parent) :
    QObject(parent),
    d(new DMHNetworkObserver_Private(logon))
{
    connect(&(*d), SIGNAL(payloadReceived(const DMHPayload&, const QString&)), this, SIGNAL(payloadReceived(const DMHPayload&, const QString&)));
    connect(&(*d), SIGNAL(DEBUG_message_contents(const QByteArray&)), this, SIGNAL(DEBUG_message_contents(const QByteArray&)));
    connect(&(*d), SIGNAL(DEBUG_response_contents(const QByteArray&)), this, SIGNAL(DEBUG_response_contents(const QByteArray&)));
}

DMHNetworkObserver::~DMHNetworkObserver()
{
}

void DMHNetworkObserver::setLogon(const DMHLogon& logon)
{
    d->setLogon(logon);
}

void DMHNetworkObserver::start()
{
    d->start();
}

void DMHNetworkObserver::stop()
{
    d->stop();
}
