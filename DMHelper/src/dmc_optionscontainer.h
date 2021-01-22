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

signals:
    void urlStringChanged();
    void userNameChanged();
    void passwordChanged();
    void sessionChanged();

public slots:
    //void editSettings();
    void readSettings();
    void writeSettings();

    // General settings
    void setURLString(const QString& urlString);
    void setUserName(const QString& username);
    void setPassword(const QString& password);
    void setSession(const QString& session);

    void copy(DMC_OptionsContainer& other);

private:

    // General settings
    QString _urlString;
    QString _userName;
    QString _password;
    QString _session;
};

#endif // DMC_OPTIONSCONTAINER_H
