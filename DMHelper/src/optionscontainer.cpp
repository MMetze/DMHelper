#include "optionscontainer.h"
#include "optionsdialog.h"
#include <QSettings>
#include <QDir>
#include <QCoreApplication>

// TODO: consider copy of MRU functionality

OptionsContainer::OptionsContainer(QObject *parent) :
    QObject(parent),
    _bestiaryFileName(),
    _lastMonster(),
#ifdef INCLUDE_CHASE_SUPPORT
    _chaseFileName(),
#endif
    _showAnimations(false),
    _audioVolume(100),
    _showOnDeck(true),
    _showCountdown(true),
    _countdownDuration(15),
#ifdef INCLUDE_NETWORK_SUPPORT
    _networkEnabled(false),
    _urlString(),
    _userName(),
    _savePassword(false),
    _password(), // note: password will not be stored in settings
    _sessionID(),
    _inviteID(),
#endif
    _mruHandler(nullptr)
{
}

OptionsContainer::~OptionsContainer()
{
}

QString OptionsContainer::getBestiaryFileName() const
{
    return _bestiaryFileName;
}

#ifdef INCLUDE_CHASE_SUPPORT
QString OptionsContainer::getChaseFileName() const
{
    return _chaseFileName;
}
#endif

QString OptionsContainer::getLastMonster() const
{
    return _lastMonster;
}

bool OptionsContainer::getShowAnimations() const
{
    return _showAnimations;
}

int OptionsContainer::getAudioVolume() const
{
    return _audioVolume;
}

bool OptionsContainer::getShowOnDeck() const
{
    return _showOnDeck;
}

bool OptionsContainer::getShowCountdown() const
{
    return _showCountdown;
}

int OptionsContainer::getCountdownDuration() const
{
    return _countdownDuration;
}

#ifdef INCLUDE_NETWORK_SUPPORT

bool OptionsContainer::getNetworkEnabled() const
{
    return _networkEnabled;
}

QString OptionsContainer::getURLString() const
{
    return _urlString;
}

QString OptionsContainer::getUserName() const
{
    return _userName;
}

bool OptionsContainer::getSavePassword() const
{
    return _savePassword;
}

QString OptionsContainer::getPassword() const
{
    return _password;
}

QString OptionsContainer::getSessionID() const
{
    return _sessionID;
}

QString OptionsContainer::getInviteID() const
{
    return _inviteID;
}

#endif //INCLUDE_NETWORK_SUPPORT

MRUHandler* OptionsContainer::getMRUHandler() const
{
    return _mruHandler;
}

void OptionsContainer::setMRUHandler(MRUHandler* mruHandler)
{
    mruHandler->setParent(this);
    _mruHandler = mruHandler;
}

void OptionsContainer::editSettings()
{
    OptionsContainer* editCopyContainer = new OptionsContainer(this);
    editCopyContainer->copy(this);

    OptionsDialog dlg(editCopyContainer);

    if(dlg.exec() == QDialog::Accepted)
    {
        copy(editCopyContainer);
    }

    delete editCopyContainer;
}

void OptionsContainer::readSettings()
{
    QSettings settings("Glacial Software", "DMHelper");

#ifdef QT_DEBUG
    settings.beginGroup("DEBUG");
#endif

    // Note: password will not be stored in settings
#ifdef Q_OS_MAC
    QDir fileDirPath(QCoreApplication::applicationDirPath());
    fileDirPath.cdUp();
    fileDirPath.cdUp();
    fileDirPath.cdUp();
    QString bestiaryFileName = fileDirPath.path() + QString("/bestiary/DMHelperBestiary.xml");
#else
    QString bestiaryFileName = QString("./bestiary/DMHelperBestiary.xml");
#endif
    setBestiaryFileName(settings.value("bestiary",bestiaryFileName).toString());
    setLastMonster(settings.value("lastMonster","").toString());
#ifdef INCLUDE_CHASE_SUPPORT
    setChaseFileName(settings.value("chase data","").toString());
#endif
    setShowAnimations(settings.value("showAnimations",QVariant(false)).toBool());
    setAudioVolume(settings.value("audioVolume",QVariant(100)).toInt());
    setShowOnDeck(settings.value("showOnDeck",QVariant(true)).toBool());
    setShowCountdown(settings.value("showCountdown",QVariant(true)).toBool());
    setCountdownDuration(settings.value("countdownDuration",QVariant(15)).toInt());
#ifdef INCLUDE_NETWORK_SUPPORT
    setNetworkEnabled(settings.value("networkEnabled",QVariant(false)).toBool());
    setURLString(settings.value("url","").toString());
    setUserName(settings.value("username","").toString());
    setSavePassword(settings.value("savePassword",QVariant(false)).toBool());
    setPassword(settings.value("password","").toString());
    setSessionID(settings.value("sessionID","").toString());
    setInviteID(settings.value("inviteID","").toString());
#endif

    if(_mruHandler)
    {
        _mruHandler->readMRUFromSettings(settings);
    }

#ifdef QT_DEBUG
    settings.endGroup(); // DEBUG
#endif
}

