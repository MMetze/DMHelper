#include "dmc_optionscontainer.h"
#include <QSettings>

DMC_OptionsContainer::DMC_OptionsContainer(QObject *parent) :
    QObject(parent),
    _urlString(),
    _userName(),
    _password(),
    _session()
{
}

DMC_OptionsContainer::~DMC_OptionsContainer()
{
}

QString DMC_OptionsContainer::getURLString() const
{
    return _urlString;
}

QString DMC_OptionsContainer::getUserName() const
{
    return _userName;
}

QString DMC_OptionsContainer::getPassword() const
{
    return _password;
}

QString DMC_OptionsContainer::getSession() const
{
    return _session;
}

DMHLogon DMC_OptionsContainer::getLogon() const
{
    return DMHLogon(getURLString(), getUserName(), getPassword(), getSession());
}

void DMC_OptionsContainer::readSettings()
{
    QSettings settings("Glacial Software", "DMHelperClient");

#ifdef QT_DEBUG
    settings.beginGroup("DEBUG");
#endif

    setURLString(settings.value("url","").toString());
    setUserName(settings.value("username","").toString());
    setSession(settings.value("session","").toString());

#ifdef QT_DEBUG
    settings.endGroup(); // DEBUG
#endif
}

void DMC_OptionsContainer::writeSettings()
{
    QSettings settings("Glacial Software", "DMHelperClient");

#ifdef QT_DEBUG
    settings.beginGroup("DEBUG");
#endif

    settings.setValue("url", getURLString());
    settings.setValue("username", getUserName());
    settings.setValue("session", getSession());

#ifdef QT_DEBUG
    settings.endGroup(); // DEBUG
#endif
}

void DMC_OptionsContainer::setURLString(const QString& urlString)
{
    if(_urlString != urlString)
    {
        _urlString = urlString;
        emit urlStringChanged();
    }
}

void DMC_OptionsContainer::setUserName(const QString& username)
{
    if(_userName != username)
    {
        _userName = username;
        emit userNameChanged();
    }
}

void DMC_OptionsContainer::setPassword(const QString& password)
{
    if(_password != password)
    {
        _password = password;
        emit passwordChanged();
    }
}

void DMC_OptionsContainer::setSession(const QString& session)
{
    if(_session != session)
    {
        _session = session;
        emit sessionChanged();
    }
}

void DMC_OptionsContainer::copy(DMC_OptionsContainer& other)
{
    setURLString(other._urlString);
    setUserName(other._userName);
    setPassword(other._password);
    setSession(other._session);
}
