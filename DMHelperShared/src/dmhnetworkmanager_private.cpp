#include "dmhnetworkmanager_private.h"
#include "dmhnetworkobserver.h"
#include "dmhpayload.h"
#include "dmhnetworkdatafactory.h"
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QHttpMultiPart>
#include <QDomDocument>
#include <QDomElement>
#include <QDebug>

const int VALID_REQUEST_ID = 100;

DMHNetworkManager_Private::DMHNetworkManager_Private(const DMHLogon& logon, QObject *parent) :
    QObject(parent),
    _manager(nullptr),
    _logon(logon),
    _replies(),
    _lastId(VALID_REQUEST_ID)
{
    _manager = new QNetworkAccessManager(this);
    connect(_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)));
}

DMHNetworkManager_Private::~DMHNetworkManager_Private()
{
    for(QNetworkReply* reply : _replies.keys())
    {
        delete reply;
    }
}

DMHNetworkObserver* DMHNetworkManager_Private::registerNetworkObserver(QObject *parent)
{
    return new DMHNetworkObserver(_logon, parent);
}

void DMHNetworkManager_Private::uploadPayload(const DMHPayload& payload)
{
    QUrl serviceUrl = QUrl(_logon.getURLString() + QString("/pll_dm.php"));
    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    qDebug() << "[DMHNetworkManager] Uploading payload. Image: " << payload.getImageFile() << ", Audio: " << payload.getAudioFile();

    QUrlQuery postData;
    postData.addQueryItem("user", _logon.getUserName());
    postData.addQueryItem("password", _logon.getPassword());
    postData.addQueryItem("session", _logon.getSession());
    QString payloadString = payload.toString();
    postData.addQueryItem("payload", payloadString);

    QString postDataString = postData.toString(QUrl::FullyEncoded);
    QByteArray postDataArray = postDataString.toUtf8();
    qDebug() << "[DMHNetworkManager] Uploading payload. Request: " << postDataArray;

#ifdef QT_DEBUG
    emit DEBUG_message_contents(postDataArray);
#endif

    _manager->post(request, postDataArray);
}

int DMHNetworkManager_Private::uploadFile(const QString& filename)
{
    QFile file(filename);

    if(!file.exists())
    {
        qDebug() << "[DMHNetworkManager] ERROR: Upload file does not exist: " << filename;
        return -1;
    }

    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[DMHNetworkManager] ERROR: Upload file cannot be opened: " << filename;
        return -1;
    }

    QByteArray readData = file.readAll();
    if(readData.size() <= 0)
    {
        qDebug() << "[DMHNetworkManager] ERROR: Upload file cannot be read or has zero size: " << filename;
        return -1;
    }

    file.close();

    int replyId = uploadData(readData);

    qDebug() << "[DMHNetworkManager] Uploading file: " << filename << " with ID " << replyId;

    return replyId;
}

int DMHNetworkManager_Private::fileExists(const QString& fileMD5, const QString& fileUuid)
{
    Q_UNUSED(fileUuid);

    QUrl serviceUrl = QUrl(_logon.getURLString() + QString("/file.php"));
    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery postData;
    postData.addQueryItem("user", _logon.getUserName());
    postData.addQueryItem("password", _logon.getPassword());
    postData.addQueryItem("action", "exists");
    postData.addQueryItem("fileids[]", fileMD5);

#ifdef QT_DEBUG
    emit DEBUG_message_contents(postData.toString(QUrl::FullyEncoded).toUtf8());
#endif

    QNetworkReply* reply = _manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    int replyId = getRequestId(true);
    _replies.insert(reply, replyId);

    qDebug() << "[DMHNetworkManager] File exists request sent for file: " << fileMD5 << " with ID " << replyId;

    return replyId;
}

int DMHNetworkManager_Private::uploadData(const QByteArray& data)
{
    QUrl serviceUrl = QUrl(_logon.getURLString() + QString("/file_push_post.php"));
    QNetworkRequest request(serviceUrl);

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart userNamePart;
    //userNamePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant( "text/plain"));
    userNamePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"user\""));
    userNamePart.setBody(_logon.getUserName().toUtf8());
    multiPart->append(userNamePart);

    QHttpPart passwordPart;
    //passwordPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant( "text/plain"));
    passwordPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"password\""));
    passwordPart.setBody(_logon.getPassword().toUtf8());
    multiPart->append(passwordPart);

    /*
#ifdef QT_DEBUG
    QHttpPart debugPart;
    debugPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant( "text/plain"));
    debugPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"debug\""));
    debugPart.setBody(QString("1").toUtf8());
    multiPart->append(debugPart);
#endif
*/

    QHttpPart filePart;
    //filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant( "text/plain"));
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"myFile[]\"; filename=\"internal\""));
    filePart.setBody(data);
    multiPart->append(filePart);

