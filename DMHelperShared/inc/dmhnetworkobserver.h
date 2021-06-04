#ifndef NETWORKOBSERVER_H
#define NETWORKOBSERVER_H

#include "dmhglobal.h"
#include <QObject>
#include <memory>

class DMHPayload;
class DMHMessage;
class DMHLogon;
class DMHNetworkObserver_Private;

//class DMHSHARED_EXPORT DMHNetworkObserver : public QObject
class DMHNetworkObserver : public QObject
{
    Q_OBJECT
public:
    enum ObserverType
    {
        ObserverType_None = 0,
        ObserverType_Payload = 1,
        ObserverType_Message = 2,

        ObserverType_All = ObserverType_Payload | ObserverType_Message
    };

    explicit DMHNetworkObserver(const DMHLogon& logon, ObserverType observerType = ObserverType_All, QObject *parent = nullptr);
    virtual ~DMHNetworkObserver();

    void setLogon(const DMHLogon& logon);
    void setObserverType(ObserverType observerType);
    void setInterval(int interval);

signals:
    void payloadReceived(const DMHPayload& payload, const QString& timestamp);
    void messageReceived(const QList<DMHMessage>& messages);

    void DEBUG_message_contents(const QByteArray& data);
    void DEBUG_response_contents(const QByteArray& data);

public slots:
    void start();
    void stop();

protected:

    std::unique_ptr<DMHNetworkObserver_Private> d;
};

#endif // NETWORKOBSERVER_H
