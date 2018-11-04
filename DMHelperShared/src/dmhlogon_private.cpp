#include "dmhlogon_private.h"

DMHLogon_Private::DMHLogon_Private() :
    _urlString(),
    _userName(),
    _password(),
    _session()
{
}

DMHLogon_Private::DMHLogon_Private(const QString& urlString, const QString& username, const QString& password, const QString& session) :
    _urlString(urlString),
    _userName(username),
    _password(password),
    _session(session)
{
}

DMHLogon_Private::DMHLogon_Private(const DMHLogon_Private& other) :
    _urlString(other._urlString),
    _userName(other._userName),
    _password(other._password),
    _session(other._session)
{
}

DMHLogon_Private::~DMHLogon_Private()
{
}

DMHLogon_Private& DMHLogon_Private::operator=(const DMHLogon_Private& other)
{
    _urlString = other._urlString;
    _userName = other._userName;
    _password = other._password;
    _session = other._session;
    return *this;
}

QString DMHLogon_Private::getURLString() const
{
    return _urlString;
}

QString DMHLogon_Private::getUserName() const
{
    return _userName;
}

QString DMHLogon_Private::getPassword() const
{
    return _password;
}

QString DMHLogon_Private::getSession() const
{
    return _session;
}

void DMHLogon_Private::setURLString(const QString& urlString)
{
    _urlString = urlString;
}

void DMHLogon_Private::setUserName(const QString& username)
{
    _userName = username;
}

void DMHLogon_Private::setPassword(const QString& password)
{
    _password = password;
}

void DMHLogon_Private::setSession(const QString& session)
{
    _session = session;
}
