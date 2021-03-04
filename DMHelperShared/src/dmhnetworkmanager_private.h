#ifndef DMHNETWORKMANAGER_PRIVATE_H
#define DMHNETWORKMANAGER_PRIVATE_H

#include <QObject>
#include <QMap>
#include "dmhlogon.h"

class DMHNetworkObserver;
class DMHPayload;
class QFile;
class QNetworkAccessManager;
class QNetworkReply;
class QDomElement;

class DMHNetworkManager_Private : public QObject
{
    Q_OBJECT
public:
    explicit DMHNetworkManager_Private(const DMHLogon& logon, QObject *parent = nullptr);
    ~DMHNetworkManager_Private();

    DMHNetworkObserver* registerNetworkObserver(QObject *parent = nullptr);
    void uploadPayload(const DMHPayload& payload);
    int uploadFile(const QString& filename);
    int fileExists(const QString& fileMD5);
    int uploadData(const QByteArray& data);
    int downloadFile(const QString& fileMD5);
    void abortRequest(int id);

    void setLogon(const DMHLogon& logon);

signals:
    void uploadComplete(int requestID, const QString& fileMD5);
    void downloadComplete(int requestID, const QString& fileMD5, const QByteArray& data);
    void existsComplete(int requestID, const QString& fileMD5, const QString& filename, bool exists);
    void requestError(int requestID);

    void DEBUG_message_contents(const QByteArray& data);
    void DEBUG_response_contents(const QByteArray& data);

public slots:

private slots:
    void requestFinished(QNetworkReply* reply);

private:
    int getRequestId(bool isDownload);
    bool isDownload(int id);
    void interpretRequestFinished(QNetworkReply* reply);
    QString outputElements(const QDomElement& element);
    void iterateElement(QString& output, const QDomElement& element);
    void registerRequestError(const QString& errorStr, int replyID);

    QNetworkAccessManager* _manager;
    DMHLogon _logon;
    QMap<QNetworkReply*, int> _replies; // Id, download
    int _lastId;

};

#endif // DMHNETWORKMANAGER_PRIVATE_H
