#include "ribbontabmap.h"
#include "ui_ribbontabmap.h"
#include "dmconstants.h"

RibbonTabMap::RibbonTabMap(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::RibbonTabMap)
{
    ui->setupUi(this);

    connect(ui->btnZoomIn, SIGNAL(clicked()), this, SIGNAL(zoomInClicked()));
    connect(ui->btnZoomOut, SIGNAL(clicked()), this, SIGNAL(zoomOutClicked()));
    connect(ui->btnZoomFull, SIGNAL(clicked()), this, SIGNAL(zoomFullClicked()));
    connect(ui->btnZoomSelect, SIGNAL(clicked(bool)), this, SIGNAL(zoomSelectClicked(bool)));

    connect(ui->btnCameraCouple, SIGNAL(clicked(bool)), this, SIGNAL(cameraCoupleClicked(bool)));
    connect(ui->btnCameraFullMap, SIGNAL(clicked(bool)), this, SIGNAL(cameraZoomClicked()));
    connect(ui->btnCameraSelect, SIGNAL(clicked(bool)), this, SIGNAL(cameraSelectClicked(bool)));
    connect(ui->btnEditCamera, SIGNAL(clicked(bool)), this, SIGNAL(cameraEditClicked(bool)));

    connect(ui->btnDistance, SIGNAL(clicked(bool)), this, SIGNAL(distanceClicked(bool)));
    connect(ui->btnHeight, SIGNAL(clicked()), this, SLOT(heightEdited()));
    connect(ui->edtHeightDiff, SIGNAL(editingFinished()), this, SLOT(heightEdited()));

    ui->edtHeightDiff->setValidator(new QDoubleValidator(-9999, 9999, 2, this));
    ui->edtHeightDiff->setText(QString::number(0.0));

    connect(ui->btnGrid, SIGNAL(clicked(bool)), this, SIGNAL(gridClicked(bool)));
    connect(ui->spinGridScale, SIGNAL(valueChanged(int)), this, SIGNAL(gridScaleChanged(int)));
    connect(ui->sliderX, SIGNAL(valueChanged(int)), this, SIGNAL(gridXOffsetChanged(int)));
    connect(ui->sliderY, SIGNAL(valueChanged(int)), this, SIGNAL(gridYOffsetChanged(int)));

    connect(ui->btnMapEdit, SIGNAL(clicked(bool)), this, SIGNAL(editFoWClicked(bool)));
    connect(ui->btnFoWErase, SIGNAL(clicked(bool)), this, SIGNAL(drawEraseClicked(bool)));
    connect(ui->btnSmooth, SIGNAL(clicked(bool)), this, SIGNAL(smoothClicked(bool)));
    connect(ui->btnBrushCircle, SIGNAL(clicked(bool)), this, SIGNAL(brushCircleClicked()));
    connect(ui->btnBrushSquare, SIGNAL(clicked(bool)), this, SIGNAL(brushSquareClicked()));
    connect(ui->spinSize, SIGNAL(valueChanged(int)), this, SIGNAL(brushSizeChanged(int)));
    connect(ui->btnBrushSelect, SIGNAL(clicked(bool)), this, SIGNAL(selectFoWClicked(bool)));
    connect(ui->btnFillFoW, SIGNAL(clicked(bool)), this, SIGNAL(fillFoWClicked()));

    connect(ui->btnPointer, SIGNAL(clicked(bool)), this, SIGNAL(pointerClicked(bool)));

    // Set up the brush mode button group
    ui->btnGrpBrush->setId(ui->btnBrushCircle, DMHelper::BrushType_Circle);
    ui->btnGrpBrush->setId(ui->btnBrushSquare, DMHelper::BrushType_Square);
    connect(ui->btnGrpBrush, SIGNAL(buttonClicked(int)), this, SIGNAL(brushModeChanged(int)));

    // Set up the extra slot to configure the erase button
    connect(ui->btnFoWErase, SIGNAL(clicked(bool)), this, SLOT(setEraseMode()));

    setEraseMode();
}

RibbonTabMap::~RibbonTabMap()
{
    delete ui;
}

PublishButtonRibbon* RibbonTabMap::getPublishRibbon() const
{
    return ui->publishFrame;
}

void RibbonTabMap::setZoomSelect(bool checked)
{
    ui->btnZoomSelect->setChecked(checked);
}

void RibbonTabMap::setCameraCouple(bool checked)
{
    ui->btnCameraCouple->setChecked(checked);
}

void RibbonTabMap::setDistanceOn(bool checked)
{
    ui->btnDistance->setChecked(checked);
}

void RibbonTabMap::setDistance(const QString& distance)
{
    ui->edtDistance->setText(distance);
}

void RibbonTabMap::setCameraSelect(bool checked)
{
    ui->btnCameraSelect->setChecked(checked);
}

void RibbonTabMap::setCameraEdit(bool checked)
{
    ui->btnEditCamera->setChecked(checked);
}

void RibbonTabMap::setGridOn(bool checked)
{
    ui->btnGrid->setChecked(checked);
}

void RibbonTabMap::setGridScale(int scale)
{
    ui->spinGridScale->setValue(scale);
}

void RibbonTabMap::setGridXOffset(int offset)
{
    ui->sliderX->setValue(offset);
}

void RibbonTabMap::setGridYOffset(int offset)
{
    ui->sliderY->setValue(offset);
}

void RibbonTabMap::setEditFoW(bool checked)
{
    ui->btnMapEdit->setChecked(checked);
}

void RibbonTabMap::setDrawErase(bool checked)
{
    ui->btnFoWErase->setChecked(checked);
    setEraseMode();
}

void RibbonTabMap::setSmooth(bool checked)
{
    ui->btnSmooth->setChecked(checked);
}

void RibbonTabMap::setBrushSize(int size)
{
    ui->spinSize->setValue(size);
}

void RibbonTabMap::setSelectFoW(bool checked)
{
    ui->btnBrushSelect->setChecked(checked);
}

void RibbonTabMap::setPointerOn(bool checked)
{
    ui->btnPointer->setChecked(checked);
}

void RibbonTabMap::setPointerFile(const QString& filename)
{
    QPixmap pointerImage;
    if((filename.isEmpty()) ||
       (!pointerImage.load(filename)))
    {
        pointerImage = QPixmap(":/img/data/arrow.png");
    }

    QPixmap scaledPointer = pointerImage.scaled(40, 40, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    ui->btnPointer->setIcon(QIcon(scaledPointer));
}

void RibbonTabMap::setEraseMode()
{
    if(ui->btnFoWErase->isChecked())
    {
        ui->btnFillFoW->setIcon(QPixmap(":/img/data/icon_square.png"));
        ui->lblFillFoW->setText(QString("Clear"));
    }
    else
    {
        ui->btnFillFoW->setIcon(QPixmap(":/img/data/square.png"));
        ui->lblFillFoW->setText(QString("Fill"));
    }
}

void RibbonTabMap::heightEdited()
{
    bool ok = false;
    qreal heightDiff = 0.0;
    heightDiff = ui->edtHeightDiff->text().toDouble(&ok);
    if(!ok)
        heightDiff = 0.0;

    emit heightChanged(ui->btnHeight->isChecked(), heightDiff);
}