#ifdef QT_DEBUG
    // TODO: String representation in
    // emit DEBUG_message_contents(postData.toString(QUrl::FullyEncoded).toUtf8());
#endif

    QNetworkReply* reply = _manager->post(request, multiPart);

    int replyId = getRequestId(false);
    _replies.insert(reply, replyId);

    qDebug() << "[DMHNetworkManager] Uploading " << data.size() << " bytes of data with ID " << replyId;

    return replyId;
}

int DMHNetworkManager_Private::downloadFile(const QString& md5, const QString& uuid)
{
//    QUrl serviceUrl = QUrl(_logon.getURLString() + QString("/file_pull.php"));
    QUrl serviceUrl = QUrl(_logon.getURLString() + QString("/file.php"));
    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery postData;
    postData.addQueryItem("user", _logon.getUserName());
    postData.addQueryItem("password", _logon.getPassword());
    postData.addQueryItem("action", "pull");
    //postData.addQueryItem("files[][md5]", md5);
    postData.addQueryItem("fileids[]", uuid);

#ifdef QT_DEBUG
    emit DEBUG_message_contents(postData.toString(QUrl::FullyEncoded).toUtf8());
#endif

    QNetworkReply* reply = _manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    int replyId = getRequestId(true);
    _replies.insert(reply, replyId);

    qDebug() << "[DMHNetworkManager] Download request sent for file: " << md5 << ", " << uuid << " with ID " << replyId;

    emit downloadStarted(replyId, md5, uuid, reply);

    return replyId;
}

// action create (creates new session)
//  <session> -> Session UUID
//    <code> -> invite code for players
int DMHNetworkManager_Private::createSession(const QString & sessionName)
{
    return sendSessionMgmt(QString("create"), QString(), sessionName);
}

// action isowner (checks if user is owner of session)
//   <owner> -> true/false (1, NULL)
int DMHNetworkManager_Private::isSessionOwner(const QString & session)
{
    return sendSessionMgmt(QString("isowner"), session, QString());
}

// action rename (renames session)
//    <session> -> Session UUID
//      <renamed> -> new Session namespace
int DMHNetworkManager_Private::renameSession(const QString & sessionName, const QString & session)
{
    return sendSessionMgmt(QString("rename"), session, sessionName);
}

// action remove (deletes session)
//     <session> -> Session UUID
//     <removed> -> 'ok' on success
int DMHNetworkManager_Private::removeSession(const QString & session)
{
    return sendSessionMgmt(QString("remove"), session, QString());
}

// action renew (creates new invite code, invalidates old one)
//     <session> -> Session UUID
//     <code> -> invite code
int DMHNetworkManager_Private::renewSessionInvite(const QString & session)
{
    return sendSessionMgmt(QString("renew"), session, QString());
}

// action close (invalidates invite codes, does not create a new one)
//     <session> -> Session UUID
int DMHNetworkManager_Private::closeSession(const QString & session)
{
    return sendSessionMgmt(QString("close"), session, QString());
}

// action member (returns members of session)
//     <session> -> Session UUID
//     <members> -> <data>* -> 	<username>
int DMHNetworkManager_Private::getSessionMembers(const QString & session)
{
    return sendSessionMgmt(QString("member"), session, QString());
}

int DMHNetworkManager_Private::createUser(const QString& username, const QString& password, const QString& email, const QString& screenName)
{
    QUrl serviceUrl = QUrl(_logon.getURLString() + QString("/usr_mng.php"));
    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery postData;
    postData.addQueryItem("user", username);
    postData.addQueryItem("password", password);
    postData.addQueryItem("action", "create");
    postData.addQueryItem("email", email);
    postData.addQueryItem("surname", screenName);

#ifdef QT_DEBUG
    emit DEBUG_message_contents(postData.toString(QUrl::FullyEncoded).toUtf8());
#endif
    qDebug() << postData.toString(QUrl::FullyEncoded).toUtf8();

    QNetworkReply* reply = _manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    int replyId = getRequestId(true);
    _replies.insert(reply, replyId);

    qDebug() << "[DMHNetworkManager] Create user request sent with ID " << replyId << " for username: " << username << " with email: " << email << " and screen name: " << screenName;

    emit createUserStarted(replyId, reply, username, email, screenName);

    return replyId;
}

