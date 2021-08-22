#include "mapmarkerdialog.h"
#include "ui_mapmarkerdialog.h"
#include "campaign.h"
#include "character.h"
#include "audiotrack.h"

MapMarkerDialog::MapMarkerDialog(const QString& title, const QString& description, const QUuid& encounter, Map& map, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MapMarkerDialog)
{
    ui->setupUi(this);

    ui->edtTitle->setText(title);
    ui->txtDescription->setPlainText(description);

    populateCombo(encounter, map);
}

MapMarkerDialog::~MapMarkerDialog()
{
    delete ui;
}

QString MapMarkerDialog::getTitle() const
{
    return ui->edtTitle->text();
}

QString MapMarkerDialog::getDescription() const
{
    return ui->txtDescription->toPlainText();
}

QUuid MapMarkerDialog::getEncounter() const
{
    if(ui->cmbEncounter->currentIndex() == -1)
        return QUuid();
    else
        return QUuid(ui->cmbEncounter->currentData().toString());
}

void MapMarkerDialog::populateCombo(const QUuid& encounter, Map& map)
{
    Campaign* campaign = dynamic_cast<Campaign*>(map.getParentByType(DMHelper::CampaignType_Campaign));
    if(!campaign)
        return;

    QList<CampaignObjectBase*> childList = campaign->getChildObjects();
    for(CampaignObjectBase* childObject : childList)
    {
        addChildEntry(childObject, 0, encounter);
    }
}

void MapMarkerDialog::addChildEntry(CampaignObjectBase* object, int depth, const QUuid& encounter)
{
    if(!object)
        return;

    QString itemName;
    if(depth > 3)
        itemName.fill(' ', depth - 3);
    itemName.prepend("   ");
    if(depth > 0)
        itemName.append(QString("|--"));
    itemName.append(object->getName());
    ui->cmbEncounter->addItem(objectIcon(object), itemName, QVariant(object->getID().toString()));
    if(object->getID() == encounter)
        ui->cmbEncounter->setCurrentIndex(ui->cmbEncounter->count() - 1);

    QList<CampaignObjectBase*> childList = object->getChildObjects();
    for(CampaignObjectBase* childObject : childList)
    {
        addChildEntry(childObject, depth + 3, encounter);
    }
}

QIcon MapMarkerDialog::objectIcon(CampaignObjectBase* object)
{
    if(!object)
        return QIcon();

    switch(object->getObjectType())
    {
        case DMHelper::CampaignType_Party:
            return QIcon(":/img/data/icon_contentparty.png");
        case DMHelper::CampaignType_Combatant:
            {
                Character* character = dynamic_cast<Character*>(object);
                return ((character) && (character->isInParty())) ? QIcon(":/img/data/icon_contentcharacter.png") : QIcon(":/img/data/icon_contentnpc.png");
            }
        case DMHelper::CampaignType_Map:
            return QIcon(":/img/data/icon_contentmap.png");
        case DMHelper::CampaignType_Text:
            return QIcon(":/img/data/icon_contenttextencounter.png");
        case DMHelper::CampaignType_Battle:
            return QIcon(":/img/data/icon_contentbattle.png");
        case DMHelper::CampaignType_ScrollingText:
            return QIcon(":/img/data/icon_contentscrollingtext.png");
        case DMHelper::CampaignType_AudioTrack:
            {
                AudioTrack* track = dynamic_cast<AudioTrack*>(object);
                if((track) && (track->getAudioType() == DMHelper::AudioType_Syrinscape))
                    return QIcon(":/img/data/icon_syrinscape.png");
                else if((track) && (track->getAudioType() == DMHelper::AudioType_Youtube))
                    return QIcon(":/img/data/icon_playerswindow.png");
                else
                    return QIcon(":/img/data/icon_soundboard.png");
            }
        default:
            return QIcon();
    }
}
