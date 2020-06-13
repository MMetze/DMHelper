#ifndef CAMPAIGNEXPORTER_H
#define CAMPAIGNEXPORTER_H

#include "campaign.h"
#include "encountertext.h"
#include "encounterbattle.h"
#include "encounterscrollingtext.h"
#include <QDomDocument>
#include <QDir>
#include <QUuid>

class CampaignExporter
{
public:
    CampaignExporter(Campaign& originalCampaign, QUuid exportId, QDir& exportDirectory);
    virtual ~CampaignExporter();

    Campaign& getExportCampaign();
    QDomDocument& getExportDocument();
    bool isValid() const;

    CampaignExporter(CampaignExporter const &) = delete;
    void operator=(CampaignExporter const &exporter) = delete;

protected:
    bool populateExport();

    bool addObjectTree(QUuid exportId, QDomDocument &doc, QDomElement &parent, QDir& targetDirectory);
    bool checkObjectTreeReferences(CampaignObjectBase* exportObject, QDomDocument &doc, QDomElement &parent, QDir& targetDirectory);
    bool checkObjectReferences(CampaignObjectBase* exportObject, QDomDocument &doc, QDomElement &parent, QDir& targetDirectory);
    void addObjectAndChildrenIds(CampaignObjectBase* object);

    /*
    void addWholeAdventure(Adventure& adventure);
    Adventure* addShellAdventure(Adventure& adventure);
    void addEncounterText(Adventure& adventure, EncounterText& encounter);
    void addEncounterBattle(Adventure& adventure, EncounterBattle& encounter);
    void addEncounterScrollingText(Adventure& adventure, EncounterScrollingText& encounter);
    void addMap(Adventure& adventure, Map& map);
    */

    Campaign& _originalCampaign;
    QUuid _exportId;
    QDir& _exportDirectory;
    Campaign* _exportCampaign;
    QDomDocument* _exportDocument;
    QList<QUuid> _exportedIds;

    bool _valid;
};

#endif // CAMPAIGNEXPORTER_H
