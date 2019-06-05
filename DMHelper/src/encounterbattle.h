#ifndef ENCOUNTERBATTLE_H
#define ENCOUNTERBATTLE_H

#include "encounter.h"
#include "combatant.h"

class QDomDocument;
class QDomElement;
class BattleDialogModel;
class AudioTrack;

class EncounterBattle : public Encounter
{
    Q_OBJECT
public:

    explicit EncounterBattle(const QString& encounterName, QObject *parent);
    virtual ~EncounterBattle();

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport);
    virtual void postProcessXML(const QDomElement &element, bool isImport);

    // Base functions to handle UI widgets
    virtual void widgetActivated(QWidget* widget);
    virtual void widgetDeactivated(QWidget* widget);

    virtual int getType() const;

    virtual bool hasData() const;

    virtual QString getText() const;

    AudioTrack* getAudioTrack();
    QUuid getAudioTrackId();
    void setAudioTrack(AudioTrack* track);

    int getWaveCount() const;
    void insertWave(int wave);
    void removeWave(int wave);
    CombatantGroupList getCombatants(int wave) const;
    int getCombatantCount(int wave) const;
    CombatantGroupList getCombatantsAllWaves() const;
    int getCombatantCountAllWaves() const;
    void addCombatant(int wave, int count, Combatant* combatant);
    void editCombatant(int wave, int index, int count, Combatant* combatant);
    void removeCombatant(int wave, int index);
    Combatant* getCombatantById(QUuid combatantId, int combatantIntId) const;

    void setBattleDialogModel(BattleDialogModel* model);
    BattleDialogModel* getBattleDialogModel() const;
    void removeBattleDialogModel();

public slots:
    virtual void setText(const QString& newText);

protected slots:
    virtual void widgetChanged();
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport);

protected:
    void inputXMLBattle(const QDomElement &element, bool isImport);

    QString _text;
    QUuid _audioTrackId;
    QList<CombatantGroupList> _combatantWaves;
    BattleDialogModel* _battleModel;

};

#endif // ENCOUNTERBATTLE_H
