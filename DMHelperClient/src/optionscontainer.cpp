#include "optionscontainer.h"
#include <QSettings>

// TODO: consider copy of MRU functionality

OptionsContainer::OptionsContainer(QObject *parent) :
    QObject(parent),
    _urlString(),
    _userName(),
    _password(QString("Ente12345")),
    _session()
    //_mruHandler(0)
{
}

OptionsContainer::~OptionsContainer()
{
}

QString OptionsContainer::getURLString() const
{
    return _urlString;
}

QString OptionsContainer::getUserName() const
{
    return _userName;
}

QString OptionsContainer::getPassword() const
{
    return _password;
}

QString OptionsContainer::getSession() const
{
    return _session;
}

/*
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
*/

void OptionsContainer::readSettings()
{
    QSettings settings("Glacial Software", "DMHelperClient");

#ifdef QT_DEBUG
    settings.beginGroup("DEBUG");
#endif

    setURLString(settings.value("url","").toString());
    setUserName(settings.value("username","").toString());
    setSession(settings.value("session","").toString());

    /*
    if(_mruHandler)
    {
        _mruHandler->readMRUFromSettings(settings);
    }
    */

#ifdef QT_DEBUG
    settings.endGroup(); // DEBUG
#endif
}

void OptionsContainer::writeSettings()
{
    QSettings settings("Glacial Software", "DMHelperClient");

#ifdef QT_DEBUG
    settings.beginGroup("DEBUG");
#endif

    settings.setValue("url", getURLString());
    settings.setValue("username", getUserName());
    settings.setValue("session", getSession());

    /*
    if(_mruHandler)
    {
        _mruHandler->writeMRUToSettings(settings);
    }
    */

#ifdef QT_DEBUG
    settings.endGroup(); // DEBUG
#endif
}

void OptionsContainer::setURLString(const QString& urlString)
{
    if(_urlString != urlString)
    {
        _urlString = urlString;
        emit urlStringChanged();
    }
}

void OptionsContainer::setUserName(const QString& username)
{
    if(_userName != username)
    {
        _userName = username;
        emit userNameChanged();
    }
}

void OptionsContainer::setPassword(const QString& password)
{
    if(_password != password)
    {
        _password = password;
        emit passwordChanged();
    }
}

void OptionsContainer::setSession(const QString& session)
{
    if(_session != session)
    {
        _session = session;
        emit sessionChanged();
    }
}

void OptionsContainer::copy(OptionsContainer& other)
{
    setURLString(other._urlString);
    setUserName(other._userName);
    setPassword(other._password);
    setSession(other._session);
}
