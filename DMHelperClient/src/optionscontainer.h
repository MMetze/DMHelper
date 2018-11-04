#ifndef OPTIONSCONTAINER_H
#define OPTIONSCONTAINER_H

#include <QObject>
#include <QString>
//#include "mruhandler.h"

class OptionsContainer : public QObject
{
    Q_OBJECT
public:
    OptionsContainer(QObject *parent = 0);
    ~OptionsContainer();

    // General settings
    QString getURLString() const;
    QString getUserName() const;
    QString getPassword() const;
    QString getSession() const;

    //MRUHandler* getMRUHandler() const;
    //void setMRUHandler(MRUHandler* mruHandler);

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

    void copy(OptionsContainer& other);

private:

    // General settings
    QString _urlString;
    QString _userName;
    QString _password;
    QString _session;

    //MRUHandler* _mruHandler;
};

#endif // OPTIONSCONTAINER_H
