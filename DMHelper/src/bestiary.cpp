#include "bestiary.h"
#include "monsterclass.h"
#include "monsterclassv2.h"
#include "monsterclassv2converter.h"
#include "monster.h"
#include "templatefactory.h"
#include "dmversion.h"
#include <QDomDocument>
#include <QDomElement>
#include <QDir>
#include <QPixmap>
#include <QMessageBox>
#include <QFileDialog>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDebug>

Bestiary* Bestiary::_instance = nullptr;

Bestiary::Bestiary(QObject *parent) :
    QObject(parent),
    _bestiaryMap(),
    _bestiaryDirectory(),
    _bestiaryFile(),
    _majorVersion(0),
    _minorVersion(0),
    _licenseText(),
    _dirty(false),
    _batchProcessing(false),
    _batchAcknowledge(false),
    _batchChanges(false),
    _changesMade(false)
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

QStringList Bestiary::search(const QString& searchString)
{
    QStringList results;
    if(searchString.isEmpty())
        return results;

    // Iterate directly over the keys in the map
    for (auto it = _bestiaryMap.constBegin(); it != _bestiaryMap.constEnd(); ++it)
    {
        const QString& key = it.key();
        if(key.contains(searchString, Qt::CaseInsensitive))
        {
            results << key << QString();
        }
        else
        {
            MonsterClassv2* monsterClass = it.value();
            if(monsterClass)
            {
                QString matchString;
                if(monsterClass->matchSearchString(searchString, matchString))
                    results << key << matchString;
            }
        }
    }

    return results;
}

bool Bestiary::writeBestiary(const QString& targetFilename)
{
    if(targetFilename.isEmpty())
    {
        qDebug() << "[Bestiary] Bestiary target filename empty, no file will be written";
        return false;
    }

    if(count() <= 0)
    {
        qDebug() << "[Bestiary] Bestiary is empty, no file will be written";
        return false;
    }

    qDebug() << "[Bestiary] Writing Bestiary to " << targetFilename;

    QDomDocument doc("DMHelperBestiaryXML");
    QDomElement root = doc.createElement("root");
    doc.appendChild(root);

    QFileInfo fileInfo(targetFilename);
    QDir targetDirectory(fileInfo.absoluteDir());
    if(outputXML(doc, root, targetDirectory, false) <= 0)
    {
        qDebug() << "[Bestiary] Bestiary output did not find any monsters. Aborting writing to file";
        return false;
    }

    QString xmlString = doc.toString();

    QFile file(targetFilename);
    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "[Bestiary] Unable to open Bestiary file for writing: " << targetFilename;
        qDebug() << "       Error " << file.error() << ": " << file.errorString();
        QFileInfo info(file);
        qDebug() << "       Full filename: " << info.absoluteFilePath();
        return false;
    }

    QTextStream ts(&file);
    ts.setEncoding(QStringConverter::Utf8);
    ts << xmlString;

    file.close();
    setDirty(false);

    qDebug() << "[MainWindow] Bestiary file writing complete: " << targetFilename;

    return true;
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
    while(i != _bestiaryMap.constEnd())
    {
        MonsterClassv2* monsterClass = i.value();
        if(monsterClass)
        {
            QDomElement monsterElement = doc.createElement("monster");
            monsterClass->outputXML(doc, monsterElement, targetDirectory, isExport);
            bestiaryElement.appendChild(monsterElement);
        }

        ++monsterCount;
        ++i;
    }

    QDomElement licenseElement = doc.createElement(QString("license"));
    for(int license = 0; license < _licenseText.count(); ++license)
    {
        QDomElement licenseTextElement = doc.createElement(QString("element"));
        licenseTextElement.appendChild(doc.createTextNode(_licenseText.at(license)));
        licenseElement.appendChild(licenseTextElement);
    }
    bestiaryElement.appendChild(licenseElement);

    parent.appendChild(bestiaryElement);
    qDebug() << "[Bestiary] Saving bestiary completed: " << monsterCount << " monsters written to XML";

    return monsterCount;
}

