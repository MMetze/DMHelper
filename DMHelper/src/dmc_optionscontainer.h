#ifndef DMC_OPTIONSCONTAINER_H
#define DMC_OPTIONSCONTAINER_H

#include <QObject>
#include <QString>
#include <QMap>
#include "dmhlogon.h"

class DMC_OptionsContainer : public QObject
{
    Q_OBJECT
public:
    DMC_OptionsContainer(QObject *parent = 0);
    ~DMC_OptionsContainer();

    // General settings
    QString getURLString() const;
    QString getUserName() const;
    QString getUserId() const;
    bool getSavePassword() const;
    QString getPassword() const;

    QString getCurrentInvite() const;
    QStringList getInvites() const;
    QString getInviteName(const QString& invite) const;

    DMHLogon getLogon() const;

    // File settings
    QString getCacheDirectory() const;

signals:
    void urlStringChanged(const QString& urlString);
    void userNameChanged(const QString& userName);
    void userIdChanged(const QString& userId);
    void savePasswordChanged(bool savePassword);
    void passwordChanged(const QString& password);

    void currentInviteChanged(const QString& invite);
    void inviteChanged(const QString& invite, const QString& inviteName);

    void cacheDirectoryChanged(const QString& cacheDirectory);

public slots:
    void readSettings();
    void writeSettings();

    // General settings
    void setURLString(const QString& urlString);
    void setUserName(const QString& username);
    void setUserId(const QString& userId);
    void setSavePassword(bool savePassword);
    void setPassword(const QString& password);

    void setCurrentInvite(const QString& invite);
    void addInvite(const QString& invite, const QString& inviteName);
    void setInviteName(const QString& invite, const QString& inviteName);
    void removeInvite(const QString& invite);

    // File settings
    void setCacheDirectory(const QString& cacheDirectory);
    QString getStandardDirectory(const QString& defaultDir);

    void copy(DMC_OptionsContainer& other);

private:

    // General settings
    QString _urlString;
    QString _userName;
    QString _userId;
    bool _savePassword;
    QString _password;
    //QString _session;
    QString _currentInvite;
    QMap<QString, QString> _invites;


    // File settings
    QString _cacheDirectory;
};

#endif // DMC_OPTIONSCONTAINER_H
