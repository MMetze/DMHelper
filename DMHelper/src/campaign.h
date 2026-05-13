#ifndef CAMPAIGN_H
#define CAMPAIGN_H

#include "campaignobjectbase.h"
#include "basicdate.h"
#include "ruleset.h"
#include <QTime>
#include <QList>

class Characterv2;
class Combatant;
class Adventure;
class Encounter;
class Map;
class AudioTrack;
class SoundboardGroup;
class Overlay;
class QDomDocument;
class QDomElement;

class Campaign : public CampaignObjectBase
{
    Q_OBJECT
public:
    explicit Campaign(const QString& campaignName = QString(), QObject *parent = nullptr);
    virtual ~Campaign() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void postProcessXML(const QDomElement &element, bool isImport) override;
    virtual int getObjectType() const override;

    // Local Interface
    virtual void preloadRulesetXML(const QDomElement &element, bool isImport);
    virtual void beginBatchChanges();
    virtual void endBatchChanges();

    Characterv2* getCharacterById(QUuid id);
    const Characterv2* getCharacterById(QUuid id) const;
    Characterv2* getCharacterByDndBeyondId(int id);
    Characterv2* getCharacterOrNPCByDndBeyondId(int id);
    QList<Characterv2*> getActiveCharacters();
    Characterv2* getNPCById(QUuid id);
    const Characterv2* getNPCById(QUuid id) const;
    AudioTrack* getTrackById(QUuid id);

    QList<SoundboardGroup*> getSoundboardGroups() const;
    void addSoundboardGroup(SoundboardGroup* soundboardGroup);
    void removeSoundboardGroup(SoundboardGroup* soundboardGroup);

    QList<Overlay*> getOverlays();
    int getOverlayCount() const;
    int getOverlayIndex(Overlay* overlay);
    bool addOverlay(Overlay* overlay);
    bool removeOverlay(Overlay* overlay);
    bool moveOverlay(int from, int to);
    void clearOverlays();

    BasicDate getDate() const;
    QTime getTime() const;
    QStringList getNotes() const;
    QString getLastMonster() const;
    QString getFilesDirectory() const;

    int getFearCount() const;
    bool getShowTokenHealthBars() const;

    Ruleset& getRuleset();
    const Ruleset& getRuleset() const;

    bool isValid() const;
    void cleanupCampaign(bool deleteAll);

    // Major version of the campaign file that was last loaded into this object
    // (zero if this campaign was created from scratch and has never been read
    // from disk). MainWindow uses this to decide whether to write a one-time
    // pre-v3 backup before the first save under the new format.
    int getLoadedMajorVersion() const { return _loadedMajorVersion; }
    void clearLoadedMajorVersion() { _loadedMajorVersion = 0; }

signals:
    void dateChanged(const BasicDate& date);
    void timeChanged(const QTime& time);
    void fearChanged(int fearCount);
    void overlaysChanged();
    void showTokenHealthBarsChanged(bool show);

public slots:
    void setDate(const BasicDate& date);
    void setTime(const QTime& time);
    void setNotes(const QString& notes);
    void addNote(const QString& note);
    void setLastMonster(const QString& monsterName);
    void setFearCount(int fearCount);
    void setShowTokenHealthBars(bool show);
    void setFilesDirectory(const QString& filesDirectory);
    bool validateCampaignIds();
    bool correctDuplicateIds();

protected slots:
    virtual void handleInternalChange() override;
    virtual void handleInternalDirty() override;

protected:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;
    virtual void internalPostProcessXML(const QDomElement &element, bool isImport) override;

    void loadOverlayXML(const QDomElement &element);
    bool validateSingleId(QList<QUuid>& knownIds, CampaignObjectBase* baseObject, bool correctDuplicates = false);
    bool isVersionCompatible(int majorVersion, int minorVersion) const;

    BasicDate _date;
    QTime _time;
    QStringList _notes;
    QString _lastMonster;
    int _fearCount; // Todo: add ruleset-specific data storage
    bool _showTokenHealthBars;

    Ruleset _ruleset;

    bool _batchChanges;
    bool _changesMade;
    bool _dirtyMade;

    bool _isValid;

    QList<SoundboardGroup*> _soundboardGroups;
    QList<Overlay*> _overlays;

    // See getLoadedMajorVersion(). Set during inputXML(); not serialised.
    int _loadedMajorVersion = 0;

    // Relative path to the campaign's files directory (relative to the campaign
    // XML file's parent directory). Empty means no directory has been set.
    QString _filesDirectory;
};

#endif // CAMPAIGN_H
