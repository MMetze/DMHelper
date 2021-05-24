#include "optionscontainer.h"
#include "optionsdialog.h"
#include <QSettings>
#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

// TODO: consider copy of MRU functionality

OptionsContainer::OptionsContainer(QMainWindow *parent) :
    QObject(parent),
    _bestiaryFileName(),
    _spellbookFileName(),
    _lastMonster(),
    _lastSpell(),
    _quickReferenceFileName(),
    _calendarFileName(),
    _equipmentFileName(),
    _shopsFileName(),
    _tablesDirectory(),
    _showAnimations(false),
    _fontFamily("Trebuchet MS"),
    _fontSize(12),
    _logicalDPI(0.0),
    _audioVolume(100),
    _showOnDeck(true),
    _showCountdown(true),
    _countdownDuration(15),
    _pointerFile(),
    _selectedIcon(),
    _activeIcon(),
    _combatantFrame(),
    _countdownFrame(),
    _dataSettingsExist(false),
    _updatesEnabled(false),
    _statisticsAccepted(false),
    _instanceUuid(),
    _lastUpdateDate(),
#ifdef INCLUDE_NETWORK_SUPPORT
    _networkEnabled(false),
    _urlString(),
    _userName(),
    _userId(),
    _savePassword(false),
    _password(), // note: password will not be stored in settings
    _currentSession(),
    _sessions(),
    _invites(),
    //_sessionID(),
    //_inviteID(),
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

QString OptionsContainer::getSpellbookFileName() const
{
    return _spellbookFileName;
}

QString OptionsContainer::getQuickReferenceFileName() const
{
    return _quickReferenceFileName;
}

QString OptionsContainer::getCalendarFileName() const
{
    return _calendarFileName;
}

QString OptionsContainer::getEquipmentFileName() const
{
    return _equipmentFileName;
}

QString OptionsContainer::getShopsFileName() const
{
    return _shopsFileName;
}

QString OptionsContainer::getTablesDirectory() const
{
    return _tablesDirectory;
}

QString OptionsContainer::getLastMonster() const
{
    return _lastMonster;
}

QString OptionsContainer::getLastSpell() const
{
    return _lastSpell;
}

bool OptionsContainer::getShowAnimations() const
{
    return _showAnimations;
}

QString OptionsContainer::getFontFamily() const
{
    return _fontFamily;
}

int OptionsContainer::getFontSize() const
{
    return _fontSize;
}

