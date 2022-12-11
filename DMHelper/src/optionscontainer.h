#ifndef OPTIONSCONTAINER_H
#define OPTIONSCONTAINER_H

#include <QMainWindow>
#include <QString>
#include <QUuid>
#include <QDate>
#include "mruhandler.h"
#include "dmconstants.h"

class OptionsAccessor;

class OptionsContainer : public QObject
{
    Q_OBJECT
public:
    explicit OptionsContainer(QMainWindow *parent = nullptr);
    ~OptionsContainer();

    // General settings
    QString getBestiaryFileName() const;
    QString getSpellbookFileName() const;
    QString getQuickReferenceFileName() const;
    QString getCalendarFileName() const;
    QString getEquipmentFileName() const;
    QString getShopsFileName() const;
    QString getTablesDirectory() const;
    QString getLastMonster() const;
    QString getLastSpell() const;
    bool getShowAnimations() const;

    // Font settings
    QString getFontFamily() const;
    int getFontSize() const;
    qreal getLogicalDPI() const;

    // Text settings
    bool getPasteRich() const;

    // Audio settings
    int getAudioVolume() const;

    // Battle settings
    int getInitiativeType() const;
    bool getShowCountdown() const;
    int getCountdownDuration() const;
    QString getPointerFile() const;
    QString getSelectedIcon() const;
    QString getActiveIcon() const;
    QString getCombatantFrame() const;
    QString getCountdownFrame() const;
    bool getGridLocked() const;
    qreal getGridLockScale() const;

    // Data settings
    QString getLastAppVersion() const;
    bool doDataSettingsExist() const;
    bool isUpdatesEnabled() const;
    bool isStatisticsAccepted() const;
    QUuid getInstanceUuid();
    QString getInstanceUuidStr();
    QDate getLastUpdateCheck() const;
    QString getHeroForgeToken() const;

#ifdef INCLUDE_NETWORK_SUPPORT
    // Network settings
    bool getNetworkEnabled() const;
    QString getURLString() const;
    QString getUserName() const;
    bool getSavePassword() const;
    QString getPassword() const; // note: password will not be stored in settings
    QString getSessionID() const;
    QString getInviteID() const;
#endif

    MRUHandler* getMRUHandler() const;
    void setMRUHandler(MRUHandler* mruHandler);

signals:
    // General settings
    void bestiaryFileNameChanged();
    void spellbookFileNameChanged();
    void quickReferenceFileNameChanged(const QString& filename);
    void calendarFileNameChanged(const QString& filename);
    void equipmentFileNameChanged(const QString& filename);
    void shopsFileNameChanged(const QString& filename);
    void tablesDirectoryChanged();
    void showAnimationsChanged(bool showAnimations);

    // Font settings
    void fontFamilyChanged(const QString& fontFamily);
    void fontSizeChanged(int fontSize);

    // Text settings
    void pasteRichChanged(bool pasteRich);

    // Audio settings
    void audioVolumeChanged(int volume);

    // Battle settings
    void initiativeTypeChanged(int initiativeType);
    void showCountdownChanged(bool showCountdown);
    void countdownDurationChanged(int countdownDuration);
    void pointerFileNameChanged(const QString& filename);
    void selectedIconChanged(const QString& selectedIcon);
    void activeIconChanged(const QString& activeIcon);
    void combatantFrameChanged(const QString& combatantFrame);
    void countdownFrameChanged(const QString& countdownFrame);
    void gridLockedChanged(bool gridLocked);
    void gridLockScaleChanged(qreal gridLockScale);

    // Data settings
    void heroForgeTokenChanged(const QString& token);

#ifdef INCLUDE_NETWORK_SUPPORT
    // Network settings
    void networkEnabledChanged(bool enabled);
    void urlStringChanged(const QString& urlString);
    void userNameChanged(const QString& username);
    void savePasswordChanged(bool savePassword);
    void passwordChanged(const QString& password); // note: password will not be stored in settings
    void sessionIDChanged(const QString& sessionID);
    void inviteIDChanged(const QString& inviteID);
    void networkSettingsChanged(const QString& urlString, const QString& username, const QString& password, const QString& sessionID, const QString& inviteID);
#endif

public slots:
    void editSettings();
    void readSettings();
    void writeSettings();

