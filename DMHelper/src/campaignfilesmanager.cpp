#include "campaignfilesmanager.h"
#include "campaign.h"
#include "campaignobjectbase.h"
#include "encountertext.h"
#include "encountertextlinked.h"
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
static const char SCAN_CONTENTS_FILENAME[] = "_contents.md";

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

// ---------- scanForNewEntries helpers ----------

static void collectEntryPaths(const CampaignFilesManager* mgr,
                               CampaignObjectBase* parent,
                               QMap<QString, CampaignObjectBase*>& pathToEntry,
                               QSet<QString>& knownPaths)
{
    for(CampaignObjectBase* child : parent->getChildObjects())
    {
        const QString p = mgr->pathForEntry(child);
        if(!p.isEmpty())
        {
            pathToEntry.insert(p, child);
            knownPaths.insert(p);
        }
        collectEntryPaths(mgr, child, pathToEntry, knownPaths);
    }
}

void CampaignFilesManager::scanForNewEntries(Campaign* campaign, QList<CampaignObjectBase*>& discovered)
{
    if(_rootDirectory.isEmpty() || campaign == nullptr)
        return;

    // Build path-to-entry map and known-paths set from the existing campaign tree
    QMap<QString, CampaignObjectBase*> pathToEntry;
    QSet<QString> knownPaths;
    collectEntryPaths(this, campaign, pathToEntry, knownPaths);

    static const QLatin1String mdSuffix("md");
    const QString cleanRoot = QDir::cleanPath(_rootDirectory);

    // --- Pass 1: collect all items from the iterator into separate lists ---
    // Directories and files are gathered in a single sweep; directories are
    // processed first (shallow before deep) so parent container entries exist
    // in pathToEntry before their children are resolved.
    QStringList unknownDirs;
    QList<QFileInfo> unknownFiles;

    QDirIterator it(_rootDirectory, QDirIterator::Subdirectories);
    while(it.hasNext())
    {
        it.next();
        const QFileInfo info = it.fileInfo();
        const QString absPath = QDir::cleanPath(info.absoluteFilePath());

        if(info.isDir())
        {
            // Skip the root itself and any dir already present in the campaign tree
            if(absPath == cleanRoot || knownPaths.contains(absPath))
                continue;
            unknownDirs.append(absPath);
        }
        else
        {
            // Only process .md files
            if(info.suffix().compare(mdSuffix, Qt::CaseInsensitive) != 0)
                continue;
            // Skip _contents.md — loaded as part of the directory entry below
            if(info.fileName() == QLatin1String(SCAN_CONTENTS_FILENAME))
                continue;
            // Base path = absolute path with .md stripped; matches pathForEntry() values
            const QString basePath = QDir::cleanPath(
                info.absolutePath() + QLatin1Char('/') + info.completeBaseName());
            if(knownPaths.contains(basePath))
                continue;
            unknownFiles.append(info);
        }
    }

    // --- Pass 2: process unknown directories, shallowest first ---
    // Sorting by path length guarantees parents are inserted before children.
    std::sort(unknownDirs.begin(), unknownDirs.end(),
              [](const QString& a, const QString& b) { return a.length() < b.length(); });

    for(const QString& dirPath : unknownDirs)
    {
        const QFileInfo dirInfo(dirPath);
        const QString parentDirPath = QDir::cleanPath(dirInfo.absolutePath());

        CampaignObjectBase* parentEntry = nullptr;
        if(parentDirPath == cleanRoot)
        {
            parentEntry = campaign;
        }
        else
        {
            auto parentIt = pathToEntry.find(parentDirPath);
            if(parentIt != pathToEntry.end())
                parentEntry = parentIt.value();
            else
                parentEntry = campaign;  // Unknown parent — fall back to campaign root
        }

        EncounterText* dirEntry = new EncounterText(dirInfo.fileName());
        parentEntry->addObject(dirEntry);

        // Register in the lookup maps BEFORE processing children so that .md files
        // nested inside this directory can resolve this entry as their parent.
        pathToEntry.insert(dirPath, dirEntry);
        knownPaths.insert(dirPath);

        discovered.append(dirEntry);

        // If a _contents.md exists in this directory, attach it as a linked child
        // so the directory entry's body is backed by the on-disk file.
        const QString contentsPath = dirPath + QLatin1Char('/') + QLatin1String(SCAN_CONTENTS_FILENAME);
        if(QFileInfo::exists(contentsPath))
        {
            // addObject() before setLinkedFile() so findOwningCampaign() works inside setLinkedFile()
            EncounterTextLinked* contentsEntry = new EncounterTextLinked(dirInfo.fileName());
            dirEntry->addObject(contentsEntry);
            contentsEntry->setLinkedFile(contentsPath);
            discovered.append(contentsEntry);
        }
    }

    // --- Pass 3: process unknown .md files ---
    for(const QFileInfo& info : unknownFiles)
    {
        const QString basePath = QDir::cleanPath(
            info.absolutePath() + QLatin1Char('/') + info.completeBaseName());
        const QString parentDirPath = QDir::cleanPath(info.absolutePath());
        CampaignObjectBase* parentEntry = nullptr;

        if(parentDirPath == cleanRoot)
        {
            // File sits directly under the root — attach to the campaign itself
            parentEntry = campaign;
        }
        else
        {
            auto parentIt = pathToEntry.find(parentDirPath);
            if(parentIt != pathToEntry.end())
                parentEntry = parentIt.value();
            else
                parentEntry = campaign;  // Unknown parent — fall back to campaign root
        }

        // Create the entry, wire it into the tree, then load its content.
        // addObject() must be called before setLinkedFile() so that
        // findOwningCampaign() can locate the CampaignFilesManager for
        // the watcher connection inside setLinkedFile().
        EncounterTextLinked* newEntry = new EncounterTextLinked(info.completeBaseName());
        parentEntry->addObject(newEntry);
        newEntry->setLinkedFile(info.absoluteFilePath());

        // Register the new entry so that subsequent iterations in this scan
        // can find it as a parent for deeper-nested files
        pathToEntry.insert(basePath, newEntry);
        knownPaths.insert(basePath);

        discovered.append(newEntry);
    }
}

