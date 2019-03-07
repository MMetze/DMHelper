#include "bestiary.h"
#include "monsterclass.h"
#include "monster.h"
#include "dmconstants.h"
#include <QDomDocument>
#include <QDomElement>
#include <QDir>
#include <QPixmap>
#include <QMessageBox>
#include <QDebug>

Bestiary* Bestiary::_instance = nullptr;

Bestiary::Bestiary(QObject *parent) :
    QObject(parent),
    _bestiaryMap(),
    _bestiaryDirectory(),
    _majorVersion(0),
    _minorVersion(0),
    _licenseText()
{
}

Bestiary::~Bestiary()
{
    qDeleteAll(_bestiaryMap);
}

Bestiary* Bestiary::Instance()
{
    return _instance;
}

void Bestiary::Initialize()
{
    if(_instance)
        return;

    qDebug() << "[Bestiary] Initializing Bestiary";
    _instance = new Bestiary();
}

void Bestiary::Shutdown()
{
    delete _instance;
}

void Bestiary::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory) const
{
    qDebug() << "[Bestiary] Saving bestiary...";
    QDomElement bestiaryElement = doc.createElement( "bestiary" );
    bestiaryElement.setAttribute( "majorversion", _majorVersion );
    bestiaryElement.setAttribute( "minorversion", _minorVersion );

    qDebug() << "[Bestiary]    Storing " << _bestiaryMap.count() << " creatures.";
    BestiaryMap::const_iterator i = _bestiaryMap.constBegin();
    while (i != _bestiaryMap.constEnd())
    {
        MonsterClass* monsterClass = i.value();
        if(monsterClass)
        {
            QDomElement monsterElement = doc.createElement("element");
            monsterClass->outputXML(doc, monsterElement, targetDirectory);
            bestiaryElement.appendChild(monsterElement);
        }

        ++i;
    }

    QDomElement licenseElement = doc.createElement( QString("license") );
    for(QString licenseText : _licenseText)
    {
        QDomElement licenseTextElement = doc.createElement(QString("element"));
        licenseTextElement.appendChild(doc.createTextNode(licenseText));
        licenseElement.appendChild(licenseTextElement);
    }
    bestiaryElement.appendChild(licenseElement);

    parent.appendChild(bestiaryElement);
    qDebug() << "[Bestiary] Saving bestiary completed";
}

void Bestiary::inputXML(const QDomElement &element)
{
    qDebug() << "[Bestiary] Loading bestiary...";

    QDomElement bestiaryElement = element.firstChildElement( QString("bestiary") );
    if(bestiaryElement.isNull())
    {
        qDebug() << "[Bestiary]    ERROR: invalid bestiary file";
        return;
    }

    _majorVersion = bestiaryElement.attribute("majorversion",QString::number(1)).toInt();
    _minorVersion = bestiaryElement.attribute("minorversion",QString::number(0)).toInt();
    qDebug() << "[Bestiary]    Bestiary version: " << getVersion();
    if(!isVersionCompatible())
    {
        qDebug() << "[Bestiary]    ERROR: New Bestiary version is not compatible with expected version: " << getExpectedVersion();
        return;
    }

    if (!isVersionIdentical())
        qDebug() << "[Bestiary]    WARNING: Bestiary version is not the same as expected version: " << getExpectedVersion();

    if(_bestiaryMap.count() > 0)
    {
        qDebug() << "[Bestiary]    Unloading previous bestiary";
        qDeleteAll(_bestiaryMap);
        _bestiaryMap.clear();
    }

    QDomElement monsterElement = bestiaryElement.firstChildElement( QString("element") );
    while( !monsterElement.isNull() )
    {
        MonsterClass* monster = new MonsterClass(monsterElement);
        insertMonsterClass(monster);
        monsterElement = monsterElement.nextSiblingElement( QString("element") );
    }

    QDomElement licenseElement = bestiaryElement.firstChildElement( QString("license") );
    if(licenseElement.isNull())
        qDebug() << "[Bestiary] ERROR: not able to find the license text in the bestiary!";

    QDomElement licenseText = licenseElement.firstChildElement(QString("element"));
    while(!licenseText.isNull())
    {
        _licenseText.append(licenseText.text());
        licenseText = licenseText.nextSiblingElement(QString("element"));
    }

    qDebug() << "[Bestiary] Loading bestiary completed. " << _bestiaryMap.count() << " creatures loaded.";

    emit changed();
}

QString Bestiary::getVersion() const
{
    return QString::number(_majorVersion) + "." + QString::number(_minorVersion);
}

bool Bestiary::isVersionCompatible() const
{
    return (_majorVersion == DMHelper::BESTIARY_MAJOR_VERSION);
}

bool Bestiary::isVersionIdentical() const
{
    return ((_majorVersion == DMHelper::BESTIARY_MAJOR_VERSION) && (_minorVersion == DMHelper::BESTIARY_MINOR_VERSION));
}

QString Bestiary::getExpectedVersion()
{
    return QString::number(DMHelper::BESTIARY_MAJOR_VERSION) + "." + QString::number(DMHelper::BESTIARY_MINOR_VERSION);
}

