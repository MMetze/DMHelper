#include "encountertextlinked.h"
#include "dmconstants.h"
#include <QDir>
#include <QFileSystemWatcher>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDebug>

EncounterTextLinked::EncounterTextLinked(const QString& encounterName, QObject *parent) :
    EncounterText{encounterName, parent},
    _linkedFile(),
    _watcher(nullptr),
    _fileType(DMHelper::FileType_Unknown),
    _metadata()
{
}

void EncounterTextLinked::inputXML(const QDomElement &element, bool isImport)
{
    _linkedFile = element.attribute("linkedFile");
    EncounterText::inputXML(element, isImport);  // this calls extractTextNode via virtual dispatch
}

void EncounterTextLinked::copyValues(const CampaignObjectBase* other)
{
    const EncounterTextLinked* otherEntry = dynamic_cast<const EncounterTextLinked*>(other);
    if(!otherEntry)
        return;

    setLinkedFile(otherEntry->_linkedFile);

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

int EncounterTextLinked::getFileType() const
{
    return _fileType;
}

QString EncounterTextLinked::getMetadata() const
{
    return _metadata;
}

void EncounterTextLinked::setText(const QString& newText)
{
    if (newText == getText())
        return;
    EncounterText::setText(newText);  // updates _text and emits dirty()
    if (_linkedFile.isEmpty())
        return;
    writeLinkedFile();
}

void EncounterTextLinked::writeLinkedFile()
{
    // TODO (filesdir-watcher): bracket with CampaignFilesManager::suspendWatch/resumeWatch
    // once CampaignFilesManager exists (chunk filesdir-manager / filesdir-watcher).
    QDomDocument doc;
    QDomElement element;
    QDir dir;
    createTextNode(doc, element, dir, false);
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
    readLinkedFileInternal(true);
}

void EncounterTextLinked::readLinkedFileInternal(bool emitDirty)
{
    if(_linkedFile.isEmpty())
        return;

    _metadata.clear();

    QFile extFile(_linkedFile);
    QFileInfo fileInfo(extFile);
    if(fileInfo.suffix() == QString("txt"))
        _fileType = DMHelper::FileType_Text;
    else if((fileInfo.suffix() == QString("htm")) || (fileInfo.suffix() == QString("html")))
        _fileType = DMHelper::FileType_HTML;
    else if(fileInfo.suffix() == QString("md"))
        _fileType = DMHelper::FileType_Markdown;
    else
    {
        qDebug() << "[EncounterTextLinked] ERROR: unabled to identify type of the linked file for reading: " << getLinkedFile();
        _fileType = DMHelper::FileType_Unknown;
        return;
    }

    if(!extFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "[EncounterTextLinked] ERROR: unabled to open the linked file for reading: " << getLinkedFile();
        return;
    }

    QTextStream in(&extFile);
    in.setEncoding(QStringConverter::Utf8);
    QString inputString;
    QString line;
    while(in.readLineInto(&line))
        inputString += QChar::LineFeed + line;

    QString extracted = extractMetadata(inputString);
    if(emitDirty)
        EncounterText::setText(extracted);   // updates _text AND emits dirty()
    else
        _text = extracted;                   // update _text silently (load-time only)
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

    QString outputString;
    if((_fileType == DMHelper::FileType_Markdown) && (!_metadata.isEmpty()))
        outputString = QString("---") + _metadata + QString("---") + _text;
    else
        outputString = _text;

    mdFile.write(outputString.toUtf8());
    mdFile.close();
}

void EncounterTextLinked::extractTextNode(const QDomElement &element, bool isImport)
{
    Q_UNUSED(element);
    Q_UNUSED(isImport);

    readLinkedFileInternal(false);
}

QString EncounterTextLinked::extractMetadata(const QString& inputString)
{
    if(_fileType != DMHelper::FileType_Markdown)
        return inputString;

    static QRegularExpression re(QString("---((\\s|.)*)---((\\s|.)*)"));
    QRegularExpressionMatch reMatch = re.match(inputString);
    if(!reMatch.hasMatch())
        return inputString;

    _metadata = reMatch.captured(1);
    parseMetadata();

    return reMatch.captured(3);
}

void EncounterTextLinked::parseMetadata()
{
    if(_metadata.isEmpty())
        return;

    qDebug() << "[EncounterTextLinked] Reading Markdown file, found metadata: ";

    static QRegularExpression re(QString("(.+): (.+)"));
    QRegularExpressionMatchIterator i = re.globalMatch(_metadata);
    QStringList words;
    while(i.hasNext())
    {
        QRegularExpressionMatch match = i.next();
        QString keyString = match.captured(1);
        QString valueString = match.captured(2);
        qDebug() << "[EncounterTextLinked]     Key: " << keyString << ", value: " << valueString;
    }
}
