#include "bestiary.h"
#include "monsterclass.h"
#include "monster.h"
#include "dmconstants.h"
#include "dmversion.h"
#include <QDomDocument>
#include <QDomElement>
#include <QDir>
#include <QPixmap>
#include <QMessageBox>
#include <QPushButton>
#include <QDebug>

Bestiary* Bestiary::_instance = nullptr;

Bestiary::Bestiary(QObject *parent) :
    QObject(parent),
    _bestiaryMap(),
    _bestiaryDirectory(),
    _majorVersion(0),
    _minorVersion(0),
    _licenseText(),
    _batchProcessing(false),
    _batchAcknowledge(false)
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
    _instance = nullptr;
}

int Bestiary::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport) const
{
    int monsterCount = 0;

    qDebug() << "[Bestiary] Saving bestiary...";
    QDomElement bestiaryElement = doc.createElement("bestiary");
    bestiaryElement.setAttribute("majorversion", DMHelper::BESTIARY_MAJOR_VERSION);
    bestiaryElement.setAttribute("minorversion", DMHelper::BESTIARY_MINOR_VERSION);

    qDebug() << "[Bestiary]    Storing " << _bestiaryMap.count() << " creatures.";
    BestiaryMap::const_iterator i = _bestiaryMap.constBegin();
    while (i != _bestiaryMap.constEnd())
    {
        MonsterClass* monsterClass = i.value();
        if(monsterClass)
        {
            QDomElement monsterElement = doc.createElement("element");
            monsterClass->outputXML(doc, monsterElement, targetDirectory, isExport);
            bestiaryElement.appendChild(monsterElement);
        }

        ++monsterCount;
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
    qDebug() << "[Bestiary] Saving bestiary completed: " << monsterCount << " monsters written to XML";
    return monsterCount;
}

void Bestiary::inputXML(const QDomElement &element, bool isImport)
{
    qDebug() << "[Bestiary] Loading bestiary...";

    QDomElement bestiaryElement = element.firstChildElement( QString("bestiary") );
    if(bestiaryElement.isNull())
    {
        qDebug() << "[Bestiary]    ERROR: invalid bestiary file, unable to find base element";
        return;
    }

    _majorVersion = bestiaryElement.attribute("majorversion",QString::number(0)).toInt();
    _minorVersion = bestiaryElement.attribute("minorversion",QString::number(0)).toInt();
    qDebug() << "[Bestiary]    Bestiary version: " << getVersion();
    if(!isVersionCompatible())
    {
        qDebug() << "[Bestiary]    ERROR: Bestiary version is not compatible with expected version: " << getExpectedVersion();
        return;
    }

    if (!isVersionIdentical())
        qDebug() << "[Bestiary]    WARNING: Bestiary version is not the same as expected version: " << getExpectedVersion();

    if(isImport)
    {
        int importCount = 0;
        QMessageBox::StandardButton challengeResult = QMessageBox::NoButton;
        QDomElement monsterElement = bestiaryElement.firstChildElement( QString("element") );
        while( !monsterElement.isNull() )
        {
            bool importOK = true;
            QString monsterName = monsterElement.firstChildElement(QString("name")).text();
            if(Bestiary::Instance()->exists(monsterName))
            {
                if((challengeResult != QMessageBox::YesToAll) && (challengeResult != QMessageBox::NoToAll))
                {
                    challengeResult = QMessageBox::question(nullptr,
                                                            QString("Import Monster Conflict"),
                                                            QString("The monster '") + monsterName + QString("' already exists in the Bestiary. Would you like to overwrite the existing entry?"),
                                                            QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );
                    if(challengeResult == QMessageBox::Cancel)
                    {
                        qDebug() << "[Bestiary] Import monsters cancelled";
                        return;
                    }
                }

                importOK = ((challengeResult == QMessageBox::Yes) || (challengeResult == QMessageBox::YesToAll));
            }

            if(importOK)
            {
                MonsterClass* monster = new MonsterClass(monsterElement, isImport);
                if(insertMonsterClass(monster))
                    ++importCount;
            }

            monsterElement = monsterElement.nextSiblingElement( QString("element") );
        }

        qDebug() << "[Bestiary] Importing bestiary completed. " << importCount << " creatures imported.";
    }
    else
    {
        if(_bestiaryMap.count() > 0)
        {
            qDebug() << "[Bestiary]    Unloading previous bestiary";
            qDeleteAll(_bestiaryMap);
            _bestiaryMap.clear();
        }

        QDomElement monsterElement = bestiaryElement.firstChildElement( QString("element") );
        while( !monsterElement.isNull() )
        {
            MonsterClass* monster = new MonsterClass(monsterElement, isImport);
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
    }

    emit changed();
}

QString Bestiary::getVersion() const
{
    return QString::number(_majorVersion) + "." + QString::number(_minorVersion);
}

int Bestiary::getMajorVersion() const
{
    return _majorVersion;
}

int Bestiary::getMinorVersion() const
{
    return _minorVersion;
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

MonsterClass* Bestiary::getMonsterClass(const QString& name)
{
    if(!_bestiaryMap.contains(name))
        showMonsterClassWarning(name);

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
    if(!monsterClass)
        return false;

    if(_bestiaryMap.contains(monsterClass->getName()))
        return false;

    _bestiaryMap.insert(monsterClass->getName(), monsterClass);
    emit changed();
    return true;
}

void Bestiary::removeMonsterClass(MonsterClass* monsterClass)
{
    if(!monsterClass)
        return;

    if(!_bestiaryMap.contains(monsterClass->getName()))
        return;

    _bestiaryMap.remove(monsterClass->getName());
    delete monsterClass;
    emit changed();
}

void Bestiary::renameMonster(MonsterClass* monsterClass, const QString& newName)
{
    if(!monsterClass)
        return;

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

Monster* Bestiary::createMonster(const QString& name)
{
    if(!_bestiaryMap.contains(name))
    {
        showMonsterClassWarning(name);
        return nullptr;
    }

    return new Monster(getMonsterClass(name));
}

Monster* Bestiary::createMonster(const QDomElement& element, bool isImport)
{
    QString monsterName = element.attribute(QString("monsterClass"));

    if(!_bestiaryMap.contains(monsterName))
    {
        showMonsterClassWarning(monsterName);
        return nullptr;
    }

    //return new Monster(getMonsterClass(monsterName), element, isImport);
    Monster* newMonster = new Monster(getMonsterClass(monsterName));
    newMonster->inputXML(element, isImport);
    return newMonster;
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

void Bestiary::startBatchProcessing()
{
    _batchProcessing = true;
    _batchAcknowledge = false;
}

void Bestiary::finishBatchProcessing()
{
    _batchProcessing = false;
    _batchAcknowledge = false;
}

void Bestiary::showMonsterClassWarning(const QString& monsterClass)
{
    qDebug() << "[Bestiary] ERROR: Requested monster class not found: " << monsterClass;

    if(_batchProcessing)
    {
        if(!_batchAcknowledge)
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle(QString("Unknown monster"));
            msgBox.setText(QString("WARNING: The monster """) + monsterClass + QString(""" was not found in the current bestiary! If you save the current campaign, all references to this monster will be lost!"));
            QPushButton* pButtonOK = msgBox.addButton(QString("OK"), QMessageBox::YesRole);
            QPushButton* pButtonOKAll = msgBox.addButton(QString("OK to All"), QMessageBox::NoRole);
            msgBox.setDefaultButton(pButtonOK);
            msgBox.setEscapeButton(pButtonOK);
            msgBox.exec();
            if (msgBox.clickedButton()==pButtonOKAll)
            {
                _batchAcknowledge = true;
            }
        }
    }
    else
    {
        QMessageBox::critical(nullptr, QString("Unknown monster"), QString("WARNING: The monster """) + monsterClass + QString(""" was not found in the current bestiary! If you save the current campaign, all references to this monster will be lost!"));
    }
}