void CampaignFilesManager::renameEntryFile(CampaignObjectBase* entry, const QString& oldName, const QString& newName)
{
    if(!entry || oldName.isEmpty() || newName.isEmpty() || oldName == newName)
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
    QString renamedToAbsPath;

    if(!entry->getChildObjects().isEmpty())
    {
        // Directory entry — rename the directory
        success = QDir().rename(oldAbsPath, newAbsPath);
        if(!success && QFileInfo::exists(newAbsPath))
        {
            // Collision — allocate a unique path and retry once
            newAbsPath = allocateUniqueSubdirPath(parentDir, newName);
            success = QDir().rename(oldAbsPath, newAbsPath);
        }
        if(success)
            renamedToAbsPath = newAbsPath;
    }
    else
    {
        // Leaf entry — rename the .md file
        QString oldMdPath = oldAbsPath + QLatin1String(MARKDOWN_EXT);
        QString newMdPath = newAbsPath + QLatin1String(MARKDOWN_EXT);
        if(QFileInfo::exists(oldMdPath))
        {
            success = QFile::rename(oldMdPath, newMdPath);
            if(!success && QFileInfo::exists(newMdPath))
            {
                // Collision — allocate a unique .md path and retry once
                newMdPath = allocateUniqueMarkdownPath(parentDir, newName);
                success = QFile::rename(oldMdPath, newMdPath);
            }
            if(success)
                renamedToAbsPath = newMdPath;
        }
        else
        {
            success = QFile::rename(oldAbsPath, newAbsPath);
            if(!success && QFileInfo::exists(newAbsPath))
            {
                newAbsPath = allocateUniqueMarkdownPath(parentDir, newName);
                success = QFile::rename(oldAbsPath, newAbsPath);
            }
            if(success)
                renamedToAbsPath = newAbsPath;
        }
    }

    if(!success)
    {
        qWarning() << LOG_PREFIX << "renameEntryFile failed:" << oldAbsPath << "->" << newAbsPath;
        return;
    }

    // Update EncounterTextLinked's linked file path after successful rename.
    // Use updateLinkedFilePath (not setLinkedFile) so we do NOT re-read the file
    // content — the content is unchanged; only the path moved.
    EncounterTextLinked* linked = qobject_cast<EncounterTextLinked*>(entry);
    if(linked && !renamedToAbsPath.isEmpty())
        linked->updateLinkedFilePath(renamedToAbsPath);
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

    if(!QDir().mkpath(ownerPath))
    {
        qWarning() << LOG_PREFIX << "copyMediaInto: failed to create directory" << ownerPath;
        outRelativePath = sourcePath;
        return false;
    }

    if(!QFile::copy(sourcePath, destPath))
    {
        qWarning() << LOG_PREFIX << "copyMediaInto: failed to copy" << sourcePath << "to" << destPath;
        outRelativePath = sourcePath;
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
