#include "encounterfactory.h"
#include "encountertext.h"
#include "encounterbattle.h"
#include "encounterscrollingtext.h"
#include "party.h"
#include "dmconstants.h"
#include <QDomElement>
#include <QString>

EncounterFactory::EncounterFactory(QObject *parent) :
    ObjectFactory(parent)
{
}

CampaignObjectBase* EncounterFactory::createObject(int objectType, int subType, const QString& objectName, bool isImport)
//Encounter* EncounterFactory::createEncounter(int encounterType, const QString& encounterName, QObject *parent)
{
    Q_UNUSED(isImport);
    Q_UNUSED(subType);

    switch(objectType)
    {
        case DMHelper::CampaignType_Text:
            return new EncounterText(objectName);
        case DMHelper::CampaignType_Battle:
            return new EncounterBattle(objectName);
        case DMHelper::CampaignType_ScrollingText:
            return new EncounterScrollingText(objectName);
        case DMHelper::CampaignType_Party:
            return new Party(objectName);
        default:
            return nullptr;
    }
}

CampaignObjectBase* EncounterFactory::createObject(const QDomElement& element, bool isImport)
//Encounter* EncounterFactory::createEncounter(int encounterType, const QDomElement& element, bool isImport, QObject *parent)
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
                    return new EncounterText();
                case ObjectFactory::EncounterType_Battle:
                    return new EncounterBattle();
                case ObjectFactory::EncounterType_ScrollingText:
                    return new EncounterScrollingText();
                default:
                    break;
            }
        }
    }
    else if(element.tagName() == QString("text-object"))
    {
        return new EncounterText();
    }
    else if(element.tagName() == QString("battle-object"))
    {
        return new EncounterBattle();
    }
    else if(element.tagName() == QString("scrolling-object"))
    {
        return new EncounterScrollingText();
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

/*
Encounter* EncounterFactory::cloneEncounter(Encounter& encounter)
{
    Encounter* clone = nullptr;

    switch(encounter.getType())
    {
        case DMHelper::EncounterType_Blank:
            break;
        case DMHelper::EncounterType_Text:
            {
                EncounterText* encounterText = dynamic_cast<EncounterText*>(&encounter);
                if(encounterText)
                    clone = new EncounterText(*encounterText);
            }
            break;
        case DMHelper::EncounterType_Battle:
            {
                EncounterBattle* encounterBattle = dynamic_cast<EncounterBattle*>(&encounter);
                if(encounterBattle)
                    clone = new EncounterBattle(*encounterBattle);
            }
            break;
        case DMHelper::EncounterType_ScrollingText:
            {
                EncounterScrollingText* encounterScrollingText = dynamic_cast<EncounterScrollingText*>(&encounter);
                if(encounterScrollingText)
                    clone = new EncounterScrollingText(*encounterScrollingText);
            }
            break;
        default:
            break;
    }

    return clone;
}
*/