    // General settings
    void setBestiaryFileName(const QString& filename);
    void setSpellbookFileName(const QString& filename);
    void setQuickReferenceFileName(const QString& filename);
    void setCalendarFileName(const QString& filename);
    void setEquipmentFileName(const QString& filename);
    void setShopsFileName(const QString& filename);
    QString getSettingsFile(OptionsAccessor& settings, const QString& key, const QString& defaultFilename, bool* exists = nullptr);
    QString getStandardFile(const QString& defaultFilename, bool* exists = nullptr);
    void setTablesDirectory(const QString& directory);
    QString getSettingsDirectory(OptionsAccessor& settings, const QString& key, const QString& defaultDir);
    QString getDataDirectory(const QString& defaultDir, bool overwrite = false);
    QString getStandardDirectory(const QString& defaultDir, bool* created = nullptr);
    void backupFile(const QString& filename);
    void resetFileSettings();

    void setLastMonster(const QString& lastMonster);
    void setLastSpell(const QString& lastSpell);
    void setShowAnimations(bool showAnimations);

    // Font settings
    void setFontFamily(const QString& fontFamily);
    void setFontFamilyFromFont(const QFont& font);
    void setFontSize(int fontSize);
    void setLogicalDPI(qreal logicalDPI);

    // Text settings
    void setPasteRich(bool pasteRich);

    // Audio settings
    void setAudioVolume(int volume);

    // Battle settings
    void setInitiativeType(int initiativeType);
    void setShowCountdown(bool showCountdown);
    void setCountdownDuration(int countdownDuration);
    void setCountdownDuration(const QString& countdownDuration);
    void setPointerFileName(const QString& filename);
    void setSelectedIcon(const QString& selectedIcon);
    void setActiveIcon(const QString& activeIcon);
    void setCombatantFrame(const QString& combatantFrame);
    void setCountdownFrame(const QString& countdownFrame);
    void setGridLocked(bool gridLocked);
    void setGridLockScale(qreal gridLockScale);

    // Data settings
    void setUpdatesEnabled(bool updatesEnabled);
    void setStatisticsAccepted(bool statisticsAccepted);
    void setLastUpdateDate(const QDate& date);
    void setHeroForgeToken(const QString& token);

#ifdef INCLUDE_NETWORK_SUPPORT
    // Network settings
    void setNetworkEnabled(bool enabled);
    void setURLString(const QString& urlString);
    void setUserName(const QString& username);
    void setSavePassword(bool savePassword);
    void setPassword(const QString& password); // note: password will not be stored in settings
    void setSessionID(const QString& sessionID);
    void setInviteID(const QString& inviteID);
#endif

private slots:
    void registerFontChange();

private:
    void copy(OptionsContainer* other);
    QMainWindow* getMainWindow();
    void cleanupLegacy(OptionsAccessor& settings);

    // General settings
    QString _bestiaryFileName;
    QString _spellbookFileName;
    QString _lastMonster;
    QString _lastSpell;
    QString _quickReferenceFileName;
    QString _calendarFileName;
    QString _equipmentFileName;
    QString _shopsFileName;
    QString _tablesDirectory;
    bool _showAnimations;

    // Font settings
    QString _fontFamily;
    int _fontSize;
    qreal _logicalDPI;
    bool _fontChanged;

    // Text settings
    bool _pasteRich;

    // Audio settings
    int _audioVolume;

    // Battle settings
    int _initiativeType;
    bool _showCountdown;
    int _countdownDuration;
    QString _pointerFile;
    QString _selectedIcon;
    QString _activeIcon;
    QString _combatantFrame;
    QString _countdownFrame;
    bool _gridLocked;
    qreal _gridLockScale;

    // Data settings
    QString _lastAppVersion;
    bool _dataSettingsExist;
    bool _updatesEnabled;
    bool _statisticsAccepted;
    QUuid _instanceUuid;
    QDate _lastUpdateDate;
    QString _heroForgeToken;

#ifdef INCLUDE_NETWORK_SUPPORT
    // Network settings
    bool _networkEnabled;
    QString _urlString;
    QString _userName;
    bool _savePassword;
    QString _password; // note: password will not be stored in settings
    QString _sessionID;
    QString _inviteID;
#endif

    MRUHandler* _mruHandler;
};

#endif // OPTIONSCONTAINER_H
