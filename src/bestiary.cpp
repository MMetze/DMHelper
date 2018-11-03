#include "bestiary.h"
#include "monsterclass.h"
#include "monster.h"
#include "dmconstants.h"
#include <QDomDocument>
#include <QDomElement>
#include <QDir>
#include <QPixmap>
#include <QDebug>

Bestiary* Bestiary::_instance = NULL;

Bestiary::Bestiary(QObject *parent) :
    QObject(parent),
    _bestiaryMap(),
    _bestiaryDirectory(),
    _majorVersion(0),
    _minorVersion(0)
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
            QDomElement monsterElement = doc.createElement( "monster" );
            monsterClass->outputXML(doc, monsterElement, targetDirectory);
            bestiaryElement.appendChild(monsterElement);
        }

        ++i;
    }

    parent.appendChild(bestiaryElement);
    qDebug() << "[Bestiary] Saving bestiary completed";
}

void Bestiary::inputXML(const QDomElement &element)
{
    qDebug() << "[Bestiary] Loading bestiary...";
    if(_bestiaryMap.count() > 0)
    {
        qDebug() << "[Bestiary]    Unloading previous bestiary";
        qDeleteAll(_bestiaryMap);
        _bestiaryMap.clear();
    }

    QDomElement bestiaryElement = element.firstChildElement( QString("bestiary") );
    if( !bestiaryElement.isNull() )
    {
        _majorVersion = bestiaryElement.attribute("majorversion",QString::number(1)).toInt();
        _minorVersion = bestiaryElement.attribute("minorversion",QString::number(0)).toInt();
        qDebug() << "[Bestiary]    Bestiary version: " << getVersion();
        if(!isVersionCompatible())
            qDebug() << "[Bestiary]    WARNING: Bestiary version is not compatible with expected version: " << getExpectedVersion();
        else if (!isVersionIdentical())
            qDebug() << "[Bestiary]    NOTE: Bestiary version is not the same as expected version: " << getExpectedVersion();

        QDomElement monsterElement = bestiaryElement.firstChildElement( QString("monster") );
        while( !monsterElement.isNull() )
        {
            MonsterClass* monster = new MonsterClass(monsterElement);
            insertMonsterClass(monster);
            monsterElement = monsterElement.nextSiblingElement( QString("monster") );
        }
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

QString Bestiary::getExpectedVersion() const
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

MonsterClass* Bestiary::getMonsterClass(const QString& name) const
{
    return _bestiaryMap.value(name, NULL);
}

MonsterClass* Bestiary::getFirstMonsterClass() const
{
    if(_bestiaryMap.count() == 0)
        return NULL;

    return _bestiaryMap.first();
}

MonsterClass* Bestiary::getLastMonsterClass() const
{
    if(_bestiaryMap.count() == 0)
        return NULL;

    return _bestiaryMap.last();
}

MonsterClass* Bestiary::getNextMonsterClass(MonsterClass* monsterClass) const
{
    if(!monsterClass)
        return NULL;

    BestiaryMap::const_iterator i = _bestiaryMap.find(monsterClass->getName());
    if(i == _bestiaryMap.constEnd())
        return NULL;

    ++i;

    if(i == _bestiaryMap.constEnd())
        return NULL;

    return i.value();
}

MonsterClass* Bestiary::getPreviousMonsterClass(MonsterClass* monsterClass) const
{
    if(!monsterClass)
        return NULL;

    BestiaryMap::const_iterator i = _bestiaryMap.find(monsterClass->getName());
    if(i == _bestiaryMap.constBegin())
        return NULL;

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
        return NULL;

    return new Monster(getMonsterClass(name));
}

Monster* Bestiary::createMonster(const QDomElement& element) const
{
    QString monsterName = element.attribute(QString("monsterClass"));

    if(!_bestiaryMap.contains(monsterName))
        return NULL;

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
