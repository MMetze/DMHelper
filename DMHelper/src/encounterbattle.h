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
    explicit EncounterBattle(const EncounterBattle& obj);
    virtual ~EncounterBattle() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void postProcessXML(const QDomElement &element, bool isImport) override;
    virtual void resolveReferences() override;

    // Base functions to handle UI widgets
    virtual void widgetActivated(QWidget* widget) override;
    virtual void widgetDeactivated(QWidget* widget) override;

    virtual int getType() const override;

    virtual bool hasData() const override;

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
    virtual void widgetChanged() override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

protected:
    void inputXMLBattle(const QDomElement &element, bool isImport);
    BattleDialogModel* createNewBattle(QPointF combatantPos);

    void connectFrameToModel();
    void disconnectFrameFromModel();

    QString _text;
    QUuid _audioTrackId;
    QList<CombatantGroupList> _combatantWaves;
    BattleDialogModel* _battleModel;

};

#endif // ENCOUNTERBATTLE_H
