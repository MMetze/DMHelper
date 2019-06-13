#include "encounterfactory.h"
#include "encountertext.h"
#include "encounterbattle.h"
#include "encounterscrollingtext.h"
#include "dmconstants.h"
#include <QDomElement>
#include <QString>

EncounterFactory::EncounterFactory(QObject *parent) :
    QObject(parent)
{
}

Encounter* EncounterFactory::createEncounter(int encounterType, const QString& encounterName, QObject *parent)
{
    switch(encounterType)
    {
        case DMHelper::EncounterType_Blank:
            return nullptr;
        case DMHelper::EncounterType_Text:
            return new EncounterText(encounterName, parent);
        case DMHelper::EncounterType_Battle:
            return new EncounterBattle(encounterName, parent);
        case DMHelper::EncounterType_ScrollingText:
            return new EncounterScrollingText(encounterName, parent);
        default:
            return nullptr;
    }
}

Encounter* EncounterFactory::createEncounter(int encounterType, const QDomElement& element, bool isImport, QObject *parent)
{
    Encounter* encounter = nullptr;

    switch(encounterType)
    {
        case DMHelper::EncounterType_Blank:
            break;
        case DMHelper::EncounterType_Text:
            encounter = new EncounterText(QString(""), parent);
            break;
        case DMHelper::EncounterType_Battle:
            encounter = new EncounterBattle(QString(""), parent);
            break;
        case DMHelper::EncounterType_ScrollingText:
            encounter = new EncounterScrollingText(QString(""), parent);
            break;
        default:
            break;
    }

    if(encounter)
        encounter->inputXML(element, isImport);

    return encounter;
}