int DMHNetworkManager_Private::joinSession(const QString& invite)
{
    QUrl serviceUrl = QUrl(_logon.getURLString() + QString("/ssn_ass.php"));
    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery postData;
    postData.addQueryItem("user", _logon.getUserName());
    postData.addQueryItem("password", _logon.getPassword());
    postData.addQueryItem("action", "join");
    postData.addQueryItem("code", invite);

#ifdef QT_DEBUG
    emit DEBUG_message_contents(postData.toString(QUrl::FullyEncoded).toUtf8());
#endif
    qDebug() << postData.toString(QUrl::FullyEncoded).toUtf8();

    QNetworkReply* reply = _manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    int replyId = getRequestId(true);
    _replies.insert(reply, replyId);

    qDebug() << "[DMHNetworkManager] Join session request sent with ID " << replyId << " for username: " << _logon.getUserName()<< " with invite: " << invite;

    emit joinSessionStarted(replyId, reply, _logon.getUserName(), invite);

    return replyId;
}

int DMHNetworkManager_Private::sendMessage(const QString& message, const QString& userId)
{
    QUrl serviceUrl = QUrl(_logon.getURLString() + QString("/communication.php"));
    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery postData;
    //postData.addQueryItem("user", _logon.getUserId());
    // TODO : replace hard-coded string with real userId!!
    postData.addQueryItem("user", QString("44C79028-2817-4F60-A0D2-ED88B62D895D"));
    postData.addQueryItem("password", _logon.getPassword());
    postData.addQueryItem("session", _logon.getSession());
    postData.addQueryItem("action", "send");
    if(userId.isEmpty())
    {
        postData.addQueryItem("type", "session");
    }
    else
    {
        postData.addQueryItem("type", "user");
        postData.addQueryItem("target", userId);
    }
    postData.addQueryItem("body", message);

#ifdef QT_DEBUG
    emit DEBUG_message_contents(postData.toString(QUrl::FullyEncoded).toUtf8());
#endif
    qDebug() << "[DMHNetworkManager] " << postData.toString(QUrl::FullyEncoded).toUtf8();

    QNetworkReply* reply = _manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    int replyId = getRequestId(true);
    _replies.insert(reply, replyId);

    qDebug() << "[DMHNetworkManager] Message sent: " << replyId << " to: " << userId << " with contents: " << message;

    return replyId;
}

int DMHNetworkManager_Private::pollMessages()
{
    QUrl serviceUrl = QUrl(_logon.getURLString() + QString("/communication.php"));
    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery postData;
    postData.addQueryItem("user", _logon.getUserName());
    postData.addQueryItem("password", _logon.getPassword());
    postData.addQueryItem("session", _logon.getSession());
    postData.addQueryItem("action", "poll");

#ifdef QT_DEBUG
    emit DEBUG_message_contents(postData.toString(QUrl::FullyEncoded).toUtf8());
#endif
    qDebug() << postData.toString(QUrl::FullyEncoded).toUtf8();

    QNetworkReply* reply = _manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    int replyId = getRequestId(true);
    _replies.insert(reply, replyId);

    qDebug() << "[DMHNetworkManager] Poll message request send.";

    return replyId;
}

int DMHNetworkManager_Private::ackMessages()
{
    QUrl serviceUrl = QUrl(_logon.getURLString() + QString("/communication.php"));
    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery postData;
    postData.addQueryItem("user", _logon.getUserName());
    postData.addQueryItem("password", _logon.getPassword());
    postData.addQueryItem("session", _logon.getSession());
    postData.addQueryItem("action", "ack");

#ifdef QT_DEBUG
    emit DEBUG_message_contents(postData.toString(QUrl::FullyEncoded).toUtf8());
#endif
    qDebug() << postData.toString(QUrl::FullyEncoded).toUtf8();

    QNetworkReply* reply = _manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    int replyId = getRequestId(true);
    _replies.insert(reply, replyId);

    qDebug() << "[DMHNetworkManager] Ack message request sent.";

    return replyId;
}

