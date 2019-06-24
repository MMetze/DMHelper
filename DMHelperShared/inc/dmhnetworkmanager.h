#ifndef DMHNETWORKMANAGER_H
#define DMHNETWORKMANAGER_H

#include "dmhglobal.h"
#include <QObject>
#include <memory>

class DMHNetworkObserver;
class DMHPayload;
class DMHLogon;
class QFile;
class DMHNetworkManager_Private;

class DMHNetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit DMHNetworkManager(const DMHLogon& logon, QObject *parent = nullptr);
    ~DMHNetworkManager();

    DMHNetworkObserver* registerNetworkObserver(QObject *parent = nullptr);
    void uploadPayload(const DMHPayload& payload);
    int uploadFile(const QString& filename);
    int fileExists(const QString& fileMD5);
    int uploadData(const QByteArray& data);
    int downloadFile(const QString& fileMD5);

    void setLogon(const DMHLogon& logon);

signals:
    void uploadComplete(int requestID, const QString& fileMD5);
    void downloadComplete(int requestID, const QString& fileMD5, const QByteArray& data);
    void existsComplete(int requestID, const QString& fileMD5, const QString& filename, bool exists);
    void requestError(int requestID);

    void DEBUG_message_contents(const QByteArray& data);
    void DEBUG_response_contents(const QByteArray& data);

public slots:


private:
    // No copy constructor allowed
    DMHNetworkManager(const DMHNetworkManager& other);

    //DMHNetworkManager_Private* d;
    std::unique_ptr<DMHNetworkManager_Private> d;
};

#endif // DMHNETWORKMANAGER_H
