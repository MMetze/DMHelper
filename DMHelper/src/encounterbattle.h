#ifndef ENCOUNTERBATTLE_H
#define ENCOUNTERBATTLE_H

#include "encountertext.h"
#include <QDomElement>

class QDomDocument;
class BattleDialogModel;
class AudioTrack;

class EncounterBattle : public EncounterText
{
    Q_OBJECT
public:

    explicit EncounterBattle(const QString& encounterName = QString(), QObject *parent = nullptr);
    virtual ~EncounterBattle() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;

    virtual int getObjectType() const override;
    virtual QIcon getDefaultIcon() override;

    virtual bool hasData() const;

    AudioTrack* getAudioTrack();
    QUuid getAudioTrackId();
    void setAudioTrack(AudioTrack* track);

    void createBattleDialogModel();
    void setBattleDialogModel(BattleDialogModel* model);
    BattleDialogModel* getBattleDialogModel() const;
    void removeBattleDialogModel();

    void inputXMLBattle(const QDomElement &element, bool isImport);

protected:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;
    virtual void internalPostProcessXML(const QDomElement &element, bool isImport) override;

    BattleDialogModel* createNewBattle(QPointF combatantPos);

    void connectFrameToModel();
    void disconnectFrameFromModel();

    QUuid _audioTrackId;
    BattleDialogModel* _battleModel;

};

#endif // ENCOUNTERBATTLE_H
