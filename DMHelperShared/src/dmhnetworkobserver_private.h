#ifndef DMHNETWORKOBSERVER_PRIVATE_H
#define DMHNETWORKOBSERVER_PRIVATE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "dmhlogon.h"

class DMHPayload;

class DMHNetworkObserver_Private : public QObject
{
    Q_OBJECT
public:
    explicit DMHNetworkObserver_Private(const DMHLogon& logon, QObject *parent = nullptr);
    ~DMHNetworkObserver_Private();

    void setLogon(const DMHLogon& logon);

signals:
    void payloadReceived(const DMHPayload& payload, const QString& timestamp);

    void DEBUG_message_contents(const QByteArray& data);
    void DEBUG_response_contents(const QByteArray& data);

public slots:
    void start();
    void stop();

protected:
    // From QObject
    void timerEvent(QTimerEvent *event);

private slots:
    void requestFinished(QNetworkReply* reply);

private:
    void interpretReply(QNetworkReply* reply);

    QNetworkAccessManager* _manager;
    QNetworkReply* _reply;
    DMHLogon _logon;
    int _timerId;
};

#endif // DMHNETWORKOBSERVER_PRIVATE_H
