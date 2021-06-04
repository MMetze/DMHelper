#include "dmhnetworkobserver_private.h"
#include "dmhpayload.h"
#include "dmhpayload_private.h"
#include "dmhnetworkdatafactory.h"
#include <QUrlQuery>

DMHNetworkObserver_Private::DMHNetworkObserver_Private(const DMHLogon& logon, DMHNetworkObserver::ObserverType observerType, QObject *parent) :
    QObject(parent),
    _manager(nullptr),
    _payloadReply(nullptr),
    _messageReply(nullptr),
    _logon(logon),
    _observerType(observerType),
    _timerId(0),
    _timerInterval(1000)
{
    _manager = new QNetworkAccessManager(this);
    connect(_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)));
}

DMHNetworkObserver_Private::~DMHNetworkObserver_Private()
{
    stop();
}

void DMHNetworkObserver_Private::setLogon(const DMHLogon& logon)
{
    _logon = logon;
}

void DMHNetworkObserver_Private::setObserverType(DMHNetworkObserver::ObserverType observerType)
{
    if(_observerType == observerType)
        return;

    _observerType = observerType;
    if((_payloadReply) && ((_observerType & DMHNetworkObserver::ObserverType_Payload) == 0))
        deletePayload(true);

    if((_messageReply) && ((_observerType & DMHNetworkObserver::ObserverType_Message) == 0))
        deleteMessage(true);
}

void DMHNetworkObserver_Private::setInterval(int interval)
{
    if(interval <= 0)
        return;

    _timerInterval = interval;
    if(_timerId)
    {
        killTimer(_timerId);
        _timerId = startTimer(_timerInterval);
    }
}

void DMHNetworkObserver_Private::start()
{
    if(_timerId == 0)
        _timerId = startTimer(_timerInterval);
}

void DMHNetworkObserver_Private::stop()
{
    if(_timerId != 0)
    {
        killTimer(_timerId);
        _timerId = 0;
    }

    deletePayload(true);
    deleteMessage(true);
}

void DMHNetworkObserver_Private::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    if(_logon.getUserId().isEmpty())
        return;

    if((_observerType & DMHNetworkObserver::ObserverType_Payload) == DMHNetworkObserver::ObserverType_Payload)
    {
        pollPayload();
    }

    if((_observerType & DMHNetworkObserver::ObserverType_Message) == DMHNetworkObserver::ObserverType_Message)
    {
        pollMessages();
    }
}

void DMHNetworkObserver_Private::pollPayload()
{
    if(_payloadReply)
        return;

    QUrl serviceUrl = QUrl(_logon.getURLString() + QString("/pll_player.php"));
    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery postData;
    // postData.addQueryItem("user", _logon.getUserName());
    postData.addQueryItem("user", _logon.getUserId());
    postData.addQueryItem("password", _logon.getPassword());
    postData.addQueryItem("session", _logon.getSession());

#ifdef QT_DEBUG
    emit DEBUG_message_contents(postData.toString(QUrl::FullyEncoded).toUtf8());
#endif

    _payloadReply = _manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
}

void DMHNetworkObserver_Private::pollMessages()
{
    if(_messageReply)
        return;

    QUrl serviceUrl = QUrl(_logon.getURLString() + QString("/communication.php"));
    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery postData;
    postData.addQueryItem("user", _logon.getUserId());
    postData.addQueryItem("password", _logon.getPassword());
    postData.addQueryItem("session", _logon.getSession());
    postData.addQueryItem("action", "poll");

#ifdef QT_DEBUG
    emit DEBUG_message_contents(postData.toString(QUrl::FullyEncoded).toUtf8());
#endif
    //qDebug() << postData.toString(QUrl::FullyEncoded).toUtf8();

    _messageReply = _manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
}

void DMHNetworkObserver_Private::ackMessages()
{
    QUrl serviceUrl = QUrl(_logon.getURLString() + QString("/communication.php"));
    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery postData;
    postData.addQueryItem("user", _logon.getUserId());
    postData.addQueryItem("password", _logon.getPassword());
    postData.addQueryItem("session", _logon.getSession());
    postData.addQueryItem("action", "ack");

#ifdef QT_DEBUG
    emit DEBUG_message_contents(postData.toString(QUrl::FullyEncoded).toUtf8());
#endif
    //qDebug() << postData.toString(QUrl::FullyEncoded).toUtf8();

    _manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
}

void DMHNetworkObserver_Private::requestFinished(QNetworkReply* reply)
{
    if(!reply)
        return;

    if(reply == _payloadReply)
    {
        interpretPayloadReply();
        deletePayload(false);
    }
    else
    {
        interpretMessageReply(reply);
         if(reply == _messageReply)
            deleteMessage(false);
        else
            reply->deleteLater();
    }
}

