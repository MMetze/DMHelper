#include "mapselectdialog.h"
#include "ui_mapselectdialog.h"
#include "map.h"

MapSelectDialog::MapSelectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MapSelectDialog)
{
    ui->setupUi(this);

    connect(ui->lstMaps, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(handleItemChanged(QListWidgetItem*,QListWidgetItem*)));
}

MapSelectDialog::~MapSelectDialog()
{
    delete ui;
}

void MapSelectDialog::addMap(Map* newMap)
{
    if(!newMap)
        return;

    QListWidgetItem* newItem = new QListWidgetItem();
    newItem->setText(newMap->getName());
    newItem->setData(Qt::UserRole, QVariant::fromValue(newMap));
    ui->lstMaps->addItem(newItem);
}

Map* MapSelectDialog::getSelectedMap() const
{
    QListWidgetItem* currentItem = ui->lstMaps->currentItem();
    if(!currentItem)
        return nullptr;

    return currentItem->data(Qt::UserRole).value<Map*>();
}

void MapSelectDialog::handleItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);

    QImage img;

    if(current)
    {
        Map* map = current->data(Qt::UserRole).value<Map*>();
        if(map)
            img = map->getPreviewImage();
    }

    ui->lblPreview->setPixmap(QPixmap::fromImage(img).scaled(ui->lblPreview->size(),Qt::KeepAspectRatio));
}
