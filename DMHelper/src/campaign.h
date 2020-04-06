#ifndef CAMPAIGN_H
#define CAMPAIGN_H

#include "campaignobjectbase.h"
#include "basicdate.h"
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
    explicit Campaign(const QString& campaignName = QString(), QObject *parent = nullptr);
    //explicit Campaign(const QDomElement& element, bool isImport, QObject *parent = nullptr);
    virtual ~Campaign() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void postProcessXML(const QDomElement &element, bool isImport) override;

    virtual int getObjectType() const override;
//    virtual void resolveReferences() override;

    //virtual void widgetActivated(QWidget* widget) override;
    //virtual void widgetDeactivated(QWidget* widget) override;

    virtual void beginBatchChanges();
    virtual void endBatchChanges();

//    QString getName() const;
//    void setName(const QString& campaignName);

    //Encounter* getNotes() const;

//    int getCharacterCount();
    Character* getCharacterById(QUuid id);
    const Character* getCharacterById(QUuid id) const;
    Character* getCharacterByDndBeyondId(int id);
    Character* getCharacterOrNPCByDndBeyondId(int id);
//    Character* getCharacterByIndex(int index);
//    QUuid addCharacter(Character* character);
//    Character* removeCharacter(QUuid id);
    QList<Character*> getActiveCharacters();
//    QList<Combatant*> getActiveCombatants();

//    int getAdventureCount();
//    Adventure* getAdventureById(QUuid id);
//    Adventure* getAdventureByIndex(int index);
//    QUuid addAdventure(Adventure* adventure);
//    Adventure* removeAdventure(QUuid id);

//    int getSettingCount();
//    Map* getSettingById(QUuid id);
//    Map* getSettingByIndex(int index);
//    QUuid addSetting(Map* setting);
//    Map* removeSetting(QUuid id);

//    int getNPCCount();
    Character* getNPCById(QUuid id);
    const Character* getNPCById(QUuid id) const;
//    Character* getNPCByIndex(int index);
//    QUuid addNPC(Character* npc);
//    Character* removeNPC(QUuid id);

//    int getTrackCount();
    AudioTrack* getTrackById(QUuid id);
//    AudioTrack* getTrackByIndex(int index);
//    QUuid addTrack(AudioTrack* track);
//    AudioTrack* removeTrack(QUuid id);

    //QList<CampaignObjectBase*> getObjectsByType(int campaignType);

    /*
    bool getPartyExpanded() const;
    void setPartyExpanded(bool expanded);

    bool getContentsExpanded() const;
    void setAdventuresExpanded(bool expanded);

    bool getWorldExpanded() const;
    void setWorldExpanded(bool expanded);

    bool getWorldSettingsExpanded() const;
    void setWorldSettingsExpanded(bool expanded);

    bool getWorldNPCsExpanded() const;
    void setWorldNPCsExpanded(bool expanded);
*/

    BasicDate getDate() const;
    QTime getTime() const;

    bool isValid() const;
    void cleanupCampaign(bool deleteAll);

signals:
    void dateChanged(const BasicDate& date);
    void timeChanged(const QTime& time);

public slots:
    void setDate(const BasicDate& date);
    void setTime(const QTime& time);
    bool validateCampaignIds();

protected slots:
    virtual void handleInternalChange() override;
    virtual void handleInternalDirty() override;

protected:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    bool validateSingleId(QList<QUuid>& knownIds, CampaignObjectBase* baseObject);
    bool isVersionCompatible(int majorVersion, int minorVersion) const;

    //QString _name;
    //Encounter* _notes;

    //bool _partyExpanded;
    //bool _contentsExpanded;

    BasicDate _date;
    QTime _time;

    bool _batchChanges;
    bool _changesMade;
    bool _dirtyMade;

    bool _isValid;
};

#endif // CAMPAIGN_H
