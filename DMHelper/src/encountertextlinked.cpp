#include "encountertextlinked.h"
#include "dmconstants.h"
#include <QDir>
#include <QFileSystemWatcher>
#include <QDebug>

EncounterTextLinked::EncounterTextLinked(const QString& encounterName, QObject *parent) :
    EncounterText{encounterName, parent},
    _linkedFile(),
    _watcher(nullptr)
{
}

void EncounterTextLinked::inputXML(const QDomElement &element, bool isImport)
{
    // TODO: markdown - read the linked file
    //extractTextNode(element, isImport);
    _linkedFile = element.attribute("linkedFile");

    EncounterText::inputXML(element, isImport);
}

void EncounterTextLinked::copyValues(const CampaignObjectBase* other)
{
    const EncounterTextLinked* otherEntry = dynamic_cast<const EncounterTextLinked*>(other);
    if(!otherEntry)
        return;

    _linkedFile = otherEntry->_linkedFile;

    EncounterText::copyValues(other);
}

int EncounterTextLinked::getObjectType() const
{
    return DMHelper::CampaignType_LinkedText;
}

QString EncounterTextLinked::getLinkedFile() const
{
    return _linkedFile;
}

void EncounterTextLinked::setText(const QString& newText)
{
    Q_UNUSED(newText);
    qDebug() << "[EncounterTextLinked] ERROR: Attempting to directly set the text of a linked entry!";
    return;
}

void EncounterTextLinked::setLinkedFile(const QString& filename)
{
    if(_linkedFile != filename)
    {
        if((_watcher) && (!_linkedFile.isEmpty()))
            _watcher->removePath(_linkedFile);

        _linkedFile = filename;
        readLinkedFile();

        if((_watcher) && (!_linkedFile.isEmpty()))
            _watcher->addPath(_linkedFile);
    }
}

void EncounterTextLinked::setWatcher(bool enable)
{
    if(enable)
    {
        if(_watcher)
            return;

        _watcher = new QFileSystemWatcher(this);
        connect(_watcher, &QFileSystemWatcher::fileChanged, this, &EncounterTextLinked::readLinkedFile);
        if(!getLinkedFile().isEmpty())
            _watcher->addPath(getLinkedFile());
    }
    else
    {
        delete _watcher;
        _watcher = nullptr;
    }
}

QDomElement EncounterTextLinked::createOutputXML(QDomDocument &doc)
{
    return doc.createElement("linked-object");
}

void EncounterTextLinked::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("linkedFile", targetDirectory.relativeFilePath(getLinkedFile()));
    EncounterText::internalOutputXML(doc, element, targetDirectory, isExport);
}

void EncounterTextLinked::readLinkedFile()
{
    if(getLinkedFile().isEmpty())
        return;

    QFile mdFile(getLinkedFile());
    if(!mdFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "[EncounterTextLinked] ERROR: unabled to open the linked file for reading: " << getLinkedFile();
        return;
    }

    QTextStream in(&mdFile);
    in.setCodec("UTF-8");
    QString inputString;
    QString line;
    while(in.readLineInto(&line))
        inputString += QChar::LineFeed + line;
//        inputString += QString("<br />") + line;

    //qDebug() << inputString;
    EncounterText::setText(inputString);
}

void EncounterTextLinked::createTextNode(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    Q_UNUSED(doc);
    Q_UNUSED(element);
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    QFile mdFile(getLinkedFile());
    if(!mdFile.open(QIODevice::WriteOnly))
    {
        qDebug() << "[EncounterTextLinked] ERROR: unabled to open the linked file for writing: " << getLinkedFile();
        return;
    }

    mdFile.write(_text.toUtf8());
    mdFile.close();
}

void EncounterTextLinked::extractTextNode(const QDomElement &element, bool isImport)
{
    Q_UNUSED(element);
    Q_UNUSED(isImport);

    readLinkedFile();
}
