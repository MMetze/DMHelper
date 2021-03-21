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

#include <QCryptographicHash>

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

int DMHNetworkManager_Private::fileExists(const QString& fileMD5)
{
    //QUrl serviceUrl = QUrl("https://dmh.wwpd.de/file.php");
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
    //QUrl serviceUrl = QUrl("https://dmh.wwpd.de/file_push_post.php");
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

int DMHNetworkManager_Private::downloadFile(const QString& fileMD5)
{
    //QUrl serviceUrl = QUrl("https://dmh.wwpd.de/file_pull.php");
    QUrl serviceUrl = QUrl(_logon.getURLString() + QString("/file_pull.php"));
    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery postData;
    postData.addQueryItem("user", _logon.getUserName());
    postData.addQueryItem("password", _logon.getPassword());
    postData.addQueryItem("files[][md5]", fileMD5);

#ifdef QT_DEBUG
    emit DEBUG_message_contents(postData.toString(QUrl::FullyEncoded).toUtf8());
#endif

    QNetworkReply* reply = _manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    int replyId = getRequestId(true);
    _replies.insert(reply, replyId);

    qDebug() << "[DMHNetworkManager] Download request sent for file: " << fileMD5 << " with ID " << replyId;

    emit downloadStarted(replyId, fileMD5, reply);

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
    qDebug() << "[DMHNetworkManager] Request with ID " << replyData << "received; payload " << bytes.size() << " bytes";
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
    std::unique_ptr<DMHNetworkData>& factoryData = factory.getData();
    if((!factoryData) || ((factory.getModeValue() != DMHShared::DMH_Message_file_exists) && (!factoryData->isValid())))
    {
        qDebug() << "[DMHNetworkManager] ERROR identified reading raw data: parsed raw data is invalid.";
        emit requestError(replyData);
        return;
    }

    if(isDownload(replyData))
    {
        if(factory.getModeValue() == DMHShared::DMH_Message_file_pull)
        {
            try
            {
                DMHNetworkData_Raw& rawNetworkData = dynamic_cast<DMHNetworkData_Raw&>(*factoryData);
                QByteArray newdata = rawNetworkData.getData();
                qDebug() << "[DMHNetworkManager] Download Complete. Filename: " << rawNetworkData.getName() << ", ID: " << rawNetworkData.getId() << ", Data: " << newdata.size();
                emit downloadComplete(replyData, rawNetworkData.getId(), newdata);
            }
            catch(const std::bad_cast& e)
            {
                Q_UNUSED(e);
                qDebug() << "[DMHNetworkManager] ERROR identified reading raw data: Unexpected failure casting data from download to raw data type!";
                emit requestError(replyData);
                return;
            }
        }
        else if(factory.getModeValue() == DMHShared::DMH_Message_file_exists)
        {
            try
            {
                DMHNetworkData_Exists& existsNetworkData = dynamic_cast<DMHNetworkData_Exists&>(*factoryData);
                qDebug() << "[DMHNetworkManager] File Exist Complete. Filename: " << existsNetworkData.getName() << ", MD5: " << existsNetworkData.getMD5() << ", Exists: " << existsNetworkData.exists();
                emit existsComplete(replyData, existsNetworkData.getMD5(), existsNetworkData.getName(), existsNetworkData.exists());
            }
            catch(const std::bad_cast& e)
            {
                Q_UNUSED(e);
                qDebug() << "[DMHNetworkManager] ERROR identified reading exists data: Unexpected failure casting data from download to raw data type!";
                emit requestError(replyData);
                return;
            }
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

        try
        {
            DMHNetworkData_Upload& uploadNetworkData = dynamic_cast<DMHNetworkData_Upload&>(*factoryData);
            qDebug() << "[DMHNetworkManager] Upload Complete. Filename: " << uploadNetworkData.getName() << ", ID: " << uploadNetworkData.getId();
            emit uploadComplete(replyData, uploadNetworkData.getId());
        }
        catch(const std::bad_cast& e)
        {
            Q_UNUSED(e);
            qDebug() << "[DMHNetworkManager] ERROR identified reading raw data: Unexpected failure casting data from upload to raw data type!";
        }
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
            emit downloadComplete(replyID, QString(""), QByteArray());
        }
        else
        {
            //emit uploadComplete(replyID, QString(""));
            emit requestError(replyID);
        }
    }
}