void DMHNetworkManager_Private::abortRequest(int id)
{
    QNetworkReply* reply = _replies.key(id, nullptr);
    if(reply)
        reply->abort();
}

const DMHLogon& DMHNetworkManager_Private::getLogon() const
{
    return _logon;
}

void DMHNetworkManager_Private::setLogon(const DMHLogon& logon)
{
    _logon = logon;
}

QNetworkReply* DMHNetworkManager_Private::getNetworkReply(int requestID)
{
    return _replies.key(requestID);
}

void DMHNetworkManager_Private::requestFinished(QNetworkReply* reply)
{
    if(!reply)
        return;

    interpretRequestFinished(reply);

    _replies.remove(reply);
    reply->deleteLater();
}

int DMHNetworkManager_Private::getRequestId(bool isDownload)
{
    return ((_lastId++ * 10) + (isDownload ? 0 : 1));
}

bool DMHNetworkManager_Private::isDownload(int id)
{
    return (id % 10 == 0);
}

void DMHNetworkManager_Private::interpretRequestFinished(QNetworkReply* reply)
{
    int replyData = _replies.value(reply);

    if(reply->error() != QNetworkReply::NoError)
    {
        registerRequestError(QString("[DMHNetworkManager] ERROR identified in network reply: ") + QString::number(replyData) + QString(", Error: ") + QString::number(reply->error()) + QString(", Error string ") + reply->errorString(), replyData);
        return;
    }

    QByteArray bytes = reply->readAll();
    //qDebug() << "[DMHNetworkManager] Request with ID " << replyData << "received; payload " << bytes.size() << " bytes";
    qDebug() << "[DMHNetworkManager] Payload contents: " << QString(bytes.left(2000));

#ifdef QT_DEBUG
    emit DEBUG_response_contents(bytes.left(2000));
#endif

    if(replyData < VALID_REQUEST_ID)
    {
        //registerRequestError(QString("[DMHNetworkManager] Request with blank ID received: ") + QString::number(replyData), replyData);
        emit otherRequestComplete();
        return;
    }

    DMHNetworkDataFactory factory(bytes);
    if(factory.getStateValue() == DMHShared::DMH_Message_State_Error)
    {
        qDebug() << "[DMHNetworkManager] Error in server response: " << factory.getErrorString();
        emit messageError(replyData, factory.getErrorString());
        return;
    }

    std::unique_ptr<DMHNetworkData>& factoryData = factory.getData();
    if((!factoryData) || ((factory.getModeValue() != DMHShared::DMH_Message_file_exists) && (!factoryData->isValid())))
    {
        qDebug() << "[DMHNetworkManager] ERROR identified reading raw data: parsed raw data is invalid.";
        emit requestError(replyData);
        return;
    }

    try
    {
        if(isDownload(replyData))
        {
            if(factory.getModeValue() == DMHShared::DMH_Message_ssn_ass)
            {
                DMHNetworkData_JoinSession& joinSessionNetworkData = dynamic_cast<DMHNetworkData_JoinSession&>(*factoryData);
                qDebug() << "[DMHNetworkManager] Join Session Received. Session: " << joinSessionNetworkData.getSession();
                emit joinSessionComplete(replyData, joinSessionNetworkData.getSession());
            }
            else if(factory.getModeValue() == DMHShared::DMH_Message_file_pull)
            {
                DMHNetworkData_Raw& rawNetworkData = dynamic_cast<DMHNetworkData_Raw&>(*factoryData);
                QByteArray newdata = rawNetworkData.getData();
                qDebug() << "[DMHNetworkManager] Download Complete. Filename: " << rawNetworkData.getName() << ", md5: " << rawNetworkData.getMD5() << ", UUID: " << rawNetworkData.getUuid() << ", Data: " << newdata.size();
                emit downloadComplete(replyData, rawNetworkData.getMD5(), rawNetworkData.getUuid(), newdata);
            }
            else if(factory.getModeValue() == DMHShared::DMH_Message_file_exists)
            {
                DMHNetworkData_Exists& existsNetworkData = dynamic_cast<DMHNetworkData_Exists&>(*factoryData);
                qDebug() << "[DMHNetworkManager] File Exist Complete. Filename: " << existsNetworkData.getName() << ", MD5: " << existsNetworkData.getMD5() << ", Exists: " << existsNetworkData.exists();
                emit existsComplete(replyData, existsNetworkData.getMD5(), existsNetworkData.getUuid(), existsNetworkData.getName(), existsNetworkData.exists());
            }
            else if(factory.getModeValue() == DMHShared::DMH_Message_ssn_create)
            {
                DMHNetworkData_CreateSession& createSessionNetworkData = dynamic_cast<DMHNetworkData_CreateSession&>(*factoryData);
                qDebug() << "[DMHNetworkManager] Create Session Complete. Session: " << createSessionNetworkData.getSession() << ", Invite: " << createSessionNetworkData.getInvite();
                emit createSessionComplete(replyData, createSessionNetworkData.getSession(), createSessionNetworkData.getInvite());
            }
            else if(factory.getModeValue() == DMHShared::DMH_Message_ssn_isowner)
            {
                DMHNetworkData_IsOwner& isOwnerNetworkData = dynamic_cast<DMHNetworkData_IsOwner&>(*factoryData);
                qDebug() << "[DMHNetworkManager] Is Owner Complete. Session: " << isOwnerNetworkData.getSession() << ", User: " << isOwnerNetworkData.getUser() << ", Is Owner: " << isOwnerNetworkData.isOwner();
                emit isOwnerComplete(replyData, isOwnerNetworkData.getSession(), isOwnerNetworkData.getSessionName(), isOwnerNetworkData.getInvite(), isOwnerNetworkData.isOwner());
            }
            else if(factory.getModeValue() == DMHShared::DMH_Message_ssn_rename)
            {
                DMHNetworkData_RenameSession& renameSessionNetworkData = dynamic_cast<DMHNetworkData_RenameSession&>(*factoryData);
                qDebug() << "[DMHNetworkManager] Rename Session Complete. Name: " << renameSessionNetworkData.getName();
                emit renameSessionComplete(replyData, renameSessionNetworkData.getName());
            }
            else if(factory.getModeValue() == DMHShared::DMH_Message_ssn_renew)
            {
                DMHNetworkData_RenewSession& renewSessionNetworkData = dynamic_cast<DMHNetworkData_RenewSession&>(*factoryData);
                qDebug() << "[DMHNetworkManager] Renew Session Complete. Session: " << renewSessionNetworkData.getSession() << ", Invite: " << renewSessionNetworkData.getInvite();
                emit renewSessionComplete(replyData, renewSessionNetworkData.getSession(), renewSessionNetworkData.getInvite());
            }
            else if(factory.getModeValue() == DMHShared::DMH_Message_ssn_close)
            {
                DMHNetworkData_CloseSession& closeSessionNetworkData = dynamic_cast<DMHNetworkData_CloseSession&>(*factoryData);
                qDebug() << "[DMHNetworkManager] Close Session Complete. Session: " << closeSessionNetworkData.getSession();
                emit closeSessionComplete(replyData, closeSessionNetworkData.getSession());
            }
            else if(factory.getModeValue() == DMHShared::DMH_Message_ssn_members)
            {
                DMHNetworkData_SessionMembers& sessionMembersNetworkData = dynamic_cast<DMHNetworkData_SessionMembers&>(*factoryData);
                //qDebug() << "[DMHNetworkManager] Session Members Received. Session: " << sessionMembersNetworkData.getSession() << ", Members: " << sessionMembersNetworkData.getMembers();
                emit sessionMembersComplete(replyData, sessionMembersNetworkData.getSession(), sessionMembersNetworkData.getMembers());
            }
            else if(factory.getModeValue() == DMHShared::DMH_Message_usr_create)
            {
                DMHNetworkData_CreateUser& createUserNetworkData = dynamic_cast<DMHNetworkData_CreateUser&>(*factoryData);
                qDebug() << "[DMHNetworkManager] Create User Received. Session: " << createUserNetworkData.getUsername() << ", Email: " << createUserNetworkData.getEmail();
                emit createUserComplete(replyData, createUserNetworkData.getUsername(), createUserNetworkData.getUserId(), createUserNetworkData.getEmail());
            }
            else if(factory.getModeValue() == DMHShared::DMH_Message_msg_send)
            {
                DMHNetworkData_Message& messageNetworkData = dynamic_cast<DMHNetworkData_Message&>(*factoryData);
                qDebug() << "[DMHNetworkManager] Message Send Received. Session: " << messageNetworkData.getData();
                emit sendMessageComplete(replyData, messageNetworkData.getData());
            }
            else if(factory.getModeValue() == DMHShared::DMH_Message_msg_poll)
            {
                DMHNetworkData_Message& messageNetworkData = dynamic_cast<DMHNetworkData_Message&>(*factoryData);
                qDebug() << "[DMHNetworkManager] Message Poll Received. Session: " << messageNetworkData.getData();
                emit pollMessageComplete(replyData, messageNetworkData.getData());
            }
            else if(factory.getModeValue() == DMHShared::DMH_Message_msg_ack)
            {
                DMHNetworkData_Message& messageNetworkData = dynamic_cast<DMHNetworkData_Message&>(*factoryData);
                qDebug() << "[DMHNetworkManager] Message Ack Received. Session: " << messageNetworkData.getData();
                emit ackMessageComplete(replyData, messageNetworkData.getData());
            }
            else
            {
                qDebug() << "[DMHNetworkManager] ERROR identified reading download data: unable to identify data from download.";
                emit requestError(replyData);
                return;
            }
        }
        else
        {
            if(factory.getModeValue() != DMHShared::DMH_Message_file_push)
            {
                qDebug() << "[DMHNetworkManager] ERROR identified reading raw data: unable to identify data from upload.";
                emit requestError(replyData);
                return;
            }

            DMHNetworkData_Upload& uploadNetworkData = dynamic_cast<DMHNetworkData_Upload&>(*factoryData);
            qDebug() << "[DMHNetworkManager] Upload Complete. Filename: " << uploadNetworkData.getName() << ", md5: " << uploadNetworkData.getMD5() << ", uuid: " << uploadNetworkData.getUuid();
            emit uploadComplete(replyData, uploadNetworkData.getMD5(), uploadNetworkData.getUuid());
        }
    }
    catch(const std::bad_cast&)
    {
        qDebug() << "[DMHNetworkManager] ERROR identified reading raw data: Unexpected failure casting data from upload to raw data type!";
        emit requestError(replyData);
        return;
    }
}

