#ifndef ENCOUNTERBATTLEDOWNLOAD_H
#define ENCOUNTERBATTLEDOWNLOAD_H

#include "encounterbattle.h"

class BattleDialogModelEffect;
class BattleDialogModelCombatantDownload;

class EncounterBattleDownload : public EncounterBattle
{
    Q_OBJECT
public:
    explicit EncounterBattleDownload(const QString& cacheDirectory, QObject *parent = nullptr);

    virtual void inputXML(const QDomElement &element, bool isImport) override;
    void updateXML(const QDomElement &element);

    bool isComplete();

public slots:
    virtual void fileReceived(const QString& md5, const QString& uuid, const QByteArray& data);

signals:
    void requestFile(const QString& md5, const QString& uuid, int fileType);
    void encounterComplete();

protected:
    virtual void inputXMLBattle(const QDomElement &element, bool isImport) override;
    virtual void inputXMLEffects(const QDomElement &parentElement, bool isImport) override;
    BattleDialogModelEffect* createEffect(const QDomElement& element, bool isImport);
    void checkComplete();
    BattleDialogModelCombatantDownload* findCombatantById(QUuid combatantId);
    BattleDialogModelEffect* findEffectById(QUuid effectId);

    QString _cacheDirectory;
};

#endif // ENCOUNTERBATTLEDOWNLOAD_H