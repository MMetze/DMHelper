#include "encountertextlinked.h"
#include "dmconstants.h"
#include <QDir>
#include <QFile>
#include <QFileSystemWatcher>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTimerEvent>
#include <QDebug>

// A single external save usually arrives as several notifications, and the file is briefly absent mid-replace
static constexpr int ENCOUNTERTEXTLINKED_RELOAD_INTERVAL = 250;
static constexpr int ENCOUNTERTEXTLINKED_MAX_RELOAD_RETRIES = 20;

EncounterTextLinked::EncounterTextLinked(const QString& encounterName, QObject *parent) :
    EncounterText{encounterName, parent},
    _linkedFile(),
    _watcher(nullptr),
    _fileType(DMHelper::FileType_Unknown),
    _metadata(),
    _reloadTimer(0),
    _reloadRetries(0)
{
}

void EncounterTextLinked::inputXML(const QDomElement &element, bool isImport)
{
    _linkedFile = element.attribute("linkedFile");

    // EncounterText::inputXML calls extractTextNode, which reads the linked file set above
    EncounterText::inputXML(element, isImport);
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
    if(_text == newText)
        return;

    _text = newText;
    writeLinkedFile();

    // No dirty() - the text lives in the linked file, not in the campaign XML
    emit textChanged(_text);
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
        connect(_watcher, &QFileSystemWatcher::fileChanged, this, &EncounterTextLinked::handleFileChanged);
        if(!getLinkedFile().isEmpty())
        {
            _watcher->addPath(getLinkedFile());
            // Picks up any external change made while this entry was not being watched
            readLinkedFile();
        }
    }
    else
    {
        if(_reloadTimer)
        {
            killTimer(_reloadTimer);
            _reloadTimer = 0;
        }

        delete _watcher;
        _watcher = nullptr;
    }
}

void EncounterTextLinked::handleFileChanged(const QString& path)
{
    Q_UNUSED(path);

    rearmWatcher();

    _reloadRetries = 0;
    if(_reloadTimer)
        killTimer(_reloadTimer);

    _reloadTimer = startTimer(ENCOUNTERTEXTLINKED_RELOAD_INTERVAL);
}

void EncounterTextLinked::rearmWatcher()
{
    // Editors that save by writing a temp file and renaming over the original drop the path from the watch list
    if((!_watcher) || (_linkedFile.isEmpty()) || (_watcher->files().contains(_linkedFile)))
        return;

    if(QFile::exists(_linkedFile))
        _watcher->addPath(_linkedFile);
}

void EncounterTextLinked::timerEvent(QTimerEvent* event)
{
    if((!event) || (event->timerId() != _reloadTimer))
    {
        EncounterText::timerEvent(event);
        return;
    }

    killTimer(_reloadTimer);
    _reloadTimer = 0;

    // Still mid-replace: wait for the rename to land rather than reading a missing or partial file
    if((!_linkedFile.isEmpty()) && (!QFile::exists(_linkedFile)) && (_reloadRetries < ENCOUNTERTEXTLINKED_MAX_RELOAD_RETRIES))
    {
        ++_reloadRetries;
        _reloadTimer = startTimer(ENCOUNTERTEXTLINKED_RELOAD_INTERVAL);
        return;
    }

    rearmWatcher();
    readLinkedFile();
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
    if(_linkedFile.isEmpty())
        return;

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
    QString inputString = in.readAll();
    extFile.close();

    _metadata.clear();
    QString newText = extractMetadata(inputString);

    // Suppresses the file system watcher notification triggered by our own write-through
    if(_text == newText)
        return;

    // No dirty() - reading from disk never changes the campaign XML
    _text = newText;
    emit textChanged(_text);
}

void EncounterTextLinked::writeLinkedFile()
{
    if((_linkedFile.isEmpty()) || (_fileType == DMHelper::FileType_Unknown))
        return;

    QFile linkedFile(_linkedFile);
    if(!linkedFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug() << "[EncounterTextLinked] ERROR: unabled to open the linked file for writing: " << getLinkedFile();
        return;
    }

    QString outputString;
    if((_fileType == DMHelper::FileType_Markdown) && (!_metadata.isEmpty()))
        outputString = QString("---") + _metadata + QString("---") + _text;
    else
        outputString = _text;

    linkedFile.write(outputString.toUtf8());
    linkedFile.close();

    rearmWatcher();
}

void EncounterTextLinked::createTextNode(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    Q_UNUSED(doc);
    Q_UNUSED(element);
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    writeLinkedFile();
}

void EncounterTextLinked::extractTextNode(const QDomElement &element, bool isImport)
{
    Q_UNUSED(element);
    Q_UNUSED(isImport);

    readLinkedFile();
}

QString EncounterTextLinked::extractMetadata(const QString& inputString)
{
    if(_fileType != DMHelper::FileType_Markdown)
        return inputString;

    // Front matter only: anchored at the start and non-greedy so that horizontal rules in the body aren't consumed
    static QRegularExpression re(QString("\\A---((?:\\s|.)*?)---((?:\\s|.)*)"));
    QRegularExpressionMatch reMatch = re.match(inputString);
    if(!reMatch.hasMatch())
        return inputString;

    _metadata = reMatch.captured(1);
    parseMetadata();

    return reMatch.captured(2);
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
