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
    virtual void loadOverlayXML(const QDomElement &element, bool isImport);
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

    int getFearCount() const;

    Ruleset& getRuleset();
    const Ruleset& getRuleset() const;

    bool isValid() const;
    void cleanupCampaign(bool deleteAll);

signals:
    void dateChanged(const BasicDate& date);
    void timeChanged(const QTime& time);
    void fearChanged(int fearCount);
    void overlaysChanged();

public slots:
    void setDate(const BasicDate& date);
    void setTime(const QTime& time);
    void setNotes(const QString& notes);
    void addNote(const QString& note);
    void setFearCount(int fearCount);
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

    bool validateSingleId(QList<QUuid>& knownIds, CampaignObjectBase* baseObject, bool correctDuplicates = false);
    bool isVersionCompatible(int majorVersion, int minorVersion) const;

    BasicDate _date;
    QTime _time;
    QStringList _notes;
    int _fearCount; // Todo: add ruleset-specific data storage

    Ruleset _ruleset;

    bool _batchChanges;
    bool _changesMade;
    bool _dirtyMade;

    bool _isValid;

    QList<SoundboardGroup*> _soundboardGroups;
    QList<Overlay*> _overlays;
};

#endif // CAMPAIGN_H
