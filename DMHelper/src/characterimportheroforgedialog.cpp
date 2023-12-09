#include "characterimportheroforgedialog.h"
#include "ui_characterimportheroforgedialog.h"
#include "characterimportheroforge.h"
#include "characterimportheroforgedata.h"
#include "dmhwaitingdialog.h"
#include <QDebug>

/*
#include <QFile>
#include <QFileInfo>
*/

const int ICON_SPACING = 8;

CharacterImportHeroForgeDialog::CharacterImportHeroForgeDialog(const QString& token, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CharacterImportHeroForgeDialog),
    _iconGrid(nullptr),
    _buttonGroup(),
    _importer(nullptr),
    _waitingDlg(nullptr),
    _token(token)
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

QImage CharacterImportHeroForgeDialog::getSelectedImage() const
{
    HeroForgeButton* selectedButton = dynamic_cast<HeroForgeButton*>(_buttonGroup.checkedButton());
    return selectedButton ? selectedButton->getImage() : QImage();
}

QString CharacterImportHeroForgeDialog::getSelectedName() const
{
    HeroForgeButton* selectedButton = dynamic_cast<HeroForgeButton*>(_buttonGroup.checkedButton());
    return selectedButton ? selectedButton->getDataName() : QString();
}

void CharacterImportHeroForgeDialog::showEvent(QShowEvent *event)
{
    if(!_importer)
    {
        if(_token.isEmpty())
        {
            qDebug() << "[CharacterImportHeroForgeDialog] ERROR: Hero Forge importer can't be started without a valid token!";
        }
        else
        {
            _importer = new CharacterImportHeroForge(this);
            connect(_importer, &CharacterImportHeroForge::importComplete, this, &CharacterImportHeroForgeDialog::importComplete);
            _importer->runImport(_token);
            _token = QString(); // clear the variable for security reasons, it isn't needed any more

            if(!_waitingDlg)
            {
                _waitingDlg = new DMHWaitingDialog(QString("Contacting HeroForge..."), this);
                _waitingDlg->setModal(true);
                _waitingDlg->show();
            }
        }
    }

    QDialog::showEvent(event);
}

void CharacterImportHeroForgeDialog::importComplete(QList<CharacterImportHeroForgeData*> data)
{
    if(_waitingDlg)
        _waitingDlg->setStatus(QString("Downloading tokens..."));

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
    if(_waitingDlg)
    {
        _waitingDlg->accept();
        _waitingDlg->deleteLater();
        _waitingDlg = nullptr;
    }

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

    addData(data->getThumbImage(), data->getName() + QString(" Thumbnail"), row, 0);
    addData(data->getPerspectiveImage(), data->getName() + QString(" Perspective"), row, 1);
    addData(data->getFrontImage(), data->getName() + QString(" Front"), row, 2);
    addData(data->getTopImage(), data->getName() + QString(" Top"), row, 3);
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

    HeroForgeButton* iconButton = new HeroForgeButton(image, dataName, this);
    connect(iconButton, &HeroForgeButton::selectButton, this, &QDialog::accept);
    _buttonGroup.addButton(iconButton);
    _iconGrid->addWidget(iconButton, row, column);
}

HeroForgeButton::HeroForgeButton(QImage image, const QString& dataName, QWidget *parent) :
    QPushButton(parent),
    _image(image),
    _dataName(dataName)
{
    setToolTip(_dataName);
    setCheckable(true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMinimumSize(_image.size());
    setMaximumSize(_image.size());
    setIconSize(_image.size());
    setIcon(QIcon(QPixmap::fromImage(_image)));
}

QImage HeroForgeButton::getImage() const
{
    return _image;
}

QString HeroForgeButton::getDataName() const
{
    return _dataName;
}

void HeroForgeButton::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    emit selectButton();
}
