#ifndef NETWORKPLAYER_H
#define NETWORKPLAYER_H

#include <QObject>
#include <QUuid>
#include <QDateTime>

class NetworkPlayer : public QObject
{
    Q_OBJECT
public:
    explicit NetworkPlayer();
    explicit NetworkPlayer(const QString& userName);
    explicit NetworkPlayer(const QString& id, const QString& userName, const QString& screenName, const QString& lastSeen = QString(), const QString& status = QString());
    explicit NetworkPlayer(const QString& id, const QString& userName, const QString& screenName, const QDateTime& lastSeen, int status);
    explicit NetworkPlayer(const NetworkPlayer& other);
    virtual ~NetworkPlayer() override;

    bool operator==(const NetworkPlayer& other);

    enum NetworkPlayer_Status
    {
        Unknown = 0,
        Blocked,
        Accepted
    };

    void setValues(const NetworkPlayer& other);
    void setValues(const QString& id, const QString& userName, const QString& screenName);

    QString getID() const;
    QString getUserName() const;
    QString getScreenName() const;
    bool isAvailable() const;
    QDateTime getLastSeen() const;
    int getStatus() const;

public slots:
    void setID(const QString& id);
    void setUserName(const QString& userName);
    void setScreenName(const QString& screenName);
    bool setLastSeen(const QDateTime& lastSeen);
    bool setLastSeen(const QString& lastSeen);
    void setLastSeenNow();
    void setStatus(int status);
    void setStatus(const QString& status);
    void setAccepted();
    void setBlocked();

signals:

protected:
    QString _id;
    QString _userName;
    QString _screenName;
    QDateTime _lastSeen;
    int _status;
};

#endif // NETWORKPLAYER_H
