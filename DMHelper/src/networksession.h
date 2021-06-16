#ifndef NETWORKSESSION_H
#define NETWORKSESSION_H

#include <QObject>
#include <QList>

class NetworkPlayer;

class NetworkSession : public QObject
{
    Q_OBJECT
public:
    explicit NetworkSession(QObject *parent = nullptr);
    explicit NetworkSession(const QString& sessionID, const QString& sessionName, const QString& inviteID, QObject *parent = nullptr);
    explicit NetworkSession(const NetworkSession& other);
    virtual ~NetworkSession();

    void setValues(const NetworkSession& other);

    QString getID() const;
    QString getName() const;
    QString getInvite() const;

    QList<NetworkPlayer*> getPlayers() const;

    NetworkPlayer* getPlayerById(const QString& uuid) const;
    NetworkPlayer* getPlayerByName(const QString& userName) const;
    NetworkPlayer* getPlayer(const QString& uuid, const QString& userName) const;

    bool playerExistsById(const QString& uuid) const;
    bool playerExistsByName(const QString& userName) const;
    bool playerExists(const NetworkPlayer& player) const;

    void removePlayer(NetworkPlayer* player);

public slots:
    void setID(const QString& id);
    void setName(const QString& name);
    void setInvite(const QString& invite);

    bool addPlayer(const QString& userName);
    bool addPlayer(NetworkPlayer* player);

signals:

protected:
    QString _sessionID;
    QString _sessionName;
    QString _inviteID;
    QList<NetworkPlayer*> _players;
};

#endif // NETWORKSESSION_H
