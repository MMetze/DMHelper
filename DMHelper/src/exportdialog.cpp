#include "exportdialog.h"
#include "ui_exportdialog.h"
#include "campaign.h"
#include "dmconstants.h"
#include "character.h"
#include "audiotrack.h"
#include <QUuid>
#include <QDebug>

ExportDialog::ExportDialog(const Campaign& campaign, const QUuid& selectedItem, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportDialog),
    _campaign(campaign),
    _selectedItem(selectedItem)
{
    ui->setupUi(this);
    connect(ui->treeWidget, &QTreeWidget::itemChanged, this, &ExportDialog::handleItemChanged);

    QList<CampaignObjectBase*> objectList = campaign.getChildObjects();
    for(CampaignObjectBase* rootObject : objectList)
    {
        if(rootObject)
        {
            QTreeWidgetItem* rootWidgetItem = createChildObject(rootObject, selectedItem);
            if(rootWidgetItem)
            {
                ui->treeWidget->addTopLevelItem(rootWidgetItem);
                rootWidgetItem->setExpanded(true);
            }
        }
    }
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

QTreeWidgetItem* ExportDialog::createChildObject(const CampaignObjectBase* childObject, const QUuid& selectedItem)
{
    if(!childObject)
        return nullptr;

    QTreeWidgetItem* childItem = new QTreeWidgetItem(QStringList(childObject->objectName()));
    setObjectIcon(childObject, childItem);
    if(!childItem)
        return nullptr;

    QList<CampaignObjectBase*> objectList = childObject->getChildObjects();
    for(CampaignObjectBase* nextObject : objectList)
    {
        if(nextObject)
        {
            QTreeWidgetItem* nextWidgetItem = createChildObject(nextObject, selectedItem);
            if(nextWidgetItem)
                childItem->addChild(nextWidgetItem);
        }
    }

    childItem->setExpanded(true);
    childItem->setFlags(childItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);

    setRecursiveChecked(childItem, childObject->getID() == selectedItem);

    return childItem;
}

void ExportDialog::handleItemChanged(QTreeWidgetItem *item, int column)
{
    if((!item) || (column > 0))
        return;

    setRecursiveChecked(item, item->checkState(0) == Qt::Checked);
}

void ExportDialog::setRecursiveChecked(QTreeWidgetItem *item, bool checked)
{
    if(!item)
        return;

    item->setCheckState(0, checked ? Qt::Checked : Qt::Unchecked);
    for(int i = 0; i < item->childCount(); ++i)
        setRecursiveChecked(item->child(i), checked);
}

void ExportDialog::setObjectIcon(const CampaignObjectBase* baseObject, QTreeWidgetItem* widgetItem)
{
    switch(baseObject->getObjectType())
    {
        case DMHelper::CampaignType_Party:
            widgetItem->setIcon(0, QIcon(":/img/data/icon_contentparty.png"));
            break;
        case DMHelper::CampaignType_Combatant:
            {
                const Character* character = dynamic_cast<const Character*>(baseObject);
                bool isPC = ((character) && (character->isInParty()));
                widgetItem->setIcon(0, isPC ? QIcon(":/img/data/icon_contentcharacter.png") : QIcon(":/img/data/icon_contentnpc.png"));
            }
            break;
        case DMHelper::CampaignType_Map:
            widgetItem->setIcon(0, QIcon(":/img/data/icon_contentmap.png"));
            break;
        case DMHelper::CampaignType_Text:
            widgetItem->setIcon(0, QIcon(":/img/data/icon_contenttextencounter.png"));
            break;
        case DMHelper::CampaignType_Battle:
            widgetItem->setIcon(0, QIcon(":/img/data/icon_contentbattle.png"));
            break;
        case DMHelper::CampaignType_ScrollingText:
            widgetItem->setIcon(0, QIcon(":/img/data/icon_contentscrollingtext.png"));
            break;
        case DMHelper::CampaignType_AudioTrack:
            {
                QString audioIcon(":/img/data/icon_soundboard.png");
                const AudioTrack* track = dynamic_cast<const AudioTrack*>(baseObject);
                if(track)
                {
                    if(track->getAudioType() == DMHelper::AudioType_Syrinscape)
                        audioIcon = QString(":/img/data/icon_syrinscape.png");
                    else if(track->getAudioType() == DMHelper::AudioType_Youtube)
                        audioIcon = QString(":/img/data/icon_playerswindow.png");
                }
                widgetItem->setIcon(0, QIcon(audioIcon));
            }
            break;
    }

}
