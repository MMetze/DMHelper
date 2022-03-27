#ifndef CAMPAIGN_H
#define CAMPAIGN_H

#include "campaignobjectbase.h"
#include "basicdate.h"
#include <QTime>
#include <QList>

class Character;
class Combatant;
class Adventure;
class Encounter;
class Map;
class AudioTrack;
class SoundboardGroup;
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

    virtual void beginBatchChanges();
    virtual void endBatchChanges();

    Character* getCharacterById(QUuid id);
    const Character* getCharacterById(QUuid id) const;
    Character* getCharacterByDndBeyondId(int id);
    Character* getCharacterOrNPCByDndBeyondId(int id);
    QList<Character*> getActiveCharacters();
    Character* getNPCById(QUuid id);
    const Character* getNPCById(QUuid id) const;
    AudioTrack* getTrackById(QUuid id);

    QList<SoundboardGroup*> getSoundboardGroups() const;
    void addSoundboardGroup(SoundboardGroup* soundboardGroup);
    void removeSoundboardGroup(SoundboardGroup* soundboardGroup);

    BasicDate getDate() const;
    QTime getTime() const;
    QStringList getNotes() const;

    bool isValid() const;
    void cleanupCampaign(bool deleteAll);

signals:
    void dateChanged(const BasicDate& date);
    void timeChanged(const QTime& time);

public slots:
    void setDate(const BasicDate& date);
    void setTime(const QTime& time);
    void setNotes(const QString& notes);
    void addNote(const QString& note);
    bool validateCampaignIds();

protected slots:
    virtual void handleInternalChange() override;
    virtual void handleInternalDirty() override;

protected:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;
    virtual void internalPostProcessXML(const QDomElement &element, bool isImport) override;

    bool validateSingleId(QList<QUuid>& knownIds, CampaignObjectBase* baseObject);
    bool isVersionCompatible(int majorVersion, int minorVersion) const;

    BasicDate _date;
    QTime _time;
    QStringList _notes;

    bool _batchChanges;
    bool _changesMade;
    bool _dirtyMade;

    bool _isValid;

    QList<SoundboardGroup*> _soundboardGroups;
};

#endif // CAMPAIGN_H