bool Bestiary::exists(const QString& name) const
{
    return _bestiaryMap.contains(name);
}

int Bestiary::count() const
{
    return _bestiaryMap.count();
}

QList<QString> Bestiary::getMonsterList() const
{
    return _bestiaryMap.keys();
}

QStringList Bestiary::getLicenseText() const
{
    return _licenseText;
}

MonsterClass* Bestiary::getMonsterClass(const QString& name) const
{
    if(!_bestiaryMap.contains(name))
    {
        qDebug() << "[Bestiary] ERROR: Requested monster class not found: " << name;
        QMessageBox::critical(nullptr, QString("Unknown monster"), QString("WARNING: The monster """) + name + QString(""" was not found in the current bestiary! If you save the current campaign, all references to this monster will be lost!"));
    }

    return _bestiaryMap.value(name, nullptr);
}

MonsterClass* Bestiary::getFirstMonsterClass() const
{
    if(_bestiaryMap.count() == 0)
        return nullptr;

    return _bestiaryMap.first();
}

MonsterClass* Bestiary::getLastMonsterClass() const
{
    if(_bestiaryMap.count() == 0)
        return nullptr;

    return _bestiaryMap.last();
}

MonsterClass* Bestiary::getNextMonsterClass(MonsterClass* monsterClass) const
{
    if(!monsterClass)
        return nullptr;

    BestiaryMap::const_iterator i = _bestiaryMap.find(monsterClass->getName());
    if(i == _bestiaryMap.constEnd())
        return nullptr;

    ++i;

    if(i == _bestiaryMap.constEnd())
        return nullptr;

    return i.value();
}

MonsterClass* Bestiary::getPreviousMonsterClass(MonsterClass* monsterClass) const
{
    if(!monsterClass)
        return nullptr;

    BestiaryMap::const_iterator i = _bestiaryMap.find(monsterClass->getName());
    if(i == _bestiaryMap.constBegin())
        return nullptr;

    --i;
    return i.value();
}

bool Bestiary::insertMonsterClass(MonsterClass* monsterClass)
{
    if(_bestiaryMap.contains(monsterClass->getName()))
        return false;

    _bestiaryMap.insert(monsterClass->getName(), monsterClass);
    emit changed();
    return true;
}

void Bestiary::removeMonsterClass(MonsterClass* monsterClass)
{
    if(!_bestiaryMap.contains(monsterClass->getName()))
        return;

    _bestiaryMap.remove(monsterClass->getName());
    delete monsterClass;
    emit changed();
}

void Bestiary::renameMonster(MonsterClass* monsterClass, const QString& newName)
{
    if(!_bestiaryMap.contains(monsterClass->getName()))
        return;

    _bestiaryMap.remove(monsterClass->getName());
    monsterClass->setName(newName);
    insertMonsterClass(monsterClass);
}

void Bestiary::setDirectory(const QDir& directory)
{
    _bestiaryDirectory = directory;
}

const QDir& Bestiary::getDirectory() const
{
    return _bestiaryDirectory;
}

Monster* Bestiary::createMonster(const QString& name) const
{
    if(!_bestiaryMap.contains(name))
    {
        qDebug() << "[Bestiary] ERROR: Requested monster class not found: " << name;
        QMessageBox::critical(nullptr, QString("Unknown monster"), QString("WARNING: The monster """) + name + QString(""" was not found in the current bestiary! If you save the current campaign, all references to this monster will be lost!"));
        return nullptr;
    }

    return new Monster(getMonsterClass(name));
}

Monster* Bestiary::createMonster(const QDomElement& element) const
{
    QString monsterName = element.attribute(QString("monsterClass"));

    if(!_bestiaryMap.contains(monsterName))
    {
        qDebug() << "[Bestiary] ERROR: Requested monster class not found: " << monsterName;
        QMessageBox::critical(nullptr, QString("Unknown monster"), QString("WARNING: The monster """) + monsterName + QString(""" was not found in the current bestiary! If you save the current campaign, all references to this monster will be lost!"));
        return nullptr;
    }

    return new Monster(getMonsterClass(monsterName), element);
}

QString Bestiary::findMonsterImage(const QString& monsterName, const QString& iconFile)
{
    QString fileName = iconFile;

    if((fileName.isEmpty()) || ( !_bestiaryDirectory.exists(fileName) ))
    {
        fileName = QString("./") + monsterName + QString(".png");
        if( !_bestiaryDirectory.exists(fileName) )
        {
            fileName = QString("./") + monsterName + QString(".jpg");
            if( !_bestiaryDirectory.exists(fileName) )
            {
                fileName = QString("./Images/") + monsterName + QString(".png");
                if( !_bestiaryDirectory.exists(fileName) )
                {
                    fileName = QString("./Images/") + monsterName + QString(".jpg");
                    if( !_bestiaryDirectory.exists(fileName) )
                    {
                        qDebug() << "[Bestiary] Not able to find monster image for " << monsterName << " with icon file " << iconFile;
                        fileName = QString();
                    }
                }
            }
        }
    }

    return fileName;
}
