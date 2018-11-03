#include "optionscontainer.h"
#include "optionsdialog.h"
#include <QSettings>

// TODO: consider copy of MRU functionality

OptionsContainer::OptionsContainer(QObject *parent) :
    QObject(parent),
    _bestiaryFileName(),
    _lastMonster(),
    _chaseFileName(),
    _showAnimations(false),
    _audioVolume(100),
    _showOnDeck(true),
    _showCountdown(true),
    _countdownDuration(15),
    _networkEnabled(false),
    _urlString(),
    _userName(),
    _savePassword(false),
    _password(), // note: password will not be stored in settings
    _sessionID(),
    _inviteID(),
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

QString OptionsContainer::getChaseFileName() const
{
    return _chaseFileName;
}


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
    setBestiaryFileName(settings.value("bestiary","").toString());
    setLastMonster(settings.value("lastMonster","").toString());
    setChaseFileName(settings.value("chase data","").toString());
    setShowAnimations(settings.value("showAnimations",QVariant(false)).toBool());
    setAudioVolume(settings.value("audioVolume",QVariant(100)).toInt());
    setShowOnDeck(settings.value("showOnDeck",QVariant(true)).toBool());
    setShowCountdown(settings.value("showCountdown",QVariant(true)).toBool());
    setCountdownDuration(settings.value("countdownDuration",QVariant(15)).toInt());
    setNetworkEnabled(settings.value("networkEnabled",QVariant(false)).toBool());
    setURLString(settings.value("url","").toString());
    setUserName(settings.value("username","").toString());
    setSavePassword(settings.value("savePassword",QVariant(false)).toBool());
    setPassword(settings.value("password","").toString());
    setSessionID(settings.value("sessionID","").toString());
    setInviteID(settings.value("inviteID","").toString());


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
    settings.setValue("chase data", getChaseFileName());
    settings.setValue("showAnimations", getShowAnimations());
    settings.setValue("audioVolume", getAudioVolume());
    settings.setValue("showOnDeck", getShowOnDeck());
    settings.setValue("showCountdown", getShowCountdown());
    settings.setValue("countdownDuration", getCountdownDuration());
    settings.setValue("networkEnabled", getNetworkEnabled());
    settings.setValue("url", getURLString());
    settings.setValue("username", getUserName());
    settings.setValue("savePassword", getSavePassword());
    if(getSavePassword())
        settings.setValue("password", getPassword());
    settings.setValue("sessionID", getSessionID());
    settings.setValue("inviteID", getInviteID());

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

void OptionsContainer::setChaseFileName(const QString& filename)
{
    if(_chaseFileName != filename)
    {
        _chaseFileName = filename;
        emit chaseFileNameChanged();
    }
}
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

void OptionsContainer::copy(OptionsContainer* other)
{
    if(other)
    {
        setBestiaryFileName(other->_bestiaryFileName);
        setLastMonster(other->_lastMonster);
        setChaseFileName(other->_chaseFileName);
        setShowAnimations(other->_showAnimations);
        setShowOnDeck(other->_showOnDeck);
        setShowCountdown(other->_showCountdown);
        setCountdownDuration(other->_countdownDuration);
        setNetworkEnabled(other->_networkEnabled);
        setURLString(other->_urlString);
        setUserName(other->_userName);
        setSavePassword(other->_savePassword);
        setPassword(other->_password);
        setSessionID(other->_sessionID);
        setInviteID(other->_inviteID);
    }
}
