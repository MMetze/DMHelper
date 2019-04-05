#ifndef CAMPAIGN_H
#define CAMPAIGN_H

#include "campaignobjectbase.h"
#include "basicdate.h"
#include <QList>
#include <QTime>

class Character;
class Combatant;
class Adventure;
class Encounter;
class Map;
class AudioTrack;
class QDomDocument;
class QDomElement;

class Campaign : public CampaignObjectBase
{
    Q_OBJECT
public:
    explicit Campaign(const QString& campaignName, QObject *parent = nullptr);
    explicit Campaign(const QDomElement& element, QObject *parent = nullptr);
    ~Campaign();

    // From CampaignObjectBase
    virtual void outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory);
    virtual void inputXML(const QDomElement &element);
    virtual void postProcessXML(const QDomElement &element);

    virtual void beginBatchChanges();
    virtual void endBatchChanges();

    QString getName() const;
    void setName(const QString& campaignName);

    Encounter* getNotes() const;

    int getCharacterCount();
    Character* getCharacterById(int id);
    const Character* getCharacterById(int id) const;
    Character* getCharacterByDndBeyondId(int id);
    Character* getCharacterByIndex(int index);
    int addCharacter(Character* character);
    Character* removeCharacter(int id);
    QList<Character*> getActiveCharacters();
    QList<Combatant*> getActiveCombatants();

    int getAdventureCount();
    Adventure* getAdventureById(int id);
    Adventure* getAdventureByIndex(int index);
    int addAdventure(Adventure* adventure);
    Adventure* removeAdventure(int id);

    int getSettingCount();
    Map* getSettingById(int id);
    Map* getSettingByIndex(int index);
    int addSetting(Map* setting);
    Map* removeSetting(int id);

    int getNPCCount();
    Character* getNPCById(int id);
    const Character* getNPCById(int id) const;
    Character* getNPCByIndex(int index);
    int addNPC(Character* npc);
    Character* removeNPC(int id);

    int getTrackCount();
    AudioTrack* getTrackById(int id);
    AudioTrack* getTrackByIndex(int index);
    int addTrack(AudioTrack* track);
    AudioTrack* removeTrack(int id);

    CampaignObjectBase* getObjectbyId(int id);

    bool getPartyExpanded() const;
    void setPartyExpanded(bool expanded);

    bool getAdventuresExpanded() const;
    void setAdventuresExpanded(bool expanded);

    bool getWorldExpanded() const;
    void setWorldExpanded(bool expanded);

    bool getWorldSettingsExpanded() const;
    void setWorldSettingsExpanded(bool expanded);

    bool getWorldNPCsExpanded() const;
    void setWorldNPCsExpanded(bool expanded);

    BasicDate getDate() const;
    QTime getTime() const;

    bool isValid() const;

signals:
    void changed();
    void dirty();

    void dateChanged(const BasicDate& date);
    void timeChanged(const QTime& time);

public slots:
    void setDate(const BasicDate& date);
    void setTime(const QTime& time);
    bool validateCampaignIds();

private slots:
    void handleInteralChange();
    void handleInternalDirty();

private:

    bool validateSingleId(QHash<int, int>& knownIds, int& largestId, CampaignObjectBase* baseObject);

    QString _name;
    Encounter* _notes;
    QList<Character*> characters;
    QList<Adventure*> adventures;
    QList<Map*> settings;
    QList<Character*> npcs;
    QList<AudioTrack*> tracks;

    bool _partyExpanded;
    bool _adventuresExpanded;
    bool _worldExpanded;
    bool _worldSettingsExpanded;
    bool _worldNPCsExpanded;

    BasicDate _date;
    QTime _time;

    bool _batchChanges;
    bool _changesMade;
    bool _dirtyMade;

    bool _isValid;
};

#endif // CAMPAIGN_H
