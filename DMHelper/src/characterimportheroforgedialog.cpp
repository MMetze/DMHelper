#include "characterimportheroforgedialog.h"
#include "ui_characterimportheroforgedialog.h"
#include "characterimportheroforge.h"
#include "characterimportheroforgedata.h"
#include <QPushButton>

/*
#include <QFile>
#include <QFileInfo>
#include <QDebug>
*/

const int ICON_SPACING = 8;

CharacterImportHeroForgeDialog::CharacterImportHeroForgeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CharacterImportHeroForgeDialog),
    _iconGrid(nullptr),
    _buttonGroup(),
    _importer(nullptr)
{
    ui->setupUi(this);

    _iconGrid = new QGridLayout;
    _iconGrid->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    _iconGrid->setContentsMargins(ICON_SPACING, ICON_SPACING, ICON_SPACING, ICON_SPACING);
    _iconGrid->setSpacing(ICON_SPACING);
    ui->scrollAreaWidgetContents->setLayout(_iconGrid);
}

CharacterImportHeroForgeDialog::~CharacterImportHeroForgeDialog()
{
    delete ui;
}

void CharacterImportHeroForgeDialog::showEvent(QShowEvent *event)
{
    if(!_importer)
    {
        _importer = new CharacterImportHeroForge(this);
        connect(_importer, &CharacterImportHeroForge::importComplete, this, &CharacterImportHeroForgeDialog::importComplete);
        _importer->runImport();
    }

    QDialog::showEvent(event);
}

void CharacterImportHeroForgeDialog::importComplete(QList<CharacterImportHeroForgeData*> data)
{
    for(int i = 0; i < data.count(); ++i)
    {
        CharacterImportHeroForgeData* singleData = data.at(i);
        if(singleData)
        {
            connect(singleData, &CharacterImportHeroForgeData::dataReady, this, &CharacterImportHeroForgeDialog::dataReady);
            singleData->getData();
        }
    }
}

void CharacterImportHeroForgeDialog::dataReady(CharacterImportHeroForgeData* data)
{
    int newRow = _iconGrid->rowCount();
    addDataRow(data, newRow);

    if(_iconGrid->rowCount() == 1)
    {
        int spacingColumn = _iconGrid->columnCount();
        _iconGrid->addItem(new QSpacerItem(20, 40, QSizePolicy::Expanding), 0, spacingColumn);
        _iconGrid->setColumnStretch(spacingColumn, 10);
    }

    //for(int i = 0; i < spacingColumn; ++i)
    //    _iconGrid->setColumnStretch(i, 1);
    _iconGrid->setColumnStretch(newRow, 1);

    update();
}

void CharacterImportHeroForgeDialog::addDataRow(CharacterImportHeroForgeData* data, int row)
{
    if(!data)
        return;

    /*
    data->getThumbImage().save(data->getName() + QString("_thumb.png"));
    QFile meshFile(data->getName() + QString("_mesh.txt"));
    if(meshFile.open(QIODevice::WriteOnly))
    {
        qDebug() << "[CharacterImportHeroForgeDialog] INFO: MESH FILE: " << QFileInfo(meshFile).absoluteFilePath();
        meshFile.write(data->getMesh().toUtf8());
        meshFile.close();
    }
    QFile metaFile(data->getName() + QString("_meta.txt"));
    if(metaFile.open(QIODevice::WriteOnly))
    {
        qDebug() << "[CharacterImportHeroForgeDialog] INFO: META FILE: " << QFileInfo(metaFile).absoluteFilePath();
        metaFile.write(data->getMeta().toUtf8());
        metaFile.close();
    }
    */

    addData(data->getThumbImage(), data->getName(), row, 0);
    addData(data->getPerspectiveImage(), data->getName(), row, 1);
    addData(data->getFrontImage(), data->getName(), row, 2);
    addData(data->getTopImage(), data->getName(), row, 3);
}

void CharacterImportHeroForgeDialog::addData(QImage image, QString dataName, int row, int column)
{
    if(image.isNull())
        return;

    /*
    QLabel* iconLabel = new QLabel(this);
    iconLabel->setPixmap(QPixmap::fromImage(image).scaled(40, 40));
    iconLabel->setToolTip(dataName);
    _iconGrid->addWidget(iconLabel, row, column);
    */

    QPushButton* iconButton = new QPushButton(this);
    iconButton->setToolTip(dataName);
    iconButton->setCheckable(true);
    iconButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    iconButton->setMinimumSize(image.size());
    iconButton->setMaximumSize(image.size());
    iconButton->setIconSize(image.size());
    iconButton->setIcon(QIcon(QPixmap::fromImage(image)));
    _buttonGroup.addButton(iconButton);
    _iconGrid->addWidget(iconButton, row, column);
}
