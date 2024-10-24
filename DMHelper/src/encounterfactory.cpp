#include "encounterfactory.h"
#include "encountertext.h"
#include "encountertextlinked.h"
#include "encounterbattle.h"
#include "party.h"
#include "ruleset.h"
#include "dmconstants.h"
#include <QDomElement>
#include <QString>

EncounterFactory::EncounterFactory(QObject *parent) :
    ObjectFactory(parent)
{
}

CampaignObjectBase* EncounterFactory::createObject(int objectType, int subType, const QString& objectName, bool isImport)
{
    Q_UNUSED(isImport);
    Q_UNUSED(subType);

    switch(objectType)
    {
        case DMHelper::CampaignType_Text:
        case DMHelper::CampaignType_ScrollingText:
            return new EncounterText(objectName);
        case DMHelper::CampaignType_LinkedText:
            return new EncounterTextLinked(objectName);
        case DMHelper::CampaignType_Battle:
            return new EncounterBattle(objectName);
        case DMHelper::CampaignType_Party:
            return new Party(objectName);
        default:
            return nullptr;
    }
}

CampaignObjectBase* EncounterFactory::createObject(const QDomElement& element, bool isImport)
{
    Q_UNUSED(isImport);

    if(element.tagName() == QString("encounter"))
    {
        // Compatibility mode
        bool ok = false;
        int encounterType = element.attribute("type").toInt(&ok);
        if(ok)
        {
            switch(encounterType)
            {
                case ObjectFactory::EncounterType_Text:
                case ObjectFactory::EncounterType_ScrollingText:
                        return new EncounterText();
                case ObjectFactory::EncounterType_Battle:
                    return new EncounterBattle();
                default:
                    break;
            }
        }
    }
    else if(element.tagName() == QString("text-object")) // for backwards-compatibility only, see entry-object
    {
        return new EncounterText();
    }
    else if(element.tagName() == QString("entry-object"))
    {
        return new EncounterText();
    }
    else if(element.tagName() == QString("linked-object"))
    {
        return new EncounterTextLinked();
    }
    else if(element.tagName() == QString("battle-object"))
    {
        return new EncounterBattle();
    }
    else if(element.tagName() == QString("scrolling-object")) // for backwards-compatibility only, see entry-object
    {
        return new EncounterText();
    }
    else if((element.tagName() == QString("characters")) ||
            (element.tagName() == QString("party")))
    {
        return new Party(QString("Party"));
    }
    else if((element.tagName() == QString("notes")) ||
            (element.tagName() == QString("adventures")) ||
            (element.tagName() == QString("adventure")) ||
            (element.tagName() == QString("encounters")) ||
            (element.tagName() == QString("maps")) ||
            (element.tagName() == QString("settings")) ||
            (element.tagName() == QString("npcs")) ||
            (element.tagName() == QString("tracks")))
    {
        // Compatibility mode
        QString encounterName = element.tagName();
        encounterName.replace(0, 1, encounterName.at(0).toUpper());
        return new EncounterText(encounterName);
    }

    return nullptr;
}

void EncounterFactory::configureFactory(const Ruleset& ruleset, int inputMajorVersion, int inputMinorVersion)
{
    Q_UNUSED(ruleset);
    Q_UNUSED(inputMajorVersion);
    Q_UNUSED(inputMinorVersion);
}