void OptionsContainer::writeSettings()
{
    QSettings settings("Glacial Software", "DMHelper");

#ifdef QT_DEBUG
    settings.beginGroup("DEBUG");
#endif

    // Note: password will not be stored in settings
    settings.setValue("bestiary", getBestiaryFileName());
    settings.setValue("lastMonster", getLastMonster());
#ifdef INCLUDE_CHASE_SUPPORT
    settings.setValue("chase data", getChaseFileName());
#endif
    settings.setValue("showAnimations", getShowAnimations());
    settings.setValue("audioVolume", getAudioVolume());
    settings.setValue("showOnDeck", getShowOnDeck());
    settings.setValue("showCountdown", getShowCountdown());
    settings.setValue("countdownDuration", getCountdownDuration());
#ifdef INCLUDE_NETWORK_SUPPORT
    settings.setValue("networkEnabled", getNetworkEnabled());
    settings.setValue("url", getURLString());
    settings.setValue("username", getUserName());
    settings.setValue("savePassword", getSavePassword());
    if(getSavePassword())
        settings.setValue("password", getPassword());
    settings.setValue("sessionID", getSessionID());
    settings.setValue("inviteID", getInviteID());
#endif

    if(_mruHandler)
    {
        _mruHandler->writeMRUToSettings(settings);
    }

#ifdef QT_DEBUG
    settings.endGroup(); // DEBUG
#endif
}

void OptionsContainer::setBestiaryFileName(const QString& filename)
{
    if(_bestiaryFileName != filename)
    {
        _bestiaryFileName = filename;
        emit bestiaryFileNameChanged();
    }
}

#ifdef INCLUDE_CHASE_SUPPORT
void OptionsContainer::setChaseFileName(const QString& filename)
{
    if(_chaseFileName != filename)
    {
        _chaseFileName = filename;
        emit chaseFileNameChanged();
    }
}
#endif

void OptionsContainer::setLastMonster(const QString& lastMonster)
{
    if(_lastMonster != lastMonster)
    {
        _lastMonster = lastMonster;
    }
}

void OptionsContainer::setShowAnimations(bool showAnimations)
{
    if(_showAnimations != showAnimations)
    {
        _showAnimations = showAnimations;
        emit showAnimationsChanged(_showAnimations);
    }
}

void OptionsContainer::setAudioVolume(int volume)
{
    if(volume < 0) volume = 0;
    if(volume > 100) volume = 100;

    if(_audioVolume != volume)
    {
        _audioVolume = volume;
        emit audioVolumeChanged(_audioVolume);
    }
}

void OptionsContainer::setShowOnDeck(bool showOnDeck)
{
    if(_showOnDeck != showOnDeck)
    {
        _showOnDeck = showOnDeck;
        emit showOnDeckChanged(_showOnDeck);
    }
}

void OptionsContainer::setShowCountdown(bool showCountdown)
{
    if(_showCountdown != showCountdown)
    {
        _showCountdown = showCountdown;
        emit showCountdownChanged(_showCountdown);
    }
}

void OptionsContainer::setCountdownDuration(int countdownDuration)
{
    if(_countdownDuration != countdownDuration)
    {
        _countdownDuration = countdownDuration;
        emit countdownDurationChanged(_countdownDuration);
    }
}

void OptionsContainer::setCountdownDuration(const QString& countdownDuration)
{
    bool ok;
    int newDuration = countdownDuration.toInt(&ok);
    if(ok)
    {
        setCountdownDuration(newDuration);
    }
}

#ifdef INCLUDE_NETWORK_SUPPORT

void OptionsContainer::setNetworkEnabled(bool enabled)
{
    if(_networkEnabled != enabled)
    {
        _networkEnabled = enabled;
        emit networkEnabledChanged(_networkEnabled);
    }
}

void OptionsContainer::setURLString(const QString& urlString)
{
    if(_urlString != urlString)
    {
        _urlString = urlString;
        emit urlStringChanged(_urlString);
        emit networkSettingsChanged(_urlString, _userName, _password, _sessionID, _inviteID);
    }
}

void OptionsContainer::setUserName(const QString& username)
{
    if(_userName != username)
    {
        _userName = username;
        emit userNameChanged(_userName);
        emit networkSettingsChanged(_urlString, _userName, _password, _sessionID, _inviteID);
    }
}

void OptionsContainer::setSavePassword(bool savePassword)
{
    if(_savePassword != savePassword)
    {
        _savePassword = savePassword;
        emit savePasswordChanged(_savePassword);
    }
}

void OptionsContainer::setPassword(const QString& password)
{
    if(_password != password)
    {
        _password = password;
        emit passwordChanged(_password);
        emit networkSettingsChanged(_urlString, _userName, _password, _sessionID, _inviteID);
    }
}

void OptionsContainer::setSessionID(const QString& sessionID)
{
    if(_sessionID != sessionID)
    {
        _sessionID = sessionID;
        emit sessionIDChanged(_sessionID);
        emit networkSettingsChanged(_urlString, _userName, _password, _sessionID, _inviteID);
    }
}

void OptionsContainer::setInviteID(const QString& inviteID)
{
    if(_inviteID != inviteID)
    {
        _inviteID = inviteID;
        emit inviteIDChanged(_inviteID);
        emit networkSettingsChanged(_urlString, _userName, _password, _sessionID, _inviteID);
    }
}

#endif //INCLUDE_NETWORK_SUPPORT

void OptionsContainer::copy(OptionsContainer* other)
{
    if(other)
    {
        setBestiaryFileName(other->_bestiaryFileName);
        setLastMonster(other->_lastMonster);
#ifdef INCLUDE_CHASE_SUPPORT
        setChaseFileName(other->_chaseFileName);
#endif
        setShowAnimations(other->_showAnimations);
        setShowOnDeck(other->_showOnDeck);
        setShowCountdown(other->_showCountdown);
        setCountdownDuration(other->_countdownDuration);
#ifdef INCLUDE_NETWORK_SUPPORT
        setNetworkEnabled(other->_networkEnabled);
        setURLString(other->_urlString);
        setUserName(other->_userName);
        setSavePassword(other->_savePassword);
        setPassword(other->_password);
        setSessionID(other->_sessionID);
        setInviteID(other->_inviteID);
#endif
    }
}
