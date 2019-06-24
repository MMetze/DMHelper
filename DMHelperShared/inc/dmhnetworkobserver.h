#ifndef NETWORKOBSERVER_H
#define NETWORKOBSERVER_H

#include "dmhglobal.h"
#include <QObject>
#include <memory>

class DMHPayload;
class DMHLogon;
class DMHNetworkObserver_Private;

//class DMHSHARED_EXPORT DMHNetworkObserver : public QObject
class DMHNetworkObserver : public QObject
{
    Q_OBJECT
public:
    explicit DMHNetworkObserver(const DMHLogon& logon, QObject *parent = nullptr);
    virtual ~DMHNetworkObserver();

    void setLogon(const DMHLogon& logon);

signals:
    void payloadReceived(const DMHPayload& payload, const QString& timestamp);

    void DEBUG_message_contents(const QByteArray& data);
    void DEBUG_response_contents(const QByteArray& data);

public slots:
    void start();
    void stop();

private:
    std::unique_ptr<DMHNetworkObserver_Private> d;
};

#endif // NETWORKOBSERVER_H
