#include "discordposter.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QDebug>

DiscordPoster::DiscordPoster(QObject *parent) :
    QObject(parent),
    _manager(nullptr)
{
}

void DiscordPoster::post()
{
    _manager = new QNetworkAccessManager(this);
    connect(_manager, &QNetworkAccessManager::finished, this, &DiscordPoster::replyFinished);

    QUrl serviceUrl = QUrl("https://discordapp.com/api/webhooks/679108629353332790/CGt7NB18xGKqYI24-OD7F4TR8MCzhtZVfioHHuA3gZtrY_AoWuzkrXLwRuzp9I6mLF8a");
    QNetworkRequest request(serviceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data");

    QUrlQuery postData;
    //postData.addQueryItem("user", "c.turner");
    //postData.addQueryItem("password", "Ente12345");
    //postData.addQueryItem("session", "7B3AA550-649A-4D51-920E-CAB465616995");
    postData.addQueryItem("content", QString("This is a test hook"));
    //postData.addQueryItem("password", _logon.getPassword());
    //postData.addQueryItem("session", _logon.getSession());
    //QString payloadString = payload.toString();
    //postData.addQueryItem("payload", payloadString);

    _manager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());

}

void DiscordPoster::replyFinished(QNetworkReply *reply)
{
    if(reply)
        qDebug() << reply->error() << " " << reply->errorString();
    deleteLater();
}
