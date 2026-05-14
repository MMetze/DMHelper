#ifndef CAMPAIGNFILESMANAGER_H
#define CAMPAIGNFILESMANAGER_H

#include <QObject>
#include <QDir>
#include <QSet>
#include <QString>
#include <QStringList>

class Campaign;
class CampaignObjectBase;

class CampaignFilesManager : public QObject
{
    Q_OBJECT
public:
    explicit CampaignFilesManager(QObject* parent = nullptr);

    // Constraint: constructor performs NO I/O and emits NO signals.
    // All I/O begins at setRootDirectory.

    void setRootDirectory(const QString& absolutePath);
    QString rootDirectory() const;

    // Path computation
    QString pathForEntry(const CampaignObjectBase* entry) const;
    QString relativePathForEntry(const CampaignObjectBase* entry) const;

    // Collision-free allocation
    QString allocateUniqueMarkdownPath(const QDir& dir, const QString& baseName) const;
    QString allocateUniqueSubdirPath(const QDir& dir, const QString& baseName) const;
    QString allocateUniqueAssetPath(const QDir& dir, const QString& baseName, const QString& suffix) const;

    // Mirror verification
    void verifyMirror(Campaign* campaign, QStringList& missingDirs) const;

    // Rename helper
    void renameEntryFile(CampaignObjectBase* entry, const QString& oldName, const QString& newName);

    // Media copy helper
    bool copyMediaInto(const QString& sourcePath, const CampaignObjectBase* owner,
                       bool isVideo, QString& outRelativePath);

    // Watcher suppression (per-path, reference-counted)
    void suspendWatch(const QString& absolutePath);
    void resumeWatch(const QString& absolutePath);
    // Whole-watcher suppression (used during migration)
    void suspendWatch();
    void resumeWatch();

    // Expected-paths registry (scanForNewEntries skips these)
    void registerExpectedPath(const QString& absolutePath);
    void clearExpectedPaths();
    bool isExpectedPath(const QString& absolutePath) const;

    // Find owning campaign by walking parent() chain
    static Campaign* findOwningCampaign(const CampaignObjectBase* entry);

signals:
    void linkedFileChanged(const QString& path);
    void markdownFileAdded(const QString& path);
    void subdirectoryAdded(const QString& path);
    void linkedFileDeleted(const QString& path);

private:
    static QString sanitiseName(const QString& name);

    QString _rootDirectory;
    QSet<QString> _expectedPaths;
    QMap<QString, int> _suspendedPaths;  // path → suspend count
    int _globalSuspendCount = 0;
};

#endif // CAMPAIGNFILESMANAGER_H
