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
    _userId(),
    _savePassword(false),
    _password(),
    _currentInvite(),
    _invites(),
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

QString DMC_OptionsContainer::getUserId() const
{
    return _userId;
}

bool DMC_OptionsContainer::getSavePassword() const
{
    return _savePassword;
}

QString DMC_OptionsContainer::getPassword() const
{
    return _password;
}

QString DMC_OptionsContainer::getCurrentInvite() const
{
    return _currentInvite;
}

QStringList DMC_OptionsContainer::getInvites() const
{
    return _invites.keys();
}

QString DMC_OptionsContainer::getInviteName(const QString& invite) const
{
    return _invites.value(invite);
}

DMHLogon DMC_OptionsContainer::getLogon() const
{
    return DMHLogon(getURLString(), getUserName(), getUserId(), getPassword(), getCurrentInvite());
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
    setSavePassword(settings.value("savePassword",QVariant(false)).toBool());
    setPassword(settings.value("password","").toString());

    settings.beginGroup("Invites");
        QStringList invites = settings.childGroups();
        for(QString invite : invites)
        {
            settings.beginGroup(invite);
                QString inviteName = settings.value("name").toString();
                if(!inviteName.isEmpty())
                    _invites.insert(invite, inviteName);
            settings.endGroup(); // Specific invite
        }
    settings.endGroup(); // Invites

    QString currentInvite = settings.value("currentInvite").toString();
    if(_invites.contains(currentInvite))
        setCurrentInvite(currentInvite);

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
    settings.setValue("savePassword", getSavePassword());
    if(getSavePassword())
        settings.setValue("password", getPassword());

    settings.beginGroup("Invites");
        QStringList invites = _invites.keys();
        for(QString invite : invites)
        {
            settings.beginGroup(invite);
                settings.setValue("name", _invites.value(invite));
            settings.endGroup(); // Invite
        }
    settings.endGroup(); // Invites

    settings.setValue("currentInvite", getCurrentInvite());

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

void DMC_OptionsContainer::setUserId(const QString& userId)
{
    if(_userId != userId)
    {
        _userId = userId;
        emit userIdChanged(userId);
    }
}

void DMC_OptionsContainer::setSavePassword(bool savePassword)
{
    if(_savePassword != savePassword)
    {
        _savePassword = savePassword;
        emit savePasswordChanged(savePassword);
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

void DMC_OptionsContainer::setCurrentInvite(const QString& invite)
{
    if((invite.isEmpty()) || (_currentInvite == invite) || (!_invites.contains(invite)))
        return;

    _currentInvite = invite;
    emit currentInviteChanged(_currentInvite);
}

void DMC_OptionsContainer::addInvite(const QString& invite, const QString& inviteName)
{
    if((invite.isEmpty()) || (inviteName.isEmpty()) || (_invites.contains(invite)))
        return;

    _invites.insert(invite, inviteName);
    emit inviteChanged(invite, inviteName);
}

void DMC_OptionsContainer::setInviteName(const QString& invite, const QString& inviteName)
{
    if((invite.isEmpty()) || (inviteName.isEmpty()) || (!_invites.contains(invite)))
        return;

    _invites[invite] = inviteName;
    emit inviteChanged(invite, inviteName);
}

void DMC_OptionsContainer::removeInvite(const QString& invite)
{
    if((invite.isEmpty()) || (!_invites.contains(invite)))
        return;

    emit inviteChanged(invite, _invites.value(invite));
    _invites.remove(invite);
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
    setUserId(other._userId);
    setPassword(other._password);
    setCurrentInvite(other._currentInvite);

    QStringList invites = other._invites.keys();
    for(QString invite : invites)
        _invites.insert(invite, other._invites.value(invite));

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
