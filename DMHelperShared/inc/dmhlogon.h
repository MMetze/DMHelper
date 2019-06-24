#ifndef DMHLOGON_H
#define DMHLOGON_H

#include "dmhglobal.h"
#include <QString>
#include <memory>

class DMHLogon_Private;

//class DMHSHARED_EXPORT DMHLogon
class DMHLogon
{
public:
    DMHLogon();
    DMHLogon(const QString& urlString, const QString& username, const QString& password, const QString& session);
    DMHLogon(const DMHLogon& other);
    virtual ~DMHLogon();
    DMHLogon& operator=(const DMHLogon& other);

    QString getURLString() const;
    QString getUserName() const;
    QString getPassword() const;
    QString getSession() const;

    void setURLString(const QString& urlString);
    void setUserName(const QString& username);
    void setPassword(const QString& password);
    void setSession(const QString& session);

private:
    std::unique_ptr<DMHLogon_Private> d;
};

#endif // DMHLOGON_H
