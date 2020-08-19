#include "spellbook.h"
#include "spell.h"
#include "dmversion.h"
#include <QMessageBox>
#include <QPushButton>
#include <QDebug>

/*
 *         <spell
                <name
                <level
                <school
                <time
                <range
                <components
                <duration
                <classes
                <text
                <text /
                <roll
        </spell
                <ritual
*/

Spellbook* Spellbook::_instance = nullptr;

Spellbook::Spellbook(QObject *parent) :
    QObject(parent),
    _spellbookMap(),
    _spellbookDirectory(),
    _majorVersion(0),
    _minorVersion(0),
    _licenseText(),
    _batchProcessing(false),
    _batchAcknowledge(false)
{
}

Spellbook::~Spellbook()
{
    qDeleteAll(_spellbookMap);
}

Spellbook* Spellbook::Instance()
{
    return _instance;
}

void Spellbook::Initialize()
{
    if(_instance)
        return;

    qDebug() << "[Spellbook] Initializing Spellbook";
    _instance = new Spellbook();
}

void Spellbook::Shutdown()
{
    delete _instance;
    _instance = nullptr;
}

int Spellbook::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport) const
{
    int spellCount = 0;

    qDebug() << "[Spellbook] Saving spellbook...";
    QDomElement spellbookElement = doc.createElement("spellbook");
    spellbookElement.setAttribute("majorversion", DMHelper::SPELLBOOK_MAJOR_VERSION);
    spellbookElement.setAttribute("minorversion", DMHelper::SPELLBOOK_MINOR_VERSION);

    qDebug() << "[Spellbook]    Storing " << _spellbookMap.count() << " spells.";
    SpellbookMap::const_iterator i = _spellbookMap.constBegin();
    while (i != _spellbookMap.constEnd())
    {
        Spell* spell = i.value();
        if(spell)
        {
            QDomElement spellElement = doc.createElement("spell");
            spell->outputXML(doc, spellElement, targetDirectory, isExport);
            spellbookElement.appendChild(spellElement);
        }

        ++spellCount;
        ++i;
    }

    QDomElement licenseElement = doc.createElement(QString("license"));
    for(QString licenseText : _licenseText)
    {
        QDomElement licenseTextElement = doc.createElement(QString("element"));
        licenseTextElement.appendChild(doc.createTextNode(licenseText));
        licenseElement.appendChild(licenseTextElement);
    }
    spellbookElement.appendChild(licenseElement);

    parent.appendChild(spellbookElement);
    qDebug() << "[Spellbook] Saving spellbook completed: " << spellCount << " spells written to XML";
    return spellCount;
}

void Spellbook::inputXML(const QDomElement &element, bool isImport)
{
    qDebug() << "[Spellbook] Loading spellbook...";

    QDomElement spellbookElement = element.firstChildElement(QString("spellbook"));
    if(spellbookElement.isNull())
    {
        qDebug() << "[Spellbook]    ERROR: invalid spellbook file, unable to find base element";
        return;
    }

    _majorVersion = spellbookElement.attribute("majorversion", QString::number(0)).toInt();
    _minorVersion = spellbookElement.attribute("minorversion", QString::number(0)).toInt();
    qDebug() << "[Spellbook]    Spellbook version: " << getVersion();
    if(!isVersionCompatible())
    {
        qDebug() << "[Spellbook]    ERROR: Spellbook version is not compatible with expected version: " << getExpectedVersion();
        return;
    }

    if (!isVersionIdentical())
        qDebug() << "[Spellbook]    WARNING: Spellbook version is not the same as expected version: " << getExpectedVersion();

    if(isImport)
    {
        // TODO: add spell import
        int importCount = 0;
        QMessageBox::StandardButton challengeResult = QMessageBox::NoButton;
        QDomElement spellElement = spellbookElement.firstChildElement(QString("spell"));
        while(!spellElement.isNull())
        {
            bool importOK = true;
            QString spellName = spellElement.firstChildElement(QString("name")).text();
            if(Spellbook::Instance()->exists(spellName))
            {
                if((challengeResult != QMessageBox::YesToAll) && (challengeResult != QMessageBox::NoToAll))
                {
                    challengeResult = QMessageBox::question(nullptr,
                                                            QString("Import Spell Conflict"),
                                                            QString("The spell '") + spellName + QString("' already exists in the Spell. Would you like to overwrite the existing entry?"),
                                                            QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll | QMessageBox::Cancel );
                    if(challengeResult == QMessageBox::Cancel)
                    {
                        qDebug() << "[Spellbook] Import spells cancelled";
                        return;
                    }
                }

                importOK = ((challengeResult == QMessageBox::Yes) || (challengeResult == QMessageBox::YesToAll));
            }

            if(importOK)
            {
                Spell* spell = new Spell(spellElement, isImport);
                if(insertSpell(spell))
                    ++importCount;
            }

            spellElement = spellElement.nextSiblingElement(QString("spell"));
        }

        qDebug() << "[Spellbook] Importing spellbook completed. " << importCount << " spells imported.";
    }
    else
    {
        if(_spellbookMap.count() > 0)
        {
            qDebug() << "[Spellbook]    Unloading previous spellbook";
            qDeleteAll(_spellbookMap);
            _spellbookMap.clear();
        }

        QDomElement spellElement = spellbookElement.firstChildElement(QString("spell"));
        while(!spellElement.isNull())
        {
            Spell* spell = new Spell(spellElement, isImport);
            insertSpell(spell);
            spellElement = spellElement.nextSiblingElement(QString("spell"));
        }

        QDomElement licenseElement = spellbookElement.firstChildElement(QString("license"));
        if(licenseElement.isNull())
            qDebug() << "[Bestiary] ERROR: not able to find the license text in the spellbook!";

        QDomElement licenseText = licenseElement.firstChildElement(QString("element"));
        while(!licenseText.isNull())
        {
            _licenseText.append(licenseText.text());
            licenseText = licenseText.nextSiblingElement(QString("element"));
        }

        qDebug() << "[Spellbook] Loading spellbook completed. " << _spellbookMap.count() << " spells loaded.";
    }

    emit changed();
}

