#ifndef DISCORDPOSTER_H
#define DISCORDPOSTER_H

#include <QObject>
#include <QNetworkReply>

class DiscordPoster : public QObject
{
    Q_OBJECT
public:
    explicit DiscordPoster(QObject *parent = nullptr);

    void post();

protected slots:
    void replyFinished(QNetworkReply *reply);

protected:
    QNetworkAccessManager* _manager;
};

#endif // DISCORDPOSTER_H
