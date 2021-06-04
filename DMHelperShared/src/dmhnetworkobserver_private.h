#ifndef DMHNETWORKOBSERVER_PRIVATE_H
#define DMHNETWORKOBSERVER_PRIVATE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "dmhlogon.h"
#include "dmhnetworkobserver.h"

class DMHPayload;

class DMHNetworkObserver_Private : public QObject
{
    Q_OBJECT
public:

    explicit DMHNetworkObserver_Private(const DMHLogon& logon, DMHNetworkObserver::ObserverType observerType, QObject *parent = nullptr);
    ~DMHNetworkObserver_Private();

    void setLogon(const DMHLogon& logon);
    void setObserverType(DMHNetworkObserver::ObserverType observerType);
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
    // From QObject
    void timerEvent(QTimerEvent *event);

    // Communication
    void pollPayload();
    void pollMessages();
    void ackMessages();

private slots:
    void requestFinished(QNetworkReply* reply);

private:
    void interpretPayloadReply();
    void interpretMessageReply(QNetworkReply* reply);

    void deletePayload(bool abort);
    void deleteMessage(bool abort);

    QNetworkAccessManager* _manager;
    QNetworkReply* _payloadReply;
    QNetworkReply* _messageReply;
    DMHLogon _logon;
    DMHNetworkObserver::ObserverType _observerType;
    int _timerId;
    int _timerInterval;
};

#endif // DMHNETWORKOBSERVER_PRIVATE_H
