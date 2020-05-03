#ifndef ENCOUNTERBATTLE_H
#define ENCOUNTERBATTLE_H

#include "encountertext.h"
#include "combatant.h"
#include <QDomElement>

class QDomDocument;
class BattleDialogModel;
class AudioTrack;

class EncounterBattle : public EncounterText
{
    Q_OBJECT
public:

    explicit EncounterBattle(const QString& encounterName = QString(), QObject *parent = nullptr);
    //explicit EncounterBattle(const EncounterBattle& obj);
    virtual ~EncounterBattle() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    //virtual void resolveReferences() override;

    // Base functions to handle UI widgets
    //virtual void widgetActivated(QWidget* widget) override;
    //virtual void widgetDeactivated(QWidget* widget) override;

    virtual int getObjectType() const override;

    virtual bool hasData() const;

//    virtual QString getText() const;

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

    void createBattleDialogModel();
    void setBattleDialogModel(BattleDialogModel* model);
    BattleDialogModel* getBattleDialogModel() const;
    void removeBattleDialogModel();

public slots:
//    virtual void setText(const QString& newText);

protected:
//    virtual void widgetChanged() override;
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;
    virtual void internalPostProcessXML(const QDomElement &element, bool isImport) override;

    void inputXMLBattle(const QDomElement &element, bool isImport);
    BattleDialogModel* createNewBattle(QPointF combatantPos);

    void connectFrameToModel();
    void disconnectFrameFromModel();

//    QString _text;
    QUuid _audioTrackId;
    QList<CombatantGroupList> _combatantWaves;
    BattleDialogModel* _battleModel;

};

#endif // ENCOUNTERBATTLE_H
