#include "mapselectdialog.h"
#include "ui_mapselectdialog.h"
#include "map.h"
#include "character.h"
#include "audiotrack.h"
#include "campaign.h"
#include "mapfactory.h"
#include "mapblankdialog.h"
#include "layerblank.h"
#include <QBrush>
#include <QMessageBox>

MapSelectDialog::MapSelectDialog(Campaign& campaign, const QUuid& currentId, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MapSelectDialog),
    _currentItem(nullptr),
    _createBlankMap(nullptr),
    _loadNewMap(nullptr),
    _blankMap(nullptr)
{
    ui->setupUi(this);
    connect(ui->lstMaps, &QTreeWidget::currentItemChanged, this, &MapSelectDialog::handleItemChanged);
    setupSelectTree(campaign, currentId);
}

MapSelectDialog::~MapSelectDialog()
{
    delete ui;
}

bool MapSelectDialog::isMapSelected() const
{
    QTreeWidgetItem* currentItem = ui->lstMaps->currentItem();
    return ((currentItem) && (currentItem != _createBlankMap) && (currentItem != _loadNewMap));
}

Map* MapSelectDialog::getSelectedMap() const
{
    QTreeWidgetItem* currentItem = ui->lstMaps->currentItem();
    if(!currentItem)
        return nullptr;

    return currentItem->data(0, Qt::UserRole).value<Map*>();
}

bool MapSelectDialog::isBlankMap() const
{
    QTreeWidgetItem* currentItem = ui->lstMaps->currentItem();
    return ((currentItem) && (currentItem == _createBlankMap));
}

Layer* MapSelectDialog::getBlankLayer() const
{
    if(!isBlankMap())
        return nullptr;

    MapBlankDialog blankDlg;
    int result = blankDlg.exec();
    if(result != QDialog::Accepted)
        return nullptr;

    LayerBlank* newLayer = new LayerBlank(QString("Blank Layer"), blankDlg.getMapColor());
    newLayer->setSize(blankDlg.getMapSize());
    return newLayer;
}

bool MapSelectDialog::isNewMapImage() const
{
    QTreeWidgetItem* currentItem = ui->lstMaps->currentItem();
    return ((currentItem) && (currentItem == _loadNewMap));
}

Layer* MapSelectDialog::getNewImageLayer() const
{
    return nullptr;
}

/*
void MapSelectDialog::accept()
{
    QTreeWidgetItem* currentItem = ui->lstMaps->currentItem();
    if((currentItem) && (currentItem == _createNewMapEntry))
        createBlankMap();

    QDialog::accept();
}
*/

void MapSelectDialog::handleItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous);

    QImage img;

    if(current)
    {
        Map* map = current->data(0, Qt::UserRole).value<Map*>();
        if(map)
        {
            if(!map->isInitialized())
                map->initialize();

            img = map->getPreviewImage();
        }
    }

    ui->lblPreview->setPixmap(QPixmap::fromImage(img).scaled(ui->lblPreview->size(), Qt::KeepAspectRatio));
}

void MapSelectDialog::setupSelectTree(Campaign& campaign, const QUuid& currentId)
{
    ui->lstMaps->clear();

    _currentItem = nullptr;

    _loadNewMap = new QTreeWidgetItem();
    _loadNewMap->setText(0, QString("Load New Map Image..."));
    ui->lstMaps->invisibleRootItem()->addChild(_loadNewMap);

    _createBlankMap = new QTreeWidgetItem();
    _createBlankMap->setText(0, QString("Create Blank Map..."));
    ui->lstMaps->invisibleRootItem()->addChild(_createBlankMap);

    QList<CampaignObjectBase*> campaignObjects = campaign.getChildObjects();
    for(CampaignObjectBase* object : campaignObjects)
    {
        insertObject(object, ui->lstMaps->invisibleRootItem(), currentId);
    }

    ui->lstMaps->expandAll();
    // Find the best guess of a map, either parent or first map child
    if(_currentItem)
    {
        // Is the parent a map?
        if((_currentItem->parent()) && (_currentItem->parent()->data(0, Qt::UserRole).value<Map*>() != nullptr))
        {
            ui->lstMaps->setCurrentItem(_currentItem->parent());
            return;
        }

        // Is there a child map?
        for(int i = 0; i < _currentItem->childCount(); ++i)
        {
            if((_currentItem->child(i)) && (_currentItem->child(i)->data(0, Qt::UserRole).value<Map*>() != nullptr))
            {
                ui->lstMaps->setCurrentItem(_currentItem->child(i));
                return;
            }
        }

        // Just select this item
        ui->lstMaps->setCurrentItem(_currentItem);
    }
}

