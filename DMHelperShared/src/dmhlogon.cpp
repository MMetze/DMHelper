#include "dmhlogon.h"
#include "dmhlogon_private.h"
#include <QDebug>

DMHLogon::DMHLogon() :
    d(new DMHLogon_Private())
{
}

DMHLogon::DMHLogon(const QString& urlString, const QString& username, const QString& userId, const QString& password, const QString& session) :
    d(new DMHLogon_Private(urlString, username, userId, password, session))
{
}

DMHLogon::DMHLogon(const DMHLogon& other) :
    d(new DMHLogon_Private(*(other.d)))
{
}

DMHLogon::~DMHLogon()
{
}

DMHLogon& DMHLogon::operator=(const DMHLogon& other)
{
    setURLString(other.getURLString());
    setUserName(other.getUserName());
    setPassword(other.getPassword());
    setSession(other.getSession());
    return *this;
}

bool DMHLogon::isValid() const
{
    return d->isValid();
}

QString DMHLogon::getURLString() const
{
    return d->getURLString();
}

QString DMHLogon::getUserName() const
{
    return d->getUserName();
}

QString DMHLogon::getUserId() const
{
    return d->getUserId();
}

QString DMHLogon::getPassword() const
{
    return d->getPassword();
}

QString DMHLogon::getSession() const
{
    return d->getSession();
}

void DMHLogon::setURLString(const QString& urlString)
{
    d->setURLString(urlString);
}

void DMHLogon::setUserName(const QString& username)
{
    d->setUserName(username);
}

void DMHLogon::setUserId(const QString& userId)
{
    d->setUserId(userId);
}

void DMHLogon::setPassword(const QString& password)
{
    d->setPassword(password);
}

void DMHLogon::setSession(const QString& session)
{
    d->setSession(session);
}

QDebug operator<<(QDebug d, const DMHLogon &logon)
{
    d << QString("(DMHLogon: URL: ") << logon.getURLString() << QString(", Username: ") << logon.getUserName() << QString(", Session: ") << logon.getSession() << QString(")");
    return d;
}
