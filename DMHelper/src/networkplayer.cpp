#include "networkplayer.h"

/*
 * <ID>D6B96235-5DF2-41CB-B99C-80A0D9AE87E4</ID>
<username>TestUser</username>
<surname>Joe</surname>
<forename/>\n </node>\
*/

NetworkPlayer::NetworkPlayer() :
    QObject(),
    _id(),
    _userName(),
    _screenName(),
    _lastSeen(),
    _status(NetworkPlayer_Status::Unknown)
{
    setLastSeenNow();
}

NetworkPlayer::NetworkPlayer(const QString& userName) :
    QObject(),
    _id(),
    _userName(userName),
    _screenName(),
    _lastSeen(),
    _status(NetworkPlayer_Status::Unknown)
{
    setLastSeenNow();
}

NetworkPlayer::NetworkPlayer(const QString& id, const QString& userName, const QString& screenName, const QString& lastSeen, const QString& status) :
    QObject(),
    _id(),
    _userName(userName),
    _screenName(screenName),
    _lastSeen(),
    _status()
{
    setID(id);

    if(!setLastSeen(lastSeen))
        setLastSeenNow();

    setStatus(status);
}

NetworkPlayer::NetworkPlayer(const QString& id, const QString& userName, const QString& screenName, const QDateTime& lastSeen, int status) :
    QObject(),
    _id(id),
    _userName(userName),
    _screenName(screenName),
    _lastSeen(lastSeen),
    _status(status)
{
}

NetworkPlayer::NetworkPlayer(const NetworkPlayer& other) :
    QObject(other.parent()),
    _id(other._id),
    _userName(other._userName),
    _screenName(other._screenName),
    _lastSeen(other._lastSeen),
    _status(other._status)
{
}

NetworkPlayer::~NetworkPlayer()
{
    int n = 0;
}

bool NetworkPlayer::operator==(const NetworkPlayer& other)
{
    return (((!_id.isNull()) && (_id == other._id)) ||
            ((_id.isNull()) && (_userName == other._userName)));
}

void NetworkPlayer::setValues(const NetworkPlayer& other)
{
    _id = other._id;
    _userName = other._userName;
    _screenName = other._screenName;
    _lastSeen = other._lastSeen;
    _status = other._status;
}

void NetworkPlayer::setValues(const QString& id, const QString& userName, const QString& screenName)
{
    _id = id;
    _userName = userName;
    _screenName = screenName;
}

QString NetworkPlayer::getID() const
{
    return _id;
}

QString NetworkPlayer::getUserName() const
{
    return _userName;
}

QString NetworkPlayer::getScreenName() const
{
    return _screenName;
}

bool NetworkPlayer::isAvailable() const
{
    return _lastSeen.secsTo(QDateTime::currentDateTime()) <= (60 * 60 * 24);
}

QDateTime NetworkPlayer::getLastSeen() const
{
    return _lastSeen;
}

int NetworkPlayer::getStatus() const
{
    return _status;
}

void NetworkPlayer::setID(const QString& id)
{
    _id = id;
}

void NetworkPlayer::setUserName(const QString& userName)
{
    _userName = userName;
}

void NetworkPlayer::setScreenName(const QString& screenName)
{
    _screenName = screenName;
}

bool NetworkPlayer::setLastSeen(const QDateTime& lastSeen)
{
    if(lastSeen.isValid())
    {
        _lastSeen = lastSeen;
        return true;
    }
    else
    {
        return false;
    }
}

bool NetworkPlayer::setLastSeen(const QString& lastSeen)
{
    return setLastSeen(QDateTime::fromString(lastSeen));
}

void NetworkPlayer::setLastSeenNow()
{
    _lastSeen = QDateTime::currentDateTime();
}

void NetworkPlayer::setStatus(int status)
{
    _status = status;
}

void NetworkPlayer::setStatus(const QString& status)
{
    _status = status.isEmpty() ? NetworkPlayer_Status::Unknown : status.toInt();
}

void NetworkPlayer::setAccepted()
{
    _status = NetworkPlayer_Status::Accepted;
}

void NetworkPlayer::setBlocked()
{
    _status = NetworkPlayer_Status::Blocked;
}