bool MapSelectDialog::insertObject(CampaignObjectBase* object, QTreeWidgetItem* parentItem, const QUuid& currentId)
{
    if((!object) || (!parentItem))
        return false;

    bool hasMap = false;

    QTreeWidgetItem* newItem = new QTreeWidgetItem();
    newItem->setText(0, object->getName());
    decorateItem(newItem, object);
    if(object->getObjectType() == DMHelper::CampaignType_Map)
    {
        hasMap = true;
        newItem->setData(0, Qt::UserRole, QVariant::fromValue(dynamic_cast<Map*>(object)));
    }
    else
    {
        newItem->setForeground(0, QBrush(Qt::gray));
    }

    if(object->getID() == currentId)
    {
        hasMap = true;
        _currentItem = newItem;
        _currentItem->setData(0, Qt::UserRole + 1, QVariant::fromValue(dynamic_cast<CampaignObjectBase*>(object)));
    }

    QList<CampaignObjectBase*> childObjects = object->getChildObjects();
    for(CampaignObjectBase* childObject : childObjects)
    {
        if(insertObject(childObject, newItem, currentId))
            hasMap = true;
    }

    if(hasMap)
        parentItem->addChild(newItem);
    else
        delete newItem;

    return hasMap;
}

void MapSelectDialog::decorateItem(QTreeWidgetItem* item, CampaignObjectBase* object)
{
    if((!item) || (!object))
        return;

    item->setIcon(0, object->getIcon());

    /*
    switch(object->getObjectType())
    {
        case DMHelper::CampaignType_Party:
            item->setIcon(0, QIcon(":/img/data/icon_contentparty.png"));
            break;
        case DMHelper::CampaignType_Combatant:
            {
                Character* character = dynamic_cast<Character*>(object);
                bool isPC = ((character) && (character->isInParty()));
                item->setIcon(0, isPC ? QIcon(":/img/data/icon_contentcharacter.png") : QIcon(":/img/data/icon_contentnpc.png"));
            }
            break;
        case DMHelper::CampaignType_Map:
            item->setIcon(0, QIcon(":/img/data/icon_contentmap.png"));
            break;
        case DMHelper::CampaignType_Text:
        case DMHelper::CampaignType_LinkedText:
            item->setIcon(0, QIcon(":/img/data/icon_contenttextencounter.png"));
            break;
        case DMHelper::CampaignType_Battle:
            item->setIcon(0, QIcon(":/img/data/icon_contentbattle.png"));
            break;
        case DMHelper::CampaignType_ScrollingText:
            item->setIcon(0, QIcon(":/img/data/icon_contentscrollingtext.png"));
            break;
        case DMHelper::CampaignType_AudioTrack:
            {
                QString audioIcon(":/img/data/icon_soundboard.png");
                AudioTrack* track = dynamic_cast<AudioTrack*>(object);
                if(track)
                {
                    if(track->getAudioType() == DMHelper::AudioType_Syrinscape)
                        audioIcon = QString(":/img/data/icon_syrinscape.png");
                    else if(track->getAudioType() == DMHelper::AudioType_Youtube)
                        audioIcon = QString(":/img/data/icon_playerswindow.png");
                }
                item->setIcon(0, QIcon(audioIcon));
            }
            break;
        default:
            break;
    }
    */
}
