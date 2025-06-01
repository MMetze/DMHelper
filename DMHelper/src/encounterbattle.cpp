#include "encounterbattle.h"
#include "dmconstants.h"
#include "battleframe.h"
#include "combatantfactory.h"
#include "campaign.h"
#include "bestiary.h"
#include "map.h"
#include "monster.h"
#include "battledialogmodel.h"
#include "battledialogmodeleffectfactory.h"
#include "audiotrack.h"
#include "encounterfactory.h"
#include <QDomDocument>
#include <QDomElement>
#include <QDomCDATASection>
#include <QTextDocument>
#include <QIcon>
#include <QDebug>

EncounterBattle::EncounterBattle(const QString& encounterName, QObject *parent) :
    EncounterText(encounterName, parent),
    _audioTrackId(),
    _battleModel(nullptr)
{
}

EncounterBattle::~EncounterBattle()
{
}

void EncounterBattle::inputXML(const QDomElement &element, bool isImport)
{
    extractTextNode(element, isImport);
    if(!getText().isEmpty())
    {
        QString battleName = element.attribute(QString("name"), QString("Battle Text"));
        CampaignObjectBase* encounter = EncounterFactory().createObject(DMHelper::CampaignType_Text, -1, battleName, isImport);
        if(encounter)
        {
            EncounterText* textEncounter = dynamic_cast<EncounterText*>(encounter);
            if(textEncounter)
            {
                textEncounter->setText(getText());
                addObject(textEncounter);
                setText(QString());
            }
            else
            {
                delete encounter;
            }
        }
    }

    CampaignObjectBase::inputXML(element, isImport);
}

void EncounterBattle::copyValues(const CampaignObjectBase* other)
{
    const EncounterBattle* otherBattle = dynamic_cast<const EncounterBattle*>(other);
    if(!otherBattle)
        return;

    _audioTrackId = otherBattle->_audioTrackId;

    EncounterText::copyValues(other);
}

int EncounterBattle::getObjectType() const
{
    return DMHelper::CampaignType_Battle;
}

QIcon EncounterBattle::getDefaultIcon()
{
    return QIcon(":/img/data/icon_contentbattle.png");
}

bool EncounterBattle::hasData() const
{
    return _battleModel != nullptr;
}

AudioTrack* EncounterBattle::getAudioTrack()
{
    Campaign* campaign = dynamic_cast<Campaign*>(getParentByType(DMHelper::CampaignType_Campaign));
    if(!campaign)
        return nullptr;

    return campaign->getTrackById(_audioTrackId);
}

QUuid EncounterBattle::getAudioTrackId()
{
    return _audioTrackId;
}

void EncounterBattle::setAudioTrack(AudioTrack* track)
{
    QUuid newTrackId = (track == nullptr) ? QUuid() : track->getID();
    if(_audioTrackId != newTrackId)
    {
        _audioTrackId = newTrackId;
        emit dirty();
    }
}

void EncounterBattle::createBattleDialogModel()
{
    qDebug() << "[EncounterBattle] Creating new battle model for encounter " << getID();
    setBattleDialogModel(createNewBattle(QPointF(0.0, 0.0)));
}

void EncounterBattle::setBattleDialogModel(BattleDialogModel* model)
{
    qDebug() << "[EncounterBattle] Setting new battle model to " << model->getID() << " for encounter " << getID();

    if(_battleModel)
    {
        qDebug() << "[EncounterBattle] Setting new battle model, found old battle model.";
        removeBattleDialogModel();
    }

    _battleModel = model;

    emit changed();
    emit dirty();
}

BattleDialogModel* EncounterBattle::getBattleDialogModel() const
{
    return _battleModel;
}

void EncounterBattle::removeBattleDialogModel()
{
    if(!_battleModel)
        return;

    qDebug() << "[EncounterBattle] Removing battle model " << _battleModel->getID() << " from encounter " << getID();
    BattleDialogModel* battleModel = _battleModel;
    _battleModel = nullptr;
    delete battleModel;

    emit changed();
    emit dirty();
}

void EncounterBattle::inputXMLBattle(const QDomElement &element, bool isImport)
{
    if((_battleModel) || (isImport))
        return;

    Campaign* campaign = dynamic_cast<Campaign*>(getParentByType(DMHelper::CampaignType_Campaign));
    if(!campaign)
        return;

    QDomElement rootBattleElement = element.firstChildElement("battle");
    if(rootBattleElement.isNull())
    {
        _battleModel = createNewBattle(QPointF(0.0, 0.0));
        return;
    }

    _battleModel = new BattleDialogModel(this);
    _battleModel->inputXML(rootBattleElement, isImport);
}

QDomElement EncounterBattle::createOutputXML(QDomDocument &doc)
{
    return doc.createElement("battle-object");
}

void EncounterBattle::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("audiotrack", _audioTrackId.toString());

    if(_battleModel)
        _battleModel->outputXML(doc, element, targetDirectory, isExport);

    EncounterText::internalOutputXML(doc, element, targetDirectory, isExport);
}

bool EncounterBattle::belongsToObject(QDomElement& element)
{
    if((element.tagName() == QString("combatants")) || (element.tagName() == QString("waves")) || (element.tagName() == QString("battle")))
        return true;
    else
        return EncounterText::belongsToObject(element);
}

void EncounterBattle::internalPostProcessXML(const QDomElement &element, bool isImport)
{
    _audioTrackId = parseIdString(element.attribute("audiotrack"));

    // Read the battle
    if(!isImport)
        inputXMLBattle(element, isImport);

    EncounterText::internalPostProcessXML(element, isImport);
}

BattleDialogModel* EncounterBattle::createNewBattle(QPointF combatantPos)
{
    Q_UNUSED(combatantPos);

    BattleDialogModel* battleModel = new BattleDialogModel(this);

    return battleModel;
}
