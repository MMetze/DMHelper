#include "campaignfilesmanager.h"
#include "campaign.h"
#include "campaignobjectbase.h"
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QDebug>

// ---------- file-local constants ----------
static const int UNIQUE_SUFFIX_MAX = 99;
static const char UNIQUE_SUFFIX_SEPARATOR[] = "-";
static const char MARKDOWN_EXT[] = ".md";
static const char RESERVED_CONTENTS_NAME[] = "_contents";
static const char RESERVED_CONTENTS_SUFFIX[] = "-entry";
static const char LOG_PREFIX[] = "[CampaignFilesManager]";

// ---------- file-local helpers ----------

QString CampaignFilesManager::sanitiseName(const QString& baseName)
{
    QString result = baseName;
    static const QRegularExpression unsafeChars(QStringLiteral("[^a-zA-Z0-9_\\-. ]"));
    result.replace(unsafeChars, QStringLiteral("-"));
    result = result.trimmed();
    result.replace(QLatin1Char(' '), QLatin1Char('-'));
    if(result == QLatin1String(RESERVED_CONTENTS_NAME))
        result = QLatin1String(RESERVED_CONTENTS_NAME) + QLatin1String(RESERVED_CONTENTS_SUFFIX);
    return result;
}
static void verifyMirrorRecursive(const CampaignFilesManager* manager,
                                   CampaignObjectBase* entry,
                                   QStringList& missingDirs)
{
    if(!entry)
        return;

    const QList<CampaignObjectBase*> children = entry->getChildObjects();
    if(!children.isEmpty())
    {
        // This entry has children — it should have a corresponding directory on disk
        QString entryPath = manager->pathForEntry(entry);
        if(!entryPath.isEmpty() && !QDir(entryPath).exists())
            missingDirs << manager->relativePathForEntry(entry);

        for(CampaignObjectBase* child : children)
            verifyMirrorRecursive(manager, child, missingDirs);
    }
}

// ---------- CampaignFilesManager ----------

CampaignFilesManager::CampaignFilesManager(QObject* parent)
    : QObject(parent)
    , _rootDirectory()
    , _expectedPaths()
    , _suspendedPaths()
    , _globalSuspendCount(0)
{
    // Constructor performs NO I/O and emits NO signals.
}

void CampaignFilesManager::setRootDirectory(const QString& absolutePath)
{
    _rootDirectory = absolutePath;
    startWatching();
}

QString CampaignFilesManager::rootDirectory() const
{
    return _rootDirectory;
}

QString CampaignFilesManager::pathForEntry(const CampaignObjectBase* entry) const
{
    if(!entry || _rootDirectory.isEmpty())
        return QString();

    // Walk up the parent chain collecting name segments until we reach Campaign.
    QStringList segments;
    const CampaignObjectBase* current = entry;
    while(current)
    {
        if(qobject_cast<const Campaign*>(current))
            break;  // Reached the Campaign root — stop

        segments.prepend(sanitiseName(current->getName()));

        QObject* parentObj = current->parent();
        current = qobject_cast<const CampaignObjectBase*>(parentObj);
    }

    if(segments.isEmpty())
        return QString();

    QDir root(_rootDirectory);
    return QDir::cleanPath(root.filePath(segments.join(QLatin1Char('/'))));
}

QString CampaignFilesManager::relativePathForEntry(const CampaignObjectBase* entry) const
{
    return QDir(_rootDirectory).relativeFilePath(pathForEntry(entry));
}

QString CampaignFilesManager::allocateUniqueMarkdownPath(const QDir& dir, const QString& baseName) const
{
    QString safe = sanitiseName(baseName);

    QString candidate = dir.absoluteFilePath(safe + QLatin1String(MARKDOWN_EXT));
    if(!QFileInfo::exists(candidate))
        return candidate;

    for(int i = 2; i <= UNIQUE_SUFFIX_MAX; ++i)
    {
        candidate = dir.absoluteFilePath(
            safe + QLatin1String(UNIQUE_SUFFIX_SEPARATOR) + QString::number(i) + QLatin1String(MARKDOWN_EXT));
        if(!QFileInfo::exists(candidate))
            return candidate;
    }

    return candidate;  // All slots taken — return last attempt
}

QString CampaignFilesManager::allocateUniqueSubdirPath(const QDir& dir, const QString& baseName) const
{
    QString safe = sanitiseName(baseName);

    QString candidate = dir.absoluteFilePath(safe);
    if(!QFileInfo::exists(candidate))
        return candidate;

    for(int i = 2; i <= UNIQUE_SUFFIX_MAX; ++i)
    {
        candidate = dir.absoluteFilePath(safe + QLatin1String(UNIQUE_SUFFIX_SEPARATOR) + QString::number(i));
        if(!QFileInfo::exists(candidate))
            return candidate;
    }

    return candidate;
}

