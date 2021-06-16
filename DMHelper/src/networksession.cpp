#include "networksession.h"
#include "networkplayer.h"

NetworkSession::NetworkSession(QObject *parent) :
    QObject(parent),
    _sessionID(),
    _sessionName(),
    _inviteID(),
    _players()
{
}

NetworkSession::NetworkSession(const QString& sessionID, const QString& sessionName, const QString& inviteID, QObject *parent) :
    QObject(parent),
    _sessionID(sessionID),
    _sessionName(sessionName),
    _inviteID(inviteID),
    _players()
{
}

NetworkSession::NetworkSession(const NetworkSession& other) :
    QObject(other.parent()),
    _sessionID(other._sessionID),
    _sessionName(other._sessionName),
    _inviteID(other._inviteID),
    _players()
{
    for(NetworkPlayer* player : other._players)
    {
        if(player)
            _players.append(new NetworkPlayer(*player));
    }
}

NetworkSession::~NetworkSession()
{
    qDeleteAll(_players);
}

void NetworkSession::setValues(const NetworkSession& other)
{
    setID(other.getID());
    setName(other.getName());
    setInvite(other.getInvite());

    // Update the players data, creating new players where needed
    for(NetworkPlayer* player : other._players)
    {
        if(player)
        {
            NetworkPlayer* currentPlayer = getPlayer(player->getID(), player->getUserName());
            if(currentPlayer)
                currentPlayer->setValues(*player);
            else
                _players.append(new NetworkPlayer(*player));
        }
    }

    // Remove any players no longer needed
    for(NetworkPlayer* existingPlayer : _players)
    {
        if(existingPlayer)
        {
            if((!other.playerExists(*existingPlayer)) &&
               (_players.removeAll(existingPlayer) > 0))
                delete existingPlayer;
        }
    }
}

QString NetworkSession::getID() const
{
    return _sessionID;
}

QString NetworkSession::getName() const
{
    return _sessionName;
}

QString NetworkSession::getInvite() const
{
    return _inviteID;
}

QList<NetworkPlayer*> NetworkSession::getPlayers() const
{
    return _players;
}

NetworkPlayer* NetworkSession::getPlayerById(const QString& uuid) const
{
    if(!uuid.isEmpty())
    {
        for(NetworkPlayer* player : _players)
        {
            if((player) && (player->getID() == uuid))
                return player;
        }
    }

    return nullptr;
}

NetworkPlayer* NetworkSession::getPlayerByName(const QString& userName) const
{
    if(!userName.isEmpty())
    {
        for(NetworkPlayer* player : _players)
        {
            if((player) && (player->getUserName() == userName))
                return player;
        }
    }

    return nullptr;
}

NetworkPlayer* NetworkSession::getPlayer(const QString& uuid, const QString& userName) const
{
    if((!uuid.isNull()) || (!userName.isEmpty()))
    {
        for(NetworkPlayer* player : _players)
        {
            if((player) &&
               (((!uuid.isNull()) && (player->getID() == uuid)) ||
                ((!userName.isEmpty()) && (player->getUserName() == userName))))
                return player;
        }
    }

    return nullptr;
}

bool NetworkSession::playerExistsById(const QString& uuid) const
{
    return (getPlayerById(uuid) != nullptr);
}

bool NetworkSession::playerExistsByName(const QString& userName) const
{
    return (getPlayerByName(userName) != nullptr);
}

bool NetworkSession::playerExists(const NetworkPlayer& player) const
{
    return (getPlayer(player.getID(), player.getUserName()) != nullptr);
}

void NetworkSession::removePlayer(NetworkPlayer* player)
{
    if(!player)
        return;

    _players.removeAll(player);
}

void NetworkSession::setID(const QString& id)
{
    _sessionID = id;
}

void NetworkSession::setName(const QString& name)
{
    _sessionName = name;
}

void NetworkSession::setInvite(const QString& invite)
{
    _inviteID = invite;
}

bool NetworkSession::addPlayer(const QString& userName)
{
    if(playerExistsByName(userName))
        return false;

    _players.append(new NetworkPlayer(userName));
    return true;
}

bool NetworkSession::addPlayer(NetworkPlayer* player)
{
    if((!player) || (playerExists(*player)))
        return false;

    _players.append(player);
    return true;
}
