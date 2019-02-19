#ifndef OPTIONSCONTAINER_H
#define OPTIONSCONTAINER_H

#include <QObject>
#include <QString>
#include "mruhandler.h"
#include "dmconstants.h"

class OptionsContainer : public QObject
{
    Q_OBJECT
public:
    OptionsContainer(QObject *parent = nullptr);
    ~OptionsContainer();

    // General settings
    QString getBestiaryFileName() const;
#ifdef INCLUDE_CHASE_SUPPORT
    QString getChaseFileName() const;
#endif
    QString getLastMonster() const;
    bool getShowAnimations() const;

    // Audio settings
    int getAudioVolume() const;

    // Battle settings
    bool getShowOnDeck() const;
    bool getShowCountdown() const;
    int getCountdownDuration() const;

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
#ifdef INCLUDE_CHASE_SUPPORT
    void chaseFileNameChanged();
#endif
    void showAnimationsChanged(bool showAnimations);

    // Audio settings
    void audioVolumeChanged(int volume);

    // Battle settings
    void showOnDeckChanged(bool showOnDeck);
    void showCountdownChanged(bool showCountdown);
    void countdownDurationChanged(int countdownDuration);

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
#ifdef INCLUDE_CHASE_SUPPORT
    void setChaseFileName(const QString& filename);
#endif
    void setLastMonster(const QString& lastMonster);
    void setShowAnimations(bool showAnimations);

    // Audio settings
    void setAudioVolume(int volume);

    // Battle settings
    void setShowOnDeck(bool showOnDeck);
    void setShowCountdown(bool showCountdown);
    void setCountdownDuration(int countdownDuration);
    void setCountdownDuration(const QString& countdownDuration);

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

private:
    void copy(OptionsContainer* other);

    // General settings
    QString _bestiaryFileName;
    QString _lastMonster;
#ifdef INCLUDE_CHASE_SUPPORT
    QString _chaseFileName;
#endif
    bool _showAnimations;

    // Audio settings
    int _audioVolume;

    // Battle settings
    bool _showOnDeck;
    bool _showCountdown;
    int _countdownDuration;

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