QString CampaignFilesManager::allocateUniqueAssetPath(const QDir& dir, const QString& baseName, const QString& suffix) const
{
    QString safe = sanitiseName(baseName);
    QString ext = suffix.isEmpty() ? QString() : (QLatin1String(".") + suffix);

    QString candidate = dir.absoluteFilePath(safe + ext);
    if(!QFileInfo::exists(candidate))
        return candidate;

    for(int i = 2; i <= UNIQUE_SUFFIX_MAX; ++i)
    {
        candidate = dir.absoluteFilePath(
            safe + QLatin1String(UNIQUE_SUFFIX_SEPARATOR) + QString::number(i) + ext);
        if(!QFileInfo::exists(candidate))
            return candidate;
    }

    return candidate;
}

void CampaignFilesManager::verifyMirror(Campaign* campaign, QStringList& missingDirs) const
{
    if(!campaign || _rootDirectory.isEmpty())
        return;

    for(CampaignObjectBase* child : campaign->getChildObjects())
        verifyMirrorRecursive(this, child, missingDirs);
}

void CampaignFilesManager::renameEntryFile(CampaignObjectBase* entry, const QString& oldName, const QString& newName)
{
    if(!entry || oldName.isEmpty() || oldName == newName)
        return;

    if(!findOwningCampaign(entry))
        return;

    if(_rootDirectory.isEmpty())
        return;

    // Compute the parent directory path
    QString parentPath;
    QObject* parentObj = entry->parent();
    if(qobject_cast<Campaign*>(parentObj))
    {
        parentPath = _rootDirectory;
    }
    else
    {
        CampaignObjectBase* parentEntry = qobject_cast<CampaignObjectBase*>(parentObj);
        if(parentEntry)
            parentPath = pathForEntry(parentEntry);
    }

    if(parentPath.isEmpty())
    {
        qWarning() << LOG_PREFIX << "renameEntryFile: could not determine parent path for" << oldName;
        return;
    }

    QDir parentDir(parentPath);
    QString safeOld = sanitiseName(oldName);
    QString safeNew = sanitiseName(newName);

    QString oldAbsPath = parentDir.absoluteFilePath(safeOld);
    QString newAbsPath = parentDir.absoluteFilePath(safeNew);

    bool success = false;
    if(!entry->getChildObjects().isEmpty())
    {
        // Directory entry — rename the directory
        success = QDir().rename(oldAbsPath, newAbsPath);
    }
    else
    {
        // Leaf entry — rename the .md file if it exists, otherwise rename bare path
        QString oldMdPath = oldAbsPath + QLatin1String(MARKDOWN_EXT);
        QString newMdPath = newAbsPath + QLatin1String(MARKDOWN_EXT);
        if(QFileInfo::exists(oldMdPath))
            success = QFile::rename(oldMdPath, newMdPath);
        else
            success = QFile::rename(oldAbsPath, newAbsPath);
    }

    if(!success)
        qWarning() << LOG_PREFIX << "renameEntryFile failed:" << oldAbsPath << "->" << newAbsPath;
}

bool CampaignFilesManager::copyMediaInto(const QString& sourcePath, const CampaignObjectBase* owner,
                                          bool isVideo, QString& outRelativePath)
{
    Q_UNUSED(isVideo)
    if(sourcePath.isEmpty() || !owner || _rootDirectory.isEmpty())
        return false;

    QDir root(_rootDirectory);
    QString relPath = root.relativeFilePath(sourcePath);

    // Already inside the files directory — nothing to copy
    if(!relPath.startsWith(QStringLiteral("..")))
    {
        outRelativePath = relPath;
        return true;
    }

    QString ownerPath = pathForEntry(owner);
    if(ownerPath.isEmpty())
        return false;

    QDir ownerDir(ownerPath);
    QFileInfo sourceInfo(sourcePath);
    QString ext = sourceInfo.suffix();
    QString baseName = sourceInfo.completeBaseName();

    QString destPath = allocateUniqueAssetPath(ownerDir, baseName, ext);

    if(!QFile::copy(sourcePath, destPath))
    {
        qWarning() << LOG_PREFIX << "copyMediaInto: failed to copy" << sourcePath << "to" << destPath;
        return false;
    }

    outRelativePath = root.relativeFilePath(destPath);
    return true;
}

void CampaignFilesManager::suspendWatch(const QString& absolutePath)
{
    ++_suspendedPaths[absolutePath];
}

void CampaignFilesManager::resumeWatch(const QString& absolutePath)
{
    auto it = _suspendedPaths.find(absolutePath);
    if(it == _suspendedPaths.end())
        return;

    if(--it.value() <= 0)
        _suspendedPaths.erase(it);
}

