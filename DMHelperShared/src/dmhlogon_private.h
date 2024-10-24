#ifndef DMHLOGON_PRIVATE_H
#define DMHLOGON_PRIVATE_H

#include <QString>

class DMHLogon_Private
{
public:
    DMHLogon_Private();
    DMHLogon_Private(const QString& urlString, const QString& username, const QString& password, const QString& session);
    DMHLogon_Private(const DMHLogon_Private& other);
    virtual ~DMHLogon_Private();
    DMHLogon_Private& operator=(const DMHLogon_Private& other);

    QString getURLString() const;
    QString getUserName() const;
    QString getPassword() const;
    QString getSession() const;

    void setURLString(const QString& urlString);
    void setUserName(const QString& username);
    void setPassword(const QString& password);
    void setSession(const QString& session);

private:
    QString _urlString;
    QString _userName;
    QString _password;
    QString _session;
};

#endif // DMHLOGON_PRIVATE_H
