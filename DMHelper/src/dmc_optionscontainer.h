#ifndef DMC_OPTIONSCONTAINER_H
#define DMC_OPTIONSCONTAINER_H

#include <QObject>
#include <QString>
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
    QString getPassword() const;
    QString getSession() const;
    DMHLogon getLogon() const;

    // File settings
    QString getCacheDirectory() const;

signals:
    void urlStringChanged(const QString& urlString);
    void userNameChanged(const QString& userName);
    void passwordChanged(const QString& password);
    void sessionChanged(const QString& session);
    void cacheDirectoryChanged(const QString& cacheDirectory);

public slots:
    void readSettings();
    void writeSettings();

    // General settings
    void setURLString(const QString& urlString);
    void setUserName(const QString& username);
    void setPassword(const QString& password);
    void setSession(const QString& session);

    // File settings
    void setCacheDirectory(const QString& cacheDirectory);
    QString getStandardDirectory(const QString& defaultDir);

    void copy(DMC_OptionsContainer& other);

private:

    // General settings
    QString _urlString;
    QString _userName;
    QString _password;
    QString _session;

    // File settings
    QString _cacheDirectory;
};

#endif // DMC_OPTIONSCONTAINER_H