void Bestiary::inputXML(const QDomElement &element, const QString& importFile)
{
    qDebug() << "[Bestiary] Bestiary provided for input";

    QDomElement bestiaryElement = element.firstChildElement(QString("bestiary"));
    if(bestiaryElement.isNull())
    {
        qDebug() << "[Bestiary]    ERROR: invalid bestiary file, unable to find base bestiary element";
        QMessageBox::critical(nullptr, QString("Invalid Bestiary File"), QString("The new bestiary file is invalid - it is missing a base bestiary element and can't be loaded."));
        return;
    }

    _majorVersion = bestiaryElement.attribute("majorversion", QString::number(0)).toInt();
    _minorVersion = bestiaryElement.attribute("minorversion", QString::number(0)).toInt();
    qDebug() << "[Bestiary]    Bestiary version: " << getVersion();
    if(!isVersionCompatible())
    {
        qDebug() << "[Bestiary]    ERROR: Bestiary version is not compatible with expected version: " << getExpectedVersion();
        QMessageBox::critical(nullptr, QString("Incompatible Bestiary File"), QString("The new bestiary file is incompatible with this version of DMHelper and can't be loaded."));
        return;
    }

    if(!isVersionIdentical())
    {
        qDebug() << "[Bestiary]    WARNING: Bestiary version is not the same as expected version: " << getExpectedVersion() << ", trying to convert to the new format.";
        if(importFile.isEmpty())
        {
            loadAndConvertBestiary(bestiaryElement);
        }
        else
        {
            qDebug() << "[Bestiary]    ERROR: Importing an old bestiary version is not supported. Please convert the file to the new format before importing.";
            QMessageBox::critical(nullptr, QString("Importing old version"), QString("Importing an old bestiary version is not supported. Please convert the file to the new format before importing. You can do this by setting the file to import as the primary bestiary file in DMHelper and then saving it again."));
            return;
        }
    }
    else
    {
        if(importFile.isEmpty())
            loadBestiary(bestiaryElement);
        else
            importBestiary(bestiaryElement, importFile);
    }

    registerChange();
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

bool Bestiary::isDirty()
{
    return _dirty;
}

MonsterClassv2* Bestiary::getMonsterClass(const QString& name)
{
    if(name.isEmpty())
        return nullptr;

    if(!_bestiaryMap.contains(name))
        showMonsterClassWarning(name);

    return _bestiaryMap.value(name, nullptr);
}

MonsterClassv2* Bestiary::getFirstMonsterClass() const
{
    if(_bestiaryMap.count() == 0)
        return nullptr;

    return _bestiaryMap.first();
}

MonsterClassv2* Bestiary::getLastMonsterClass() const
{
    if(_bestiaryMap.count() == 0)
        return nullptr;

    return _bestiaryMap.last();
}

MonsterClassv2* Bestiary::getNextMonsterClass(MonsterClassv2* monsterClass) const
{
    if(!monsterClass)
        return nullptr;

    BestiaryMap::const_iterator i = _bestiaryMap.find(monsterClass->getStringValue("name"));
    if(i == _bestiaryMap.constEnd())
        return nullptr;

    ++i;

    if(i == _bestiaryMap.constEnd())
        return nullptr;

    return i.value();
}

MonsterClassv2* Bestiary::getPreviousMonsterClass(MonsterClassv2* monsterClass) const
{
    if(!monsterClass)
        return nullptr;

    BestiaryMap::const_iterator i = _bestiaryMap.find(monsterClass->getStringValue("name"));
    if(i == _bestiaryMap.constBegin())
        return nullptr;

    --i;
    return i.value();
}

bool Bestiary::insertMonsterClass(MonsterClassv2* monsterClass)
{
    if(!monsterClass)
        return false;

    if(_bestiaryMap.contains(monsterClass->getStringValue("name")))
    {
        qDebug() << "[Bestiary] Attempted to insert a monster class that already exists in the bestiary: " << monsterClass->getStringValue("name");
        return false;
    }

    _bestiaryMap.insert(monsterClass->getStringValue("name"), monsterClass);
    connect(monsterClass, &MonsterClassv2::dirty, this, &Bestiary::registerDirty);
    registerChange();
    setDirty();
    return true;
}

void Bestiary::removeMonsterClass(MonsterClassv2* monsterClass)
{
    if(!monsterClass)
        return;

    if(!_bestiaryMap.contains(monsterClass->getStringValue("name")))
        return;

    disconnect(monsterClass, &MonsterClassv2::dirty, this, &Bestiary::registerDirty);
    _bestiaryMap.remove(monsterClass->getStringValue("name"));
    delete monsterClass;
    setDirty();
    registerChange();
}

void Bestiary::renameMonster(MonsterClassv2* monsterClass, const QString& newName)
{
    if(!monsterClass)
        return;

    if(!_bestiaryMap.contains(monsterClass->getStringValue("name")))
        return;

    _bestiaryMap.remove(monsterClass->getStringValue("name"));
    monsterClass->setStringValue("name", newName);
    insertMonsterClass(monsterClass);
    setDirty();
}

void Bestiary::setDirectory(const QDir& directory)
{
    _bestiaryDirectory = directory;
    setDirty();
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

    Monster* newMonster = new Monster(getMonsterClass(monsterName));
    newMonster->inputXML(element, isImport);
    return newMonster;
}

QString Bestiary::findMonsterImage(const QString& monsterName, const QString& iconFile)
{
    if((!iconFile.isEmpty()) && (_bestiaryDirectory.exists(iconFile)))
        return iconFile;

    QString resultName = findMonsterImage(_bestiaryDirectory, monsterName);
    if(resultName.isEmpty())
        qDebug() << "[Bestiary] Not able to find monster image for " << monsterName << " with icon file " << iconFile;

    return resultName;
}

QString Bestiary::findMonsterImage(const QDir& sourceDir, const QString& monsterName)
{
    QString fileName = QString("./") + monsterName + QString(".png");
    if(!sourceDir.exists(fileName))
    {
        fileName = QString("./") + monsterName + QString(".jpg");
        if(!sourceDir.exists(fileName))
        {
            fileName = QString("./Images/") + monsterName + QString(".png");
            if(!sourceDir.exists(fileName))
            {
                fileName = QString("./Images/") + monsterName + QString(".jpg");
                if(!sourceDir.exists(fileName))
                    fileName = QString();
            }
        }
    }

    return fileName;
}

QStringList Bestiary::findMonsterImages(const QString& monsterName)
{
    QStringList monsterNameFilter;
    monsterNameFilter << (monsterName + QString("*.png")) << (monsterName + QString("*.jpg"));
    QStringList imageNameFilter;
    imageNameFilter << QString("*.png") << QString("*.jpg");

    QStringList entries = findSpecificImages(_bestiaryDirectory, monsterNameFilter, monsterName);
    entries << findSpecificImages(QDir(_bestiaryDirectory.absolutePath() + QString("/") + monsterName + QString("/")), imageNameFilter);
    entries << findSpecificImages(QDir(_bestiaryDirectory.absolutePath() + QString("/Images/")), monsterNameFilter, monsterName);
    entries << findSpecificImages(QDir(_bestiaryDirectory.absolutePath() + QString("/Images/") + monsterName + QString("/")), imageNameFilter);

    return entries;
}

QStringList Bestiary::findSpecificImages(const QDir& sourceDir, const QStringList& filterList, const QString& filterName)
{
    QStringList result;

    QFileInfoList entries = sourceDir.entryInfoList(filterList);
    for(int i = 0; i < entries.count(); ++i)
    {
        bool accept = false;
        if(filterName.isEmpty())
        {
            accept = true;
        }
        else
        {
            QRegularExpression re(QString("([a-zA-Z\\s]*)"));
            QRegularExpressionMatch reMatch = re.match(entries.at(i).baseName());
            QString matchName = reMatch.captured(1);
            if((!reMatch.hasMatch()) || (matchName.isEmpty()) || (matchName == filterName) || (!exists(matchName)))
                accept = true;
        }

        if(accept)
            result << _bestiaryDirectory.relativeFilePath(entries.at(i).absoluteFilePath());
    }

    return result;
}

bool Bestiary::readBestiary(const QString& targetFilename)
{
    if(targetFilename.isEmpty())
    {
        qDebug() << "[Bestiary] ERROR! No known bestiary found, unable to load bestiary: " << targetFilename;
        QMessageBox::critical(nullptr, QString("Invalid bestiary file"), QString("The bestiary file is invalid: ") + targetFilename);
        return false;
    }

    QString absoluteTargetFilename = TemplateFactory::getAbsoluteTemplateFile(targetFilename);
    if(absoluteTargetFilename.isEmpty())
    {
        qDebug() << "[Bestiary] ERROR! Bestiary not found based on relative file path: " << targetFilename;
        QMessageBox::critical(nullptr, QString("Invalid bestiary file"), QString("The bestiary file could not be found: ") + targetFilename);
        return false;
    }

    if(absoluteTargetFilename == _bestiaryFile)
    {
        qDebug() << "[Bestiary] Requested to reload bestiary file, no action taken: " << targetFilename;
        return false;
    }

    if(isDirty())
    {
        qDebug() << "[Bestiary] Existing bestiary is unsaved!";
        QMessageBox::StandardButton result = QMessageBox::critical(nullptr,
                                                                   QString("Unsaved Bestiary"),
                                                                   QString("The current bestiary has not been saved. Would you like to save it before loading a new bestiary? If you don't. you may lose monster data!"),
                                                                   QMessageBox::Yes | QMessageBox::No);
        if(result == QMessageBox::Yes)
        {
            QString bestiarySaveFileName = QFileDialog::getSaveFileName(nullptr, QString("Save Bestiary"), QString(), QString("XML files (*.xml)"));
            if(!bestiarySaveFileName.isEmpty())
            {
                if(writeBestiary(bestiarySaveFileName))
                    qDebug() << "[Bestiary] Bestiary file writing complete: " << bestiarySaveFileName;
                else
                    qDebug() << "[Bestiary] ERROR: Bestiary file writing failed: " << bestiarySaveFileName;
            }
        }
    }

    qDebug() << "[Bestiary] Reading bestiary: " << absoluteTargetFilename;
#ifdef QT_DEBUG
    QFileInfo bestiaryInfo(absoluteTargetFilename);
    qDebug() << "[Bestiary] Debug full path:" << bestiaryInfo.absoluteFilePath() << ", exists: " << bestiaryInfo.exists() << ", is file: " << bestiaryInfo.isFile();
#endif

    QDomDocument doc("DMHelperBestiaryXML");
    QFile file(absoluteTargetFilename);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[Bestiary] Reading bestiary file open failed: " << file.error() << ", " << file.errorString();
        QMessageBox::critical(nullptr, QString("Bestiary file open failed"), QString("Unable to open the bestiary file: ") + absoluteTargetFilename + QString(": ") + file.errorString());
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString errMsg;
    int errRow;
    int errColumn;
    bool contentResult = doc.setContent(in.readAll(), &errMsg, &errRow, &errColumn);

    file.close();

    if(contentResult == false)
    {
        qDebug() << "[Bestiary] Error reading bestiary XML content. The XML is probably not valid.";
        qDebug() << errMsg << errRow << errColumn;
        QMessageBox::critical(nullptr, QString("Bestiary file invalid"), QString("Unable to read the bestiary file: ") + absoluteTargetFilename + QString(", the XML is invalid"));
        return false;
    }

    QDomElement root = doc.documentElement();
    if((root.isNull()) || (root.tagName() != "root"))
    {
        qDebug() << "[Bestiary] Bestiary file missing root item";
        QMessageBox::critical(nullptr, QString("Bestiary file invalid"), QString("Unable to read the bestiary file: ") + absoluteTargetFilename + QString(", the XML does not have the expected root item."));
        return false;
    }

    startBatchChanges();

    // Remove the existing bestiary before resetting the directory and filename
    if(_bestiaryMap.count() > 0)
    {
        qDebug() << "[Bestiary]    Unloading previous bestiary";
        qDeleteAll(_bestiaryMap);
        _bestiaryMap.clear();
    }

    QFileInfo fileInfo(absoluteTargetFilename);
    setDirectory(fileInfo.absoluteDir());
    _bestiaryFile = absoluteTargetFilename;
    inputXML(root);

    finishBatchChanges();

    if(isVersionCompatible())
        emit bestiaryLoaded(_bestiaryFile, !isVersionIdentical());

    return true;
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

void Bestiary::registerDirty()
{
    setDirty();
}

void Bestiary::setDirty(bool dirty)
{
    _dirty = dirty;
}

void Bestiary::startBatchChanges()
{
    _batchChanges = true;
    _changesMade = false;
}

void Bestiary::finishBatchChanges()
{
    if(_batchChanges)
    {
        _batchChanges = false;
        if(_changesMade)
            emit changed();
    }
}

void Bestiary::registerChange()
{
    if(_batchChanges)
        _changesMade = true;
    else
        emit changed();
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
                _batchAcknowledge = true;
        }
    }
    else
    {
        QMessageBox::critical(nullptr, QString("Unknown monster"), QString("WARNING: The monster """) + monsterClass + QString(""" was not found in the current bestiary! If you save the current campaign, all references to this monster will be lost!"));
    }
}

void Bestiary::loadAndConvertBestiary(const QDomElement& bestiaryElement)
{
    qDebug() << "[Bestiary] Loading bestiary...";

    QDomElement monsterElement = bestiaryElement.firstChildElement(QString("element"));
    while(!monsterElement.isNull())
    {
        MonsterClassv2* monster = new MonsterClassv2Converter(monsterElement);
        insertMonsterClass(monster);
        monsterElement = monsterElement.nextSiblingElement(QString("element"));
    }

    QDomElement licenseElement = bestiaryElement.firstChildElement(QString("license"));
    if(licenseElement.isNull())
        qDebug() << "[Bestiary] ERROR: not able to find the license text in the bestiary!";

    QDomElement licenseText = licenseElement.firstChildElement(QString("element"));
    while(!licenseText.isNull())
    {
        if(!_licenseText.contains(licenseText.text()))
            _licenseText.append(licenseText.text());
        licenseText = licenseText.nextSiblingElement(QString("element"));
    }

    qDebug() << "[Bestiary] Loading bestiary completed. " << _bestiaryMap.count() << " creatures loaded.";

    setDirty(false);
}

void Bestiary::loadBestiary(const QDomElement& bestiaryElement)
{
    qDebug() << "[Bestiary] Loading bestiary...";

    QDomElement monsterElement = bestiaryElement.firstChildElement(QString("monster"));
    while(!monsterElement.isNull())
    {
        MonsterClassv2* monster = new MonsterClassv2(monsterElement, false);
        insertMonsterClass(monster);
        monsterElement = monsterElement.nextSiblingElement(QString("monster"));
    }

    QDomElement licenseElement = bestiaryElement.firstChildElement(QString("license"));
    if(licenseElement.isNull())
        qDebug() << "[Bestiary] ERROR: not able to find the license text in the bestiary!";

    QDomElement licenseText = licenseElement.firstChildElement(QString("element"));
    while(!licenseText.isNull())
    {
        _licenseText.append(licenseText.text());
        licenseText = licenseText.nextSiblingElement(QString("element"));
    }

    qDebug() << "[Bestiary] Loading bestiary completed. " << _bestiaryMap.count() << " creatures loaded.";

    setDirty(false);
}

void Bestiary::importBestiary(const QDomElement& bestiaryElement, const QString& importFile)
{
    qDebug() << "[Bestiary] Importing bestiary...";

    int importCount = 0;
    QMessageBox::StandardButton challengeResult = QMessageBox::NoButton;
    QDomElement monsterElement = bestiaryElement.firstChildElement(QString("monster"));
    while(!monsterElement.isNull())
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
                                                        QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel);
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
            importMonsterImage(monsterElement, importFile);
            MonsterClassv2* monster = new MonsterClassv2(monsterElement, true);
            if(insertMonsterClass(monster))
            {
                ++importCount;
            }
        }

        monsterElement = monsterElement.nextSiblingElement(QString("element"));
    }

    qDebug() << "[Bestiary] Importing bestiary completed. " << importCount << " creatures imported.";
    setDirty();
}

void Bestiary::importMonsterImage(const QDomElement& monsterElement, const QString& importFile)
{
    QString monsterIcon = monsterElement.attribute(("icon"));
    if(!monsterIcon.isEmpty())
    {
        QFileInfo embeddedIconInfo(monsterIcon);

        // Files in absolute paths can't be copied
        if(embeddedIconInfo.isAbsolute())
            return;

        QString relativePath = embeddedIconInfo.path();
        if(!_bestiaryDirectory.mkpath(relativePath))
            return;

        //
        //if(!embeddedIconInfo.exists())
        //    return;
        QFileInfo importFileInfo(importFile);
        QDir importFileDir(importFileInfo.absoluteDir());
        //QString relativePath = importFileDir.relativeFilePath(monsterIcon);

        //QString absolutePath = embeddedIconInfo.absoluteFilePath();
        QString absolutePath = importFileDir.absoluteFilePath(monsterIcon);
        QFile currentFile(absolutePath);

        QString targetFile = _bestiaryDirectory.absoluteFilePath(monsterIcon);
        bool result = currentFile.copy(targetFile);

        qDebug() << "[Bestiary] Copied " << currentFile.fileName() << " to " << targetFile << ", result: " << result;
    }
    else
    {
        QFileInfo importFileInfo(importFile);
        QDir importFileDir(importFileInfo.absoluteDir());
        QString sourcePath = findMonsterImage(importFileDir, monsterElement.firstChildElement(QString("name")).text());
        if(sourcePath.isEmpty())
            return;

        QFileInfo embeddedIconInfo(sourcePath);

        QString relativePath = embeddedIconInfo.path();
        if(!_bestiaryDirectory.mkpath(relativePath))
            return;

        QString absolutePath = importFileDir.absoluteFilePath(sourcePath);
        QFile currentFile(absolutePath);
        QString targetFile = _bestiaryDirectory.absoluteFilePath(sourcePath);
        bool result = currentFile.copy(targetFile);

        qDebug() << "[Bestiary] Copied " << currentFile.fileName() << " to " << targetFile << ", result: " << result;
    }
}

