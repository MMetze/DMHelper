#ifndef DMHNETWORKMANAGER_H
#define DMHNETWORKMANAGER_H

#include "dmhglobal.h"
#include <QObject>
#include <memory>

class DMHNetworkObserver;
class DMHPayload;
class DMHLogon;
class QFile;
class QNetworkReply;
class DMHNetworkManager_Private;

class DMHNetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit DMHNetworkManager(const DMHLogon& logon, QObject *parent = nullptr);
    ~DMHNetworkManager();

    DMHNetworkObserver* registerNetworkObserver(QObject *parent = nullptr);

    // Payload Management
    void uploadPayload(const DMHPayload& payload);

    // File Management
    int uploadFile(const QString& filename);
    int fileExists(const QString& fileMD5);
    int uploadData(const QByteArray& data);
    int downloadFile(const QString& fileMD5);

    // Session Management
    int createSession(const QString& sessionName);
    int isSessionOwner(const QString& session = QString());
    int renameSession(const QString& sessionName, const QString & session = QString());
    int removeSession(const QString& session = QString());
    int renewSessionInvite(const QString& session = QString());
    int closeSession(const QString& session = QString());
    int getSessionMembers(const QString& session = QString());

    // User Management
    int createUser(const QString& username, const QString& password, const QString& email, const QString& screenName);
    int joinSession(const QString& invite);

    // Communication
    int sendMessage(const QString& message, const QString& userId = QString());
    int pollMessages();
    int ackMessages();

    // Request controls
    void abortRequest(int id);

    // Access Methods
    const DMHLogon& getLogon() const;
    void setLogon(const DMHLogon& logon);

    QNetworkReply* getNetworkReply(int requestID);

signals:
    // File Management
    void existsComplete(int requestID, const QString& fileMD5, const QString& filename, bool exists);
    void uploadComplete(int requestID, const QString& fileMD5);
    void downloadStarted(int requestID, const QString& fileMD5, QNetworkReply* reply);
    void downloadComplete(int requestID, const QString& fileMD5, const QByteArray& data);

    // Session Management
    void sessionMgmtStarted(int requestID, QNetworkReply* reply, const QString& action, const QString& session, const QString& sessionName);
    void createSessionComplete(int requestID, const QString& session, const QString& invite);
    void isOwnerComplete(int requestID, const QString& session, const QString& sessionName, const QString& invite, bool isOwner);
    void renameSessionComplete(int requestID, const QString& sessionName);
    void renewSessionComplete(int requestID, const QString& sessionName, const QString& invite);
    void closeSessionComplete(int requestID, const QString& sessionName);
    void sessionMembersComplete(int requestID, const QString& sessionName, const QString& members);

    // User Management
    void createUserStarted(int requestID, QNetworkReply* reply, const QString& username, const QString& email, const QString& screenName);
    void createUserComplete(int requestID, const QString& username, const QString& userId, const QString& email);
    void joinSessionStarted(int requestID, QNetworkReply* reply, const QString& username, const QString& invite);
    void joinSessionComplete(int requestID, const QString& session);

    // Request controls
    void otherRequestComplete();
    void messageError(int requestID, const QString& errorString);
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