void CampaignFilesManager::suspendWatch()
{
    ++_globalSuspendCount;
}

void CampaignFilesManager::resumeWatch()
{
    if(_globalSuspendCount > 0)
        --_globalSuspendCount;
}

void CampaignFilesManager::registerExpectedPath(const QString& absolutePath)
{
    _expectedPaths.insert(absolutePath);
}

void CampaignFilesManager::clearExpectedPaths()
{
    _expectedPaths.clear();
}

bool CampaignFilesManager::isExpectedPath(const QString& absolutePath) const
{
    return _expectedPaths.contains(absolutePath);
}

Campaign* CampaignFilesManager::findOwningCampaign(const CampaignObjectBase* entry)
{
    if(!entry)
        return nullptr;

    QObject* obj = entry->parent();
    while(obj)
    {
        Campaign* campaign = qobject_cast<Campaign*>(obj);
        if(campaign)
            return campaign;
        obj = obj->parent();
    }

    return nullptr;
}

void CampaignFilesManager::startWatching()
{
    if(_rootDirectory.isEmpty())
        return;

    if(!_watcher)
    {
        _watcher = new QFileSystemWatcher(this);
        connect(_watcher, &QFileSystemWatcher::fileChanged,
                this, &CampaignFilesManager::onFileChanged);
        connect(_watcher, &QFileSystemWatcher::directoryChanged,
                this, &CampaignFilesManager::onDirectoryChanged);
    }
    else
    {
        QStringList watched = _watcher->files() + _watcher->directories();
        if(!watched.isEmpty())
            _watcher->removePaths(watched);
    }

    _dirSnapshot.clear();

    // Add root directory itself
    {
        QString rootAbs = QDir::cleanPath(_rootDirectory);
        _watcher->addPath(rootAbs);
        if(!_watcher->directories().contains(rootAbs))
            qWarning() << LOG_PREFIX << "startWatching: failed to watch directory" << rootAbs;
        QDir d(rootAbs);
        _dirSnapshot[rootAbs] = d.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    }

    // Recurse into all subdirectories and .md files
    QDirIterator it(_rootDirectory, QDirIterator::Subdirectories);
    while(it.hasNext())
    {
        it.next();
        QFileInfo info = it.fileInfo();
        if(info.isDir())
        {
            // Watch directory for structural changes
            QString absPath = info.absoluteFilePath();
            if(!_watcher->addPath(absPath))
                qWarning() << LOG_PREFIX << "startWatching: failed to watch directory" << absPath;
            // Snapshot directory contents
            _dirSnapshot[absPath] = QDir(absPath).entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        }
        else if(info.suffix().compare(QLatin1String("md"), Qt::CaseInsensitive) == 0)
        {
            // Watch .md files for content changes
            if(!_watcher->addPath(info.absoluteFilePath()))
                qWarning() << LOG_PREFIX << "startWatching: failed to watch file:" << info.absoluteFilePath();
        }
    }
}

void CampaignFilesManager::onFileChanged(const QString& path)
{
    if(_globalSuspendCount > 0)
        return;

    auto it = _suspendedPaths.find(path);
    if(it != _suspendedPaths.end())
    {
        if(--it.value() <= 0)
            _suspendedPaths.erase(it);
        return;
    }

    emit linkedFileChanged(path);
}

void CampaignFilesManager::onDirectoryChanged(const QString& dirPath)
{
    if(_globalSuspendCount > 0)
        return;

    QDir dir(dirPath);
    QStringList newEntries = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList oldEntries = _dirSnapshot.value(dirPath);

    // Detect additions
    for(const QString& entry : newEntries)
    {
        if(!oldEntries.contains(entry))
        {
            QString absEntry = dir.absoluteFilePath(entry);
            QFileInfo info(absEntry);
            if(info.isDir())
            {
                emit subdirectoryAdded(absEntry);
                // Watch the new directory and take its snapshot
                _watcher->addPath(absEntry);
                if(!_watcher->directories().contains(absEntry))
                    qWarning() << LOG_PREFIX << "onDirectoryChanged: failed to watch new directory" << absEntry;
                QDir newDir(absEntry);
                _dirSnapshot[absEntry] = newDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
            }
            else if(entry.endsWith(QLatin1String(MARKDOWN_EXT), Qt::CaseInsensitive))
            {
                _watcher->addPath(absEntry);
                emit markdownFileAdded(absEntry);
            }
        }
    }

    // Detect removals
    for(const QString& entry : oldEntries)
    {
        if(!newEntries.contains(entry))
        {
            QString absEntry = dir.absoluteFilePath(entry);
            if(entry.endsWith(QLatin1String(MARKDOWN_EXT), Qt::CaseInsensitive))
                emit linkedFileDeleted(absEntry);
        }
    }

    _dirSnapshot[dirPath] = newEntries;
}