qreal OptionsContainer::getLogicalDPI() const
{
    return _logicalDPI;
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

QString OptionsContainer::getPointerFile() const
{
    return _pointerFile;
}

QString OptionsContainer::getSelectedIcon() const
{
    return _selectedIcon;
}

QString OptionsContainer::getActiveIcon() const
{
    return _activeIcon;
}

QString OptionsContainer::getCombatantFrame() const
{
    return _combatantFrame;
}

QString OptionsContainer::getCountdownFrame() const
{
    return _countdownFrame;
}

bool OptionsContainer::doDataSettingsExist() const
{
    return _dataSettingsExist;
}

bool OptionsContainer::isUpdatesEnabled() const
{
    return _updatesEnabled;
}

bool OptionsContainer::isStatisticsAccepted() const
{
    return _statisticsAccepted;
}

QUuid OptionsContainer::getInstanceUuid()
{
    if(!_statisticsAccepted)
        return QUuid();

    if(_instanceUuid.isNull())
        _instanceUuid = QUuid::createUuid();

    return _instanceUuid;
}

QString OptionsContainer::getInstanceUuidStr()
{
    return getInstanceUuid().toString();
}

QDate OptionsContainer::getLastUpdateCheck() const
{
    return _lastUpdateDate;
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

QString OptionsContainer::getUserId() const
{
    return _userId;
}

bool OptionsContainer::getSavePassword() const
{
    return _savePassword;
}

QString OptionsContainer::getPassword() const
{
    return _password;
}

QString OptionsContainer::getCurrentSession() const
{
    return _currentSession;
}

QStringList OptionsContainer::getSessions() const
{
    return _sessions.keys();
}

bool OptionsContainer::doesSessionExist(const QString& session) const
{
    return _sessions.contains(session);
}

QString OptionsContainer::getSessionName(const QString& session) const
{
    return _sessions.value(session, QString());
}

QString OptionsContainer::getSessionInvite(const QString& session) const
{
    return _invites.value(session, QString());
}

/*
QString OptionsContainer::getSessionID() const
{
    return _sessionID;
}

QString OptionsContainer::getInviteID() const
{
    return _inviteID;
}
*/

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

void OptionsContainer::editSettings(DMHelper::OptionsTab startTab)
{
    OptionsContainer* editCopyContainer = new OptionsContainer(getMainWindow());
    editCopyContainer->copy(this);

    _fontChanged = false;
    connect(editCopyContainer, &OptionsContainer::fontFamilyChanged, this, &OptionsContainer::registerFontChange);
    connect(editCopyContainer, &OptionsContainer::fontSizeChanged, this, &OptionsContainer::registerFontChange);

    OptionsDialog dlg(editCopyContainer, startTab);

    if(dlg.exec() == QDialog::Accepted)
    {
        if(_fontChanged)
            QMessageBox::information(nullptr, QString("Font Changed"), QString("Changes made in the font used by the DMHelper will only be applied when then application is restarted."));

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

    QMainWindow* mainWindow = getMainWindow();
    if(mainWindow)
    {
        mainWindow->restoreGeometry(settings.value("geometry").toByteArray());
        mainWindow->restoreState(settings.value("windowState").toByteArray());
        qDebug() << "[OptionsContainer] Restoring window geometry and state to: " << mainWindow->frameGeometry();
    }

    // Note: password will not be stored in settings
    bool bestiaryExists = true;
    setBestiaryFileName(getSettingsFile(settings, QString("bestiary"), QString("DMHelperBestiary.xml"), &bestiaryExists));
    if((!settings.contains(QString("bestiary"))) || (!bestiaryExists))
        getDataDirectory(QString("Images"), true);
    setLastMonster(settings.value("lastMonster","").toString());

    bool spellbookExists = true;
    setSpellbookFileName(getSettingsFile(settings, QString("spellbook"), QString("spellbook.xml"), &spellbookExists));
    if((!settings.contains(QString("spellbook"))) || (!spellbookExists))
        getDataDirectory(QString("Images"), true);
    setLastSpell(settings.value("lastSpell","").toString());

    setQuickReferenceFileName(getSettingsFile(settings, QString("quickReference"), QString("quickref_data.xml")));
    setCalendarFileName(getSettingsFile(settings, QString("calendar"), QString("calendar.xml")));
    setEquipmentFileName(getSettingsFile(settings, QString("equipment"), QString("equipment.xml")));
    setShopsFileName(getSettingsFile(settings, QString("shops"), QString("shops.xml")));

    //setTablesDirectory(settings.value("tables", getTablesDirectory()).toString());
    setTablesDirectory(getSettingsDirectory(settings, QString("tables"), QString("tables")));

    setShowAnimations(settings.value("showAnimations",QVariant(false)).toBool());
    setFontFamily(settings.value("fontFamily","Trebuchet MS").toString());

    //12*96/72 = 16 Pixels
    //10*96/72 = 13 Pixels
    // 8*96/72 = 10 Pixels
    int defaultFontSize = 10;
    if(_logicalDPI > 0)
        defaultFontSize = (20*72)/_logicalDPI;
    setFontSize(settings.value("fontSize",QVariant(defaultFontSize)).toInt());
    setAudioVolume(settings.value("audioVolume",QVariant(100)).toInt());
    setShowOnDeck(settings.value("showOnDeck",QVariant(true)).toBool());
    setShowCountdown(settings.value("showCountdown",QVariant(true)).toBool());
    setCountdownDuration(settings.value("countdownDuration",QVariant(15)).toInt());
    setPointerFileName(settings.value("pointerFile").toString());
    setSelectedIcon(settings.value("selectedIcon").toString());
    setActiveIcon(settings.value("activeIcon").toString());
    setCombatantFrame(settings.value("combatantFrame").toString());
    setCountdownFrame(settings.value("countdownFrame").toString());

    _dataSettingsExist = (settings.contains("updatesEnabled") || settings.contains("statisticsAccepted"));
    if(_dataSettingsExist)
    {
        setUpdatesEnabled(settings.value("updatesEnabled",QVariant(false)).toBool());
        setStatisticsAccepted(settings.value("statisticsAccepted",QVariant(false)).toBool());
        setLastUpdateDate(settings.value("lastUpdateCheck","").toDate());
    }

    QString uuidString = settings.value("instanceUuid").toString();
    if(uuidString.isEmpty())
        _instanceUuid = QUuid();
    else
        _instanceUuid = QUuid::fromString(uuidString);

#ifdef INCLUDE_NETWORK_SUPPORT
    //setNetworkEnabled(settings.value("networkEnabled",QVariant(false)).toBool());
    setNetworkEnabled(false);
    setURLString(settings.value("url","https://dmh.wwpd.de").toString());
    setUserName(settings.value("username","").toString());
    setSavePassword(settings.value("savePassword",QVariant(false)).toBool());
    setPassword(settings.value("password","").toString());

    settings.beginGroup("Sessions");
        QStringList sessions = settings.childGroups();
        for(QString session : sessions)
        {
            settings.beginGroup(session);
                QString sessionName = settings.value("name").toString();
                if(!sessionName.isEmpty())
                    _sessions.insert(session, sessionName);

                QString invite = settings.value("invite").toString();
                if(!invite.isEmpty())
                    _invites.insert(session, invite);
            settings.endGroup();
        }
    settings.endGroup(); // Sessions

    QString currentSession = settings.value("currentSession").toString();
    if(_sessions.contains(currentSession))
        setCurrentSession(currentSession);

//    setSessionID(settings.value("sessionID","").toString());
//    setInviteID(settings.value("inviteID","").toString());
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

    QMainWindow* mainWindow = getMainWindow();
    if(mainWindow)
    {
        qDebug() << "[OptionsContainer] Storing window geometry and state: " << mainWindow->frameGeometry();
        settings.setValue("geometry", mainWindow->saveGeometry());
        settings.setValue("windowState", mainWindow->saveState());
    }

    // Note: password will not be stored in settings
    settings.setValue("bestiary", getBestiaryFileName());
    settings.setValue("lastMonster", getLastMonster());
    settings.setValue("spellbook", getSpellbookFileName());
    settings.setValue("lastSpell", getLastSpell());
    settings.setValue("quickReference", getQuickReferenceFileName());
    settings.setValue("calendar", getCalendarFileName());
    settings.setValue("equipment", getEquipmentFileName());
    settings.setValue("shops", getShopsFileName());
    settings.setValue("tables", getTablesDirectory());
    settings.setValue("showAnimations", getShowAnimations());
    settings.setValue("fontFamily", getFontFamily());
    settings.setValue("fontSize", getFontSize());
    settings.setValue("audioVolume", getAudioVolume());
    settings.setValue("showOnDeck", getShowOnDeck());
    settings.setValue("showCountdown", getShowCountdown());
    settings.setValue("countdownDuration", getCountdownDuration());
    settings.setValue("pointerFile", getPointerFile());
    settings.setValue("selectedIcon", getSelectedIcon());
    settings.setValue("activeIcon", getActiveIcon());
    settings.setValue("combatantFrame", getCombatantFrame());
    settings.setValue("countdownFrame", getCountdownFrame());

    if(_dataSettingsExist)
    {
        settings.setValue("updatesEnabled", isUpdatesEnabled());
        settings.setValue("statisticsAccepted", isStatisticsAccepted());
        if((!_instanceUuid.isNull()) && (_statisticsAccepted))
            settings.setValue("instanceUuid", _instanceUuid.toString());
        else
            settings.setValue("instanceUuid", QUuid().toString());

        if(isUpdatesEnabled())
            settings.setValue("lastUpdateCheck", _lastUpdateDate);
    }

#ifdef INCLUDE_NETWORK_SUPPORT
    settings.setValue("networkEnabled", getNetworkEnabled());
    settings.setValue("url", getURLString());
    settings.setValue("username", getUserName());
    settings.setValue("savePassword", getSavePassword());
    if(getSavePassword())
        settings.setValue("password", getPassword());

    settings.beginGroup("Sessions");
        QStringList sessions = _sessions.keys();
        for(QString session : sessions)
        {
            settings.beginGroup(session);
                settings.setValue("name", _sessions.value(session));
                QString inviteSession = _invites.value(session);
                if(!inviteSession.isEmpty())
                    settings.setValue("invite", inviteSession);
            settings.endGroup(); // Session
        }
    settings.endGroup(); // Sessions

    settings.setValue("currentSession", getCurrentSession());

    //settings.setValue("sessionID", getSessionID());
    //settings.setValue("inviteID", getInviteID());
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
        qDebug() << "[OptionsContainer] Bestiary filename set to: " << filename;
        emit bestiaryFileNameChanged();
    }
}

void OptionsContainer::setSpellbookFileName(const QString& filename)
{
    if(_spellbookFileName != filename)
    {
        _spellbookFileName = filename;
        qDebug() << "[OptionsContainer] Spellbook filename set to: " << filename;
        emit spellbookFileNameChanged();
    }
}

void OptionsContainer::setQuickReferenceFileName(const QString& filename)
{
    if(_quickReferenceFileName!= filename)
    {
        _quickReferenceFileName = filename;
        qDebug() << "[OptionsContainer] Quick Reference filename set to: " << filename;
        emit quickReferenceFileNameChanged(filename);
    }
}

void OptionsContainer::setCalendarFileName(const QString& filename)
{
    if(_calendarFileName != filename)
    {
        _calendarFileName = filename;
        qDebug() << "[OptionsContainer] Calendar filename set to: " << filename;
        emit calendarFileNameChanged(filename);
    }
}

void OptionsContainer::setEquipmentFileName(const QString& filename)
{
    if(_equipmentFileName != filename)
    {
        _equipmentFileName = filename;
        qDebug() << "[OptionsContainer] Equipment filename set to: " << filename;
        emit equipmentFileNameChanged(filename);
    }
}

void OptionsContainer::setShopsFileName(const QString& filename)
{
    if(_shopsFileName != filename)
    {
        _shopsFileName = filename;
        qDebug() << "[OptionsContainer] Shops filename set to: " << filename;
        emit shopsFileNameChanged(filename);
    }
}

QString OptionsContainer::getSettingsFile(QSettings& settings, const QString& key, const QString& defaultFilename, bool* exists)
{
    QString result = settings.value(key, QVariant()).toString();

    if(result == QString("./bestiary/DMHelperBestiary.xml"))
    {
        qDebug() << "[OptionsContainer] WARNING: old style relative path found for bestiary. Asking user for how to proceed...";
        QMessageBox::StandardButton response = QMessageBox::warning(nullptr,
                                                                    QString("Invalid bestiary path"),
                                                                    QString("Older versions of the DMHelper had a bad choice of location for the bestiary. The file itself is fine, but sometimes the application would get confused where the file is actually located.") + QChar::LineFeed + QChar::LineFeed + QString("Would you like to point the DMHelper at the right location of your Bestiary file now?") + QChar::LineFeed + QChar::LineFeed + QString("If you answer No, it will create a new default bestiary in the ""right"" location for your system."),
                                                                    QMessageBox::Yes | QMessageBox::No);
        if(response == QMessageBox::Yes)
        {
            result = QFileDialog::getOpenFileName(nullptr, QString("Select Bestiary File..."), QString(), QString("XML files (*.xml)"));
            qDebug() << "[OptionsContainer] WARNING: ... user selected file: " << result;
        }
        else
        {
            result = QString();
            qDebug() << "[OptionsContainer] WARNING: ... user does not want to locate their Bestiary, default bestiary will be created";
        }
    }

    if(!result.isEmpty())
    {
        if(exists)
            *exists = true;
        return result;
    }
    else
    {
        return getStandardFile(defaultFilename, exists);
    }
}

QString OptionsContainer::getStandardFile(const QString& defaultFilename, bool* exists)
{
    QString standardPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString standardFile = standardPath + QString("/") + defaultFilename;
    if(QFileInfo::exists(standardFile))
    {
        if(exists)
            *exists = true;
        qDebug() << "[OptionsContainer] Standard File found: " << standardFile;
        return standardFile;
    }

    QString appFile;
#ifdef Q_OS_MAC
    QDir fileDirPath(QCoreApplication::applicationDirPath());
    fileDirPath.cdUp();
    appFile = fileDirPath.path() + QString("/Resources/") + defaultFilename;
#else
    QDir fileDirPath(QCoreApplication::applicationDirPath());
    appFile = fileDirPath.path() + QString("/resources/") + defaultFilename;
#endif

    QDir().mkpath(standardPath);

    if(exists)
        *exists = false;

    if(QFile::copy(appFile, standardFile))
    {
        qDebug() << "[OptionsContainer] Standard default file copied from " << appFile << QString(" to ") << standardFile;
        return standardFile;
    }
    else
    {
        qDebug() << "[OptionsContainer] ERROR: Standard default file copy failed from " << appFile << QString(" to ") << standardFile;
        return QString();
    }
}

void OptionsContainer::setTablesDirectory(const QString& directory)
{
    if(_tablesDirectory != directory)
    {
        _tablesDirectory = directory;
        qDebug() << "[OptionsContainer] Tables directory set to: " << directory;
        emit tablesDirectoryChanged();
    }
}

QString OptionsContainer::getSettingsDirectory(QSettings& settings, const QString& key, const QString& defaultDir)
{
    QString result = settings.value(key, QVariant()).toString();
    if(!result.isEmpty())
        return result;
    else
        return getDataDirectory(defaultDir);
}

QString OptionsContainer::getDataDirectory(const QString& defaultDir, bool overwrite)
{
    bool created = false;
    QString standardPath = getStandardDirectory(defaultDir, &created);
    QDir standardDir(standardPath);
    if(!standardDir.exists())
    {
        qDebug() << "[OptionsContainer] ERROR: Data directory NOT FOUND: " << standardPath;
        return QString();
    }

    if((!created)&&(!overwrite))
    {
        qDebug() << "[OptionsContainer] Data Directory found: " << standardPath;
        return standardPath;
    }

    QString applicationPath = QCoreApplication::applicationDirPath();
    QDir fileDirPath(applicationPath);
#ifdef Q_OS_MAC
    fileDirPath.cdUp();
    if(!fileDirPath.cd(QString("Resources/") + defaultDir))
        return QString();
#else
    if(!fileDirPath.cd(QString("resources/") + defaultDir))
        return QString();
#endif

    QStringList filters;
    filters << "*.xml" << "*.png" << "*.jpg";
    QStringList fileEntries = fileDirPath.entryList(filters);
    for(int i = 0; i < fileEntries.size(); ++i)
    {
        QFile::copy(fileDirPath.filePath(fileEntries.at(i)), standardDir.filePath(fileEntries.at(i)));
    }

    qDebug() << "[OptionsContainer] Data default files copied to directory: " << standardPath;
    return standardPath;
}

QString OptionsContainer::getStandardDirectory(const QString& defaultDir, bool* created)
{
    QString standardPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString result = standardPath + QString("/") + defaultDir;
    QDir standardDir(result);
    if(standardDir.exists())
    {
        qDebug() << "[OptionsContainer] Standard directory found: " << result;
        if(created)
            *created = false;
        return result;
    }

    qDebug() << "[OptionsContainer] Creating standard directory: " << result;
    QDir().mkpath(result);

    if(!standardDir.exists())
    {
        qDebug() << "[OptionsContainer] ERROR: Standard directory creation failed!";
        if(created)
            *created = false;
        return QString();
    }

    qDebug() << "[OptionsContainer] Standard directory created";
    if(created)
        *created = true;
    return result;
}

void OptionsContainer::backupFile(const QString& filename)
{
    QFileInfo fileInfo(filename);

    QString backupPath = getStandardDirectory("backup");
    if(!backupPath.isEmpty())
    {
        QDir backupDir(backupPath);
        QFile previousBackup(backupDir.filePath(fileInfo.fileName()));
        QFileInfo backupFileInfo(previousBackup);
        qDebug() << "[OptionsContainer] Checking backup file: " << previousBackup << " exists: " << backupFileInfo.exists() << ", size: " << backupFileInfo.size() << ", current file size: " << fileInfo.size();
        if((!backupFileInfo.exists()) || (backupFileInfo.size() != fileInfo.size()))
        {
            if(backupFileInfo.exists())
            {
                qDebug() << "[OptionsContainer] Replacing file backup, removing current backup.";
                previousBackup.remove();
            }

            qDebug() << "[OptionsContainer] Backing up file to: " << backupDir.filePath(fileInfo.fileName());
            QFile file(filename);
            file.copy(backupDir.filePath(fileInfo.fileName()));
        }
    }
}

void OptionsContainer::resetFileSettings()
{
    setBestiaryFileName(getStandardFile(QString("DMHelperBestiary.xml")));
    setSpellbookFileName(getStandardFile(QString("spellbook.xml")));
    setQuickReferenceFileName(getStandardFile(QString("quickref_data.xml")));
    setCalendarFileName(getStandardFile(QString("calendar.xml")));
    setEquipmentFileName(getStandardFile(QString("equipment.xml")));
    setShopsFileName(getStandardFile(QString("shops.xml")));
    setTablesDirectory(getDataDirectory(QString("tables"), true));
    getDataDirectory(QString("Images"), true);
}

void OptionsContainer::setLastMonster(const QString& lastMonster)
{
    if(_lastMonster != lastMonster)
    {
        _lastMonster = lastMonster;
    }
}

void OptionsContainer::setLastSpell(const QString& lastSpell)
{
    if(_lastSpell!= lastSpell)
    {
        _lastSpell = lastSpell;
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

void OptionsContainer::setFontFamily(const QString& fontFamily)
{
    if(_fontFamily != fontFamily)
    {
        _fontFamily = fontFamily;
        emit fontFamilyChanged(_fontFamily);
    }
}

void OptionsContainer::setFontFamilyFromFont(const QFont& font)
{
    setFontFamily(font.family());
}


void OptionsContainer::setFontSize(int fontSize)
{
    if(_fontSize != fontSize)
    {
        _fontSize = fontSize;
        emit fontSizeChanged(_fontSize);
    }
}

void OptionsContainer::setLogicalDPI(qreal logicalDPI)
{
    if(logicalDPI != _logicalDPI)
    {
        _logicalDPI = logicalDPI;
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

void OptionsContainer::setPointerFileName(const QString& filename)
{
    if(_pointerFile != filename)
    {
        _pointerFile = filename;
        qDebug() << "[OptionsContainer] Pointer filename set to: " << _pointerFile;
        emit pointerFileNameChanged(_pointerFile);
    }
}

void OptionsContainer::setSelectedIcon(const QString& selectedIcon)
{
    if(_selectedIcon != selectedIcon)
    {
        _selectedIcon = selectedIcon;
        qDebug() << "[OptionsContainer] Selected icon set to: " << _selectedIcon;
        emit selectedIconChanged(_selectedIcon);
    }
}

void OptionsContainer::setActiveIcon(const QString& activeIcon)
{
    if(_activeIcon != activeIcon)
    {
        _activeIcon = activeIcon;
        qDebug() << "[OptionsContainer] Active icon set to: " << _activeIcon;
        emit activeIconChanged(_activeIcon);
    }
}

void OptionsContainer::setCombatantFrame(const QString& combatantFrame)
{
    if(_combatantFrame != combatantFrame)
    {
        _combatantFrame = combatantFrame;
        qDebug() << "[OptionsContainer] Combatant frame set to: " << _combatantFrame;
        emit combatantFrameChanged(_combatantFrame);
    }
}

void OptionsContainer::setCountdownFrame(const QString& countdownFrame)
{
    if(_countdownFrame != countdownFrame)
    {
        _countdownFrame = countdownFrame;
        qDebug() << "[OptionsContainer] Countdown frame set to: " << _countdownFrame;
        emit countdownFrameChanged(_countdownFrame);
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

void OptionsContainer::setUpdatesEnabled(bool updatesEnabled)
{
    _updatesEnabled = updatesEnabled;
    _dataSettingsExist = true;
}

void OptionsContainer::setStatisticsAccepted(bool statisticsAccepted)
{
    _statisticsAccepted = statisticsAccepted;
    _dataSettingsExist = true;
}

void OptionsContainer::setLastUpdateDate(const QDate& date)
{
    _lastUpdateDate = date;
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
        emit networkSettingsChanged(_urlString, _userName, _userId, _password, _currentSession, _invites.value(_currentSession));
    }
}

void OptionsContainer::setUserName(const QString& username)
{
    if(_userName != username)
    {
        _userName = username;
        _userId.clear();
        emit userNameChanged(_userName);
        emit userIdChanged(_userId);
        emit networkSettingsChanged(_urlString, _userName, _userId, _password, _currentSession, _invites.value(_currentSession));
    }
}

void OptionsContainer::setUserId(const QString& userId)
{
    if(_userId != userId)
    {
        _userId = userId;
        emit userIdChanged(_userId);
        emit networkSettingsChanged(_urlString, _userName, _userId, _password, _currentSession, _invites.value(_currentSession));
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
        emit networkSettingsChanged(_urlString, _userName, _userId, _password, _currentSession, _invites.value(_currentSession));
    }
}

void OptionsContainer::setCurrentSession(const QString& session)
{
    if(_currentSession != session)
    {
        _currentSession = session;
        emit currentSessionChanged(_currentSession);
        emit networkSettingsChanged(_urlString, _userName, _userId, _password, _currentSession, _invites.value(_currentSession));
    }
}

void OptionsContainer::addSession(const QString& session, const QString& sessionName)
{
    if((session.isEmpty()) || (_sessions.contains(session)))
        return;

    _sessions.insert(session, sessionName);
    emit sessionChanged(session, sessionName);
}

void OptionsContainer::setSessionName(const QString& session, const QString& sessionName)
{
    if((session.isEmpty()) || (!_sessions.contains(session)))
        return;

    _sessions[session] = sessionName;
    emit sessionChanged(session, sessionName);
}

void OptionsContainer::removeSession(const QString& session)
{
    if((session.isEmpty()) || (!_sessions.contains(session)))
        return;

    emit sessionChanged(session, _sessions.value(session));
    _sessions.remove(session);
}

void OptionsContainer::addInvite(const QString& session, const QString& invite)
{
    if((session.isEmpty()) || (invite.isEmpty()) || (_invites.contains(session)))
        return;

    _invites.insert(session, invite);
    emit inviteChanged(session, invite);
}

void OptionsContainer::setInvite(const QString& session, const QString& invite)
{
    if((session.isEmpty()) || (invite.isEmpty()))
        return;

    if(!_invites.contains(session))
    {
        addInvite(session, invite);
    }
    else
    {
        _invites[session] = invite;
        emit inviteChanged(session, QString());
    }
}

void OptionsContainer::removeInvite(const QString& session)
{
    if((session.isEmpty()) || (!_invites.contains(session)))
        return;

    _invites.remove(session);
    emit inviteChanged(session, QString());
}

/*
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
*/

#endif //INCLUDE_NETWORK_SUPPORT

void OptionsContainer::registerFontChange()
{
    _fontChanged = true;
}

void OptionsContainer::copy(OptionsContainer* other)
{
    if(other)
    {
        setBestiaryFileName(other->_bestiaryFileName);
        setSpellbookFileName(other->_spellbookFileName);
        setQuickReferenceFileName(other->_quickReferenceFileName);
        setCalendarFileName(other->_calendarFileName);
        setEquipmentFileName(other->_equipmentFileName);
        setShopsFileName(other->_shopsFileName);
        setTablesDirectory(other->_tablesDirectory);
        setLastMonster(other->_lastMonster);
        setLastSpell(other->_lastSpell);
        setShowAnimations(other->_showAnimations);
        setFontFamily(other->_fontFamily);
        setFontSize(other->_fontSize);
        setShowOnDeck(other->_showOnDeck);
        setShowCountdown(other->_showCountdown);
        setCountdownDuration(other->_countdownDuration);
        setPointerFileName(other->_pointerFile);
        setSelectedIcon(other->_selectedIcon);
        setActiveIcon(other->_activeIcon);
        setCombatantFrame(other->_combatantFrame);
        setCountdownFrame(other->_countdownFrame);
        _dataSettingsExist = other->_dataSettingsExist;
        _updatesEnabled = other->_updatesEnabled;
        _statisticsAccepted = other->_statisticsAccepted;
        _instanceUuid = QUuid::fromString(other->_instanceUuid.toString());
        _lastUpdateDate = other->_lastUpdateDate;
#ifdef INCLUDE_NETWORK_SUPPORT
        setURLString(other->_urlString);
        setUserName(other->_userName);
        setSavePassword(other->_savePassword);
        setPassword(other->_password);
        //setSessionID(other->_sessionID);
        //setInviteID(other->_inviteID);
        setNetworkEnabled(other->_networkEnabled);
        setCurrentSession(other->_currentSession);

        QStringList sessionList = other->_sessions.keys();
        for(QString session : sessionList)
            _sessions.insert(session, other->_sessions.value(session));

        QStringList inviteList = other->_invites.keys();
        for(QString inviteSession : inviteList)
            _invites.insert(inviteSession, other->_invites.value(inviteSession));
#endif
    }
}

QMainWindow* OptionsContainer::getMainWindow()
{
    return dynamic_cast<QMainWindow*>(parent());
}
