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
}

bool NetworkPlayer::operator==(const NetworkPlayer& other)
{
    return (((!_id.isNull()) && (_id == other._id)) ||
            ((_id.isNull()) && (_userName == other._userName)));
}

void NetworkPlayer::setValues(const NetworkPlayer& other)
{
    setID(other._id);
    setUserName(other._userName);
    setScreenName(other._screenName);
    setLastSeen(other._lastSeen);
    setStatus(other._status);
}

void NetworkPlayer::setValues(const QString& id, const QString& userName, const QString& screenName)
{
    setID(id);
    setUserName(userName);
    setScreenName(screenName);
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
    if(_id != id)
    {
        _id = id;
        emit idChanged(_id);
    }
}

void NetworkPlayer::setUserName(const QString& userName)
{
    if(_userName != userName)
    {
        _userName = userName;
        emit userNameChanged(_userName);
    }
}

void NetworkPlayer::setScreenName(const QString& screenName)
{
    if(_screenName != screenName)
    {
        _screenName = screenName;
        emit screenNameChanged(_screenName);
    }
}

bool NetworkPlayer::setLastSeen(const QDateTime& lastSeen)
{
    if(lastSeen.isValid())
    {
        if(_lastSeen != lastSeen)
        {
            _lastSeen = lastSeen;
            emit lastSeenChanged(_lastSeen);
        }
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
    setLastSeen(QDateTime::currentDateTime());
}

void NetworkPlayer::setStatus(int status)
{
    if(_status != status)
    {
        _status = status;
        emit statusChanged(_status);
    }
}

void NetworkPlayer::setStatus(const QString& status)
{
    setStatus(status.isEmpty() ? NetworkPlayer_Status::Unknown : status.toInt());
}

void NetworkPlayer::setAccepted()
{
    setStatus(NetworkPlayer_Status::Accepted);
}

void NetworkPlayer::setBlocked()
{
    setStatus(NetworkPlayer_Status::Blocked);
}
