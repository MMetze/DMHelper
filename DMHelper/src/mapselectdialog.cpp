#include "mapselectdialog.h"
#include "ui_mapselectdialog.h"
#include "map.h"
#include "campaign.h"
#include "encounterbattle.h"
#include "battledialogmodel.h"
#include "mapblankdialog.h"
#include "layerblank.h"
#include <QBrush>
#include <QPainter>

static bool isMapLikeBattleObject(CampaignObjectBase* object)
{
    if((!object) || (object->getObjectType() != DMHelper::CampaignType_Battle))
        return false;

    EncounterBattle* battle = dynamic_cast<EncounterBattle*>(object);
    BattleDialogModel* model = battle ? battle->getBattleDialogModel() : nullptr;
    return (model) && (model->getCombatantCount() == 0);
}

MapSelectDialog::MapSelectDialog(Campaign& campaign, const QUuid& currentId, bool includeMapLikeBattles, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MapSelectDialog),
    _currentItem(nullptr),
    _createBlankMap(nullptr),
    _loadNewMap(nullptr),
    _includeMapLikeBattles(includeMapLikeBattles),
    _blankMap(nullptr)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);
    connect(ui->lstMaps, &QTreeWidget::currentItemChanged, this, &MapSelectDialog::handleItemChanged);
    connect(ui->lstMaps, &QTreeWidget::itemDoubleClicked, this, &MapSelectDialog::accept);
    setupSelectTree(campaign, currentId);
}

MapSelectDialog::~MapSelectDialog()
{
    delete ui;
}

bool MapSelectDialog::isMapSelected() const
{
    QTreeWidgetItem* currentItem = ui->lstMaps->currentItem();
    return ((currentItem) && (currentItem != _createBlankMap) && (currentItem != _loadNewMap) &&
            (currentItem->data(0, Qt::UserRole).value<CampaignObjectBase*>() != nullptr));
}

CampaignObjectBase* MapSelectDialog::getSelectedObject() const
{
    QTreeWidgetItem* currentItem = ui->lstMaps->currentItem();
    if(!currentItem)
        return nullptr;

    return currentItem->data(0, Qt::UserRole).value<CampaignObjectBase*>();
}

Map* MapSelectDialog::getSelectedMap() const
{
    return dynamic_cast<Map*>(getSelectedObject());
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

    MapBlankDialog blankDlg(const_cast<MapSelectDialog*>(this));
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

    if(!current)
        return;

    QImage image;

    if((current == _loadNewMap) || (current == _createBlankMap))
    {
        // Show a generic blank map
        image = QImage(400, 300, QImage::Format_ARGB32);
        image.fill(Qt::white);
    }
    else
    {
        CampaignObjectBase* selectedObject = current->data(0, Qt::UserRole).value<CampaignObjectBase*>();
        Map* map = dynamic_cast<Map*>(selectedObject);
        if(map)
        {
            if(!map->isInitialized())
                map->initialize();

            image = map->getPreviewImage();
        }
        else if(selectedObject)
        {
            image = QImage(400, 300, QImage::Format_ARGB32);
            image.fill(Qt::white);

            QPainter painter(&image);
            QPixmap objectIcon = selectedObject->getIcon().pixmap(96, 96);
            if(!objectIcon.isNull())
            {
                QPoint iconPos((image.width() - objectIcon.width()) / 2,
                               (image.height() - objectIcon.height()) / 2);
                painter.drawPixmap(iconPos, objectIcon);
            }
        }
    }

    ui->lblPreview->setPixmap(image.isNull() ? QPixmap() : QPixmap::fromImage(image).scaled(ui->lblPreview->size(), Qt::KeepAspectRatio));
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
        if((_currentItem->parent()) && (_currentItem->parent()->data(0, Qt::UserRole).value<CampaignObjectBase*>() != nullptr))
        {
            ui->lstMaps->setCurrentItem(_currentItem->parent());
            return;
        }

        // Is there a child map?
        for(int i = 0; i < _currentItem->childCount(); ++i)
        {
            if((_currentItem->child(i)) && (_currentItem->child(i)->data(0, Qt::UserRole).value<CampaignObjectBase*>() != nullptr))
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

    bool hasSelectionCandidate = false;

    QTreeWidgetItem* newItem = new QTreeWidgetItem();
    newItem->setText(0, object->getName());
    decorateItem(newItem, object);
    if(object->getObjectType() == DMHelper::CampaignType_Map)
    {
        hasSelectionCandidate = true;
        newItem->setData(0, Qt::UserRole, QVariant::fromValue(dynamic_cast<CampaignObjectBase*>(object)));
    }
    else if((_includeMapLikeBattles) && isMapLikeBattleObject(object))
    {
        hasSelectionCandidate = true;
        newItem->setData(0, Qt::UserRole, QVariant::fromValue(dynamic_cast<CampaignObjectBase*>(object)));
    }
    else
    {
        newItem->setForeground(0, QBrush(Qt::gray));
    }

    if(object->getID() == currentId)
    {
        hasSelectionCandidate = true;
        _currentItem = newItem;
        _currentItem->setData(0, Qt::UserRole + 1, QVariant::fromValue(dynamic_cast<CampaignObjectBase*>(object)));
    }

    QList<CampaignObjectBase*> childObjects = object->getChildObjects();
    for(CampaignObjectBase* childObject : childObjects)
    {
        if(insertObject(childObject, newItem, currentId))
            hasSelectionCandidate = true;
    }

    if(hasSelectionCandidate)
        parentItem->addChild(newItem);
    else
        delete newItem;

    return hasSelectionCandidate;
}

void MapSelectDialog::decorateItem(QTreeWidgetItem* item, CampaignObjectBase* object)
{
    if((!item) || (!object))
        return;

    item->setIcon(0, object->getIcon());
}
