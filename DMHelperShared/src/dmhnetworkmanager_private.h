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

/*
        Management interface for sessions

        ssn_mng.php
        POST Vars
        user -> str, req
        password -> str, req
        session -> str, UUID, opt
        action -> str, req
        name -> str, opt

        returns XML
        <status> -> tells if last action was successful or not
        <error> -> in case of error, holds the kind of error
        - action create (creates new session)
          <session> -> Session UUID
            <code> -> invite code for players
        - action isowner (checks if user is owner of session)
          <owner> -> true/false (1, NULL)
        - action rename (renames session)
          <session> -> Session UUID
            <renamed> -> new Session namespace
        - action remove (deletes session)
            <session> -> Session UUID
            <removed> -> 'ok' on success
        - action renew (creates new invite code, invalidates old one)
            <session> -> Session UUID
            <code> -> invite code
        - action close (invalidates invite codes, does not create a new one)
            <session> -> Session UUID
        - action member (returns members of session)
            <session> -> Session UUID
            <members> -> <data>* -> 	<username>
                                                            <surname>
                                                            <forename>

*/

class DMHNetworkManager_Private : public QObject
{
    Q_OBJECT
public:
    explicit DMHNetworkManager_Private(const DMHLogon& logon, QObject *parent = nullptr);
    ~DMHNetworkManager_Private();

    DMHNetworkObserver* registerNetworkObserver(QObject *parent = nullptr);

    // Payload Management
    void uploadPayload(const DMHPayload& payload);

    // File Management
    int uploadFile(const QString& filename);
    int fileExists(const QString& fileMD5, const QString& fileUuid);
    int uploadData(const QByteArray& data);
    int downloadFile(const QString& md5, const QString& uuid);

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
    void existsComplete(int requestID, const QString& fileMD5, const QString& fileUuid, const QString& filename, bool exists);
    void uploadComplete(int requestID, const QString& fileMD5, const QString& fileUuid);
    void downloadStarted(int requestID, const QString& fileMD5, const QString& fileUuid, QNetworkReply* reply);
    void downloadComplete(int requestID, const QString& fileMD5, const QString& fileUuid, const QByteArray& data);

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

    // Communication
    void sendMessageComplete(int requestID, const QString& data);
    void pollMessageComplete(int requestID, const QString& data);
    void ackMessageComplete(int requestID, const QString& data);

    // Request controls
    void otherRequestComplete();
    void messageError(int requestID, const QString& errorString);
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

    int sendSessionMgmt(const QString& action, const QString& session = QString(), const QString& sessionName = QString());


    QNetworkAccessManager* _manager;
    DMHLogon _logon;
    QMap<QNetworkReply*, int> _replies; // Id, download
    int _lastId;

};

#endif // DMHNETWORKMANAGER_PRIVATE_H