void DMHNetworkObserver_Private::interpretPayloadReply()
{
    if(!_payloadReply)
        return;

    if(_payloadReply->error() != QNetworkReply::NoError)
    {
        qDebug() << "[DMHNetworkObserver] ERROR identified in payload network reply: " << _payloadReply->error() << ", Error string " << _payloadReply->errorString();
        return;
    }

    QByteArray bytes = _payloadReply->readAll();
    if(bytes.size() <= 0)
    {
        qDebug() << "[DMHNetworkObserver] ERROR identified reading payload: 0 bytes in network reply";
        return;
    }

#ifdef DMH_VERBOSE
    qDebug() << "[DMHNetworkObserver] Payload contents: " << QString(bytes.left(1000));
#endif

#ifdef QT_DEBUG
    emit DEBUG_response_contents(bytes.left(2000));
#endif

    DMHNetworkDataFactory factory(bytes);
    if(factory.getModeValue() != DMHShared::DMH_Message_pl_pull)
    {
        qDebug() << "[DMHNetworkObserver] ERROR identified reading payload: unable to identify payload data!";
        return;
    }

    std::unique_ptr<DMHNetworkData>& factoryData = factory.getData();
    if(!factoryData->isValid())
    {
        qDebug() << "[DMHNetworkObserver] ERROR identified reading payload data: parsed payload data is invalid!";
        return;
    }

    try
    {
        DMHNetworkData_Payload& payloadNetworkData = dynamic_cast<DMHNetworkData_Payload&>(*factoryData);
        const DMHPayload& payload = payloadNetworkData.getPayload();
        emit payloadReceived(payload, payloadNetworkData.getTimestamp());
    }
    catch(const std::bad_cast& e)
    {
        Q_UNUSED(e);
        qDebug() << "[DMHNetworkObserver] ERROR identified reading payload data: Unexpected failure casting data from download to payload data type!";
    }
}

void DMHNetworkObserver_Private::interpretMessageReply(QNetworkReply* reply)
{
    if(!reply)
        return;

    if(reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "[DMHNetworkObserver] ERROR identified in network reply, Error: " << reply->error() << ", Error string " << reply->errorString();
        return;
    }

    QByteArray bytes = reply->readAll();
    if(bytes.size() <= 0)
    {
        qDebug() << "[DMHNetworkObserver] ERROR identified reading payload: 0 bytes in network reply";
        return;
    }

#ifdef DMH_VERBOSE
    qDebug() << "[DMHNetworkObserver] Payload contents: " << QString(bytes.left(1000));
#endif

#ifdef QT_DEBUG
    emit DEBUG_response_contents(bytes.left(2000));
#endif

    DMHNetworkDataFactory factory(bytes);
    if(factory.getStateValue() == DMHShared::DMH_Message_State_Error)
    {
        qDebug() << "[DMHNetworkObserver] Error in server response: " << factory.getErrorString();
        return;
    }

    std::unique_ptr<DMHNetworkData>& factoryData = factory.getData();
    if((!factoryData) || (!factoryData->isValid()))
    {
        qDebug() << "[DMHNetworkObserver] ERROR identified reading raw data: parsed raw data is invalid.";
        return;
    }

    try
    {
        if(factory.getModeValue() == DMHShared::DMH_Message_msg_poll)
        {
            DMHNetworkData_Message& messageNetworkData = dynamic_cast<DMHNetworkData_Message&>(*factoryData);
            if(!messageNetworkData.getMessages().isEmpty())
            {
                qDebug() << "[DMHNetworkObserver] Message Poll Received. Messages: " << messageNetworkData.getMessages().count();
                emit messageReceived(messageNetworkData.getMessages());
                // TODO: make sure only the latest known messages are acked, not any new ones
                ackMessages();
            }
        }
        else if(factory.getModeValue() == DMHShared::DMH_Message_msg_ack)
        {
            DMHNetworkData_SimpleMessage& messageNetworkData = dynamic_cast<DMHNetworkData_SimpleMessage&>(*factoryData);
            qDebug() << "[DMHNetworkObserver] Message Ack Received. Data: " << messageNetworkData.getData();
        }
    }
    catch(const std::bad_cast&)
    {
        qDebug() << "[DMHNetworkObserver] ERROR identified reading raw data: Unexpected failure casting data from upload to raw data type!";
    }
}

void DMHNetworkObserver_Private::deletePayload(bool abort)
{
    if(!_payloadReply)
        return;

    QNetworkReply* deleteReply = _payloadReply;
    _payloadReply = nullptr;

    if(abort)
        deleteReply->abort();
    deleteReply->deleteLater();
}

void DMHNetworkObserver_Private::deleteMessage(bool abort)
{
    if(!_messageReply)
        return;

    QNetworkReply* deleteReply = _messageReply;
    _messageReply = nullptr;

    if(abort)
        deleteReply->abort();
    deleteReply->deleteLater();
}
