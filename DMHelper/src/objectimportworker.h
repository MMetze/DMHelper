#ifndef OBJECTIMPORTWORKER_H
#define OBJECTIMPORTWORKER_H

#include <QObject>
#include <QUuid>
#include <QStringList>
#include <QFileInfo>
#include <QDir>

class Campaign;
class QStandardItem;
class QDomElement;
class CampaignObjectBase;
class EncounterBattle;

class ObjectImportWorker : public QObject
{
    Q_OBJECT
public:
    explicit ObjectImportWorker(QObject *parent = nullptr);
    virtual ~ObjectImportWorker() override;

    bool setImportObject(Campaign* campaign, CampaignObjectBase* parentObject, const QString& importFilename, const QString& assetPath, bool replaceDuplicates);

public slots:
    bool doWork();

signals:
    void workComplete(bool success, const QString& error);
    void updateStatus(const QString& status, const QString& statusSuffix);

protected:
    void importObject(CampaignObjectBase* object, CampaignObjectBase* parentObject, Campaign& targetCampaign, Campaign& importCampaign);
    void importObjectAssets(CampaignObjectBase* object);
    QString importFile(const QString& filename);
    void importBattle(EncounterBattle* battle);
    bool registerImportResult(bool success, const QString& debugString, const QString& userMessage = QString());
    void postProcessBattles(const QDomElement& campaignElement);
    QDomElement findBattleElement(const QDomElement& element, const QString& idString);

    Campaign* _campaign;
    CampaignObjectBase* _parentObject;
    QString _importFilename;
    QString _assetPath;
    QDir _assetDir;
    bool _replaceDuplicates;

    Campaign* _importCampaign;
    QList<CampaignObjectBase*> _duplicateObjects;
    QList<EncounterBattle*> _importedBattles;

};

#endif // OBJECTIMPORTWORKER_H
