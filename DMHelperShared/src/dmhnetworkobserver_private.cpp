#include "dmhnetworkobserver_private.h"
#include "dmhpayload.h"
#include "dmhpayload_private.h"
#include "dmhnetworkdatafactory.h"
#include <QUrlQuery>

DMHNetworkObserver_Private::DMHNetworkObserver_Private(const DMHLogon& logon, QObject *parent) :
    QObject(parent),
    _manager(nullptr),
    _reply(nullptr),
    _logon(logon),
    _timerId(0)
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

void DMHNetworkObserver_Private::start()
{
    if(_timerId == 0)
        _timerId = startTimer(1000);
}

void DMHNetworkObserver_Private::stop()
{
    if(_timerId != 0)
    {
        killTimer(_timerId);
        _timerId = 0;
    }

    if(_reply)
    {
        _reply->abort();
        _reply->deleteLater();
        _reply = nullptr;
    }
}

void DMHNetworkObserver_Private::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    if((_reply) || (_logon.getUserName().isEmpty()))
        return;

    //QUrl serviceUrl = QUrl("https://dmh.wwpd.de/pll_player.php");
    QUrl serviceUrl = QUrl(_logon.getURLString() + QString("/pll_player.php"));
    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery postData;
    //postData.addQueryItem("user", "c.turner");
    //postData.addQueryItem("password", "Ente12345");
    //postData.addQueryItem("session", "7B3AA550-649A-4D51-920E-CAB465616995");
    postData.addQueryItem("user", _logon.getUserName());
    //postData.addQueryItem("user", _logon.getUserId());
    postData.addQueryItem("password", _logon.getPassword());
    postData.addQueryItem("session", _logon.getSession());

#ifdef QT_DEBUG
    emit DEBUG_message_contents(postData.toString(QUrl::FullyEncoded).toUtf8());
#endif

    _reply = _manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
}

void DMHNetworkObserver_Private::requestFinished(QNetworkReply* reply)
{
    if((!_reply) || (_reply != reply))
        return;

    interpretReply(reply);

    _reply->deleteLater();
    _reply = nullptr;
}

void DMHNetworkObserver_Private::interpretReply(QNetworkReply* reply)
{
    if(!_reply)
        return;

    if(reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "[DMHNetworkObserver] ERROR identified in payload network reply: " << reply->error() << ", Error string " << reply->errorString();
        return;
    }

    QByteArray bytes = _reply->readAll();
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