QString DMHNetworkManager_Private::outputElements(const QDomElement& element)
{
    QString result;

    iterateElement(result, element);

    return result;
}

void DMHNetworkManager_Private::iterateElement(QString& output, const QDomElement& element)
{
    output += element.tagName();
    output += QString("<");
    QDomElement e = element.firstChildElement();
    while(!e.isNull())
    {
        iterateElement(output, e);
        e = e.nextSiblingElement();
    }
    output += QString(">");
}

void DMHNetworkManager_Private::registerRequestError(const QString& errorStr, int replyID)
{
    qDebug() << errorStr;
    if(replyID >= VALID_REQUEST_ID)
    {
        if(isDownload(replyID))
        {
            emit downloadComplete(replyID, QString(""), QString(""), QByteArray());
        }
        else
        {
            //emit uploadComplete(replyID, QString(""));
            emit requestError(replyID);
        }
    }
}

int DMHNetworkManager_Private::sendSessionMgmt(const QString& action, const QString& session, const QString& sessionName)
{
    QUrl serviceUrl = QUrl(_logon.getURLString() + QString("/ssn_mng.php"));
    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery postData;
    postData.addQueryItem("user", _logon.getUserName());
    postData.addQueryItem("password", _logon.getPassword());
    postData.addQueryItem("action", action);
    postData.addQueryItem("session", session.isEmpty() ? _logon.getSession() : session);
    if(!sessionName.isEmpty())
        postData.addQueryItem("name", sessionName);

#ifdef QT_DEBUG
    emit DEBUG_message_contents(postData.toString(QUrl::FullyEncoded).toUtf8());
#endif

    QNetworkReply* reply = _manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    int replyId = getRequestId(true);
    _replies.insert(reply, replyId);

    if(action != QString("member"))
        qDebug() << "[DMHNetworkManager] Session management request sent: " << action << " with ID " << replyId << " for session " << session << " with name " << sessionName;

    emit sessionMgmtStarted(replyId, reply, action, session, sessionName);

    return replyId;
}
