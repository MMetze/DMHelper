#include "dmc_optionscontainer.h"
#include <QSettings>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDebug>

DMC_OptionsContainer::DMC_OptionsContainer(QObject *parent) :
    QObject(parent),
    _urlString(),
    _userName(),
    _password("Ente2020"),
    _session(),
    _cacheDirectory()
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

QString DMC_OptionsContainer::getCacheDirectory() const
{
    return _cacheDirectory;
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
    setCacheDirectory(settings.value("cacheDirectory", "").toString());

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
    settings.setValue("cacheDirectory", getCacheDirectory());

#ifdef QT_DEBUG
    settings.endGroup(); // DEBUG
#endif
}

void DMC_OptionsContainer::setURLString(const QString& urlString)
{
    if(_urlString != urlString)
    {
        _urlString = urlString;
        emit urlStringChanged(urlString);
    }
}

void DMC_OptionsContainer::setUserName(const QString& username)
{
    if(_userName != username)
    {
        _userName = username;
        emit userNameChanged(username);
    }
}

void DMC_OptionsContainer::setPassword(const QString& password)
{
    if(_password != password)
    {
        _password = password;
        emit passwordChanged(password);
    }
}

void DMC_OptionsContainer::setSession(const QString& session)
{
    if(_session != session)
    {
        _session = session;
        emit sessionChanged(session);
    }
}

void DMC_OptionsContainer::setCacheDirectory(const QString& cacheDirectory)
{
    QString resolvedDirectory;
    if(!cacheDirectory.isEmpty())
        resolvedDirectory = cacheDirectory;
    else
        resolvedDirectory = getStandardDirectory("cache");

    if(_cacheDirectory != resolvedDirectory)
    {
        _cacheDirectory = resolvedDirectory;
        emit cacheDirectoryChanged(cacheDirectory);
    }
}

void DMC_OptionsContainer::copy(DMC_OptionsContainer& other)
{
    setURLString(other._urlString);
    setUserName(other._userName);
    setPassword(other._password);
    setSession(other._session);
    setCacheDirectory(other._cacheDirectory);
}

QString DMC_OptionsContainer::getStandardDirectory(const QString& defaultDir)
{
    QString standardPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString result = standardPath + QString("/") + defaultDir;
    QDir standardDir(result);
    if(standardDir.exists())
    {
        qDebug() << "[DMC_OptionsContainer] Standard directory found: " << result;
        return result;
    }

    qDebug() << "[DMC_OptionsContainer] Creating standard directory: " << result;
    QDir().mkpath(result);

    if(!standardDir.exists())
    {
        qDebug() << "[DMC_OptionsContainer] ERROR: Standard directory creation failed!";
        return QString();
    }

    qDebug() << "[DMC_OptionsContainer] Standard directory created";
    return result;
}