QString Spellbook::getVersion() const
{
    return QString::number(_majorVersion) + "." + QString::number(_minorVersion);
}

int Spellbook::getMajorVersion() const
{
    return _majorVersion;
}

int Spellbook::getMinorVersion() const
{
    return _minorVersion;
}

bool Spellbook::isVersionCompatible() const
{
    return (_majorVersion == DMHelper::SPELLBOOK_MAJOR_VERSION);
}

bool Spellbook::isVersionIdentical() const
{
    return ((_majorVersion == DMHelper::SPELLBOOK_MAJOR_VERSION) && (_minorVersion == DMHelper::SPELLBOOK_MINOR_VERSION));
}

QString Spellbook::getExpectedVersion()
{
    return QString::number(DMHelper::SPELLBOOK_MAJOR_VERSION) + "." + QString::number(DMHelper::SPELLBOOK_MINOR_VERSION);
}

bool Spellbook::exists(const QString& name) const
{
    return _spellbookMap.contains(name);
}

int Spellbook::count() const
{
    return _spellbookMap.count();
}

QList<QString> Spellbook::getSpellList() const
{
    return _spellbookMap.keys();
}

QStringList Spellbook::getLicenseText() const
{
    return _licenseText;
}

Spell* Spellbook::getSpell(const QString& name)
{
    if(!_spellbookMap.contains(name))
        showSpellWarning(name);

    return _spellbookMap.value(name, nullptr);
}

Spell* Spellbook::getFirstSpell() const
{
    if(_spellbookMap.count() == 0)
        return nullptr;

    return _spellbookMap.first();
}

Spell* Spellbook::getLastSpell() const
{
    if(_spellbookMap.count() == 0)
        return nullptr;

    return _spellbookMap.last();
}

Spell* Spellbook::getNextSpell(Spell* spell) const
{
    if(!spell)
        return nullptr;

    SpellbookMap::const_iterator i = _spellbookMap.find(spell->getName());
    if(i == _spellbookMap.constEnd())
        return nullptr;

    ++i;

    if(i == _spellbookMap.constEnd())
        return nullptr;

    return i.value();
}

Spell* Spellbook::getPreviousSpell(Spell* spell) const
{
    if(!spell)
        return nullptr;

    SpellbookMap::const_iterator i = _spellbookMap.find(spell->getName());
    if(i == _spellbookMap.constBegin())
        return nullptr;

    --i;
    return i.value();
}

bool Spellbook::insertSpell(Spell* spell)
{
    if(!spell)
        return false;

    if(_spellbookMap.contains(spell->getName()))
        return false;

    _spellbookMap.insert(spell->getName(), spell);
    emit changed();
    return true;
}

void Spellbook::removeSpell(Spell* spell)
{
    if(!spell)
        return;

    if(!_spellbookMap.contains(spell->getName()))
        return;

    _spellbookMap.remove(spell->getName());
    delete spell;
    emit changed();
}

void Spellbook::renameSpell(Spell* spell, const QString& newName)
{
    if(!spell)
        return;

    if(!_spellbookMap.contains(spell->getName()))
        return;

    _spellbookMap.remove(spell->getName());
    spell->setName(newName);
    insertSpell(spell);
}

void Spellbook::setDirectory(const QDir& directory)
{
    _spellbookDirectory = directory;
}

const QDir& Spellbook::getDirectory() const
{
    return _spellbookDirectory;
}

QString Spellbook::findSpellImage(const QString& spellName, const QString& iconFile)
{
    QString fileName = iconFile;

    if((fileName.isEmpty()) || ( !_spellbookDirectory.exists(fileName) ))
    {
        fileName = QString("./") + spellName + QString(".png");
        if( !_spellbookDirectory.exists(fileName) )
        {
            fileName = QString("./") + spellName + QString(".jpg");
            if( !_spellbookDirectory.exists(fileName) )
            {
                fileName = QString("./Images/") + spellName + QString(".png");
                if( !_spellbookDirectory.exists(fileName) )
                {
                    fileName = QString("./Images/") + spellName + QString(".jpg");
                    if( !_spellbookDirectory.exists(fileName) )
                    {
                        qDebug() << "[Spellbook] Not able to find spell image for " << spellName << " with icon file " << iconFile;
                        fileName = QString();
                    }
                }
            }
        }
    }

    return fileName;
}

void Spellbook::startBatchProcessing()
{
    _batchProcessing = true;
    _batchAcknowledge = false;
}

void Spellbook::finishBatchProcessing()
{
    _batchProcessing = false;
    _batchAcknowledge = false;
}

void Spellbook::showSpellWarning(const QString& spell)
{
    qDebug() << "[Spellbook] ERROR: Requested spell not found: " << spell;

    if(_batchProcessing)
    {
        if(!_batchAcknowledge)
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle(QString("Unknown spell"));
            msgBox.setText(QString("WARNING: The spell """) + spell + QString(""" was not found in the current spellbook! If you save the current campaign, all references to this spell will be lost!"));
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
        QMessageBox::critical(nullptr, QString("Unknown spell"), QString("WARNING: The spell """) + spell + QString(""" was not found in the current spellbook! If you save the current campaign, all references to this spell will be lost!"));
    }
}
