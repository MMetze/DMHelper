#include "ribbontabbattlemap.h"
#include "ui_ribbontabbattlemap.h"
#include "dmconstants.h"
#include <QDebug>

RibbonTabBattleMap::RibbonTabBattleMap(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::RibbonTabBattleMap)
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

RibbonTabBattleMap::~RibbonTabBattleMap()
{
    delete ui;
}

PublishButtonRibbon* RibbonTabBattleMap::getPublishRibbon()
{
    return ui->framePublish;
}

void RibbonTabBattleMap::setZoomSelect(bool checked)
{
    ui->btnZoomSelect->setChecked(checked);
}

void RibbonTabBattleMap::setCameraCouple(bool checked)
{
    ui->btnCameraCouple->setChecked(checked);
}

void RibbonTabBattleMap::setDistanceOn(bool checked)
{
    ui->btnDistance->setChecked(checked);
}

void RibbonTabBattleMap::setDistance(const QString& distance)
{
    ui->edtDistance->setText(distance);
}

void RibbonTabBattleMap::setCameraSelect(bool checked)
{
    ui->btnCameraSelect->setChecked(checked);
}

void RibbonTabBattleMap::setCameraEdit(bool checked)
{
    ui->btnEditCamera->setChecked(checked);
}

void RibbonTabBattleMap::setGridOn(bool checked)
{
    ui->btnGrid->setChecked(checked);
}

void RibbonTabBattleMap::setGridScale(int scale)
{
    ui->spinGridScale->setValue(scale);
}

void RibbonTabBattleMap::setGridXOffset(int offset)
{
    ui->sliderX->setValue(offset);
}

void RibbonTabBattleMap::setGridYOffset(int offset)
{
    ui->sliderY->setValue(offset);
}

void RibbonTabBattleMap::setEditFoW(bool checked)
{
    ui->btnMapEdit->setChecked(checked);
}

void RibbonTabBattleMap::setDrawErase(bool checked)
{
    ui->btnFoWErase->setChecked(checked);
    setEraseMode();
}

void RibbonTabBattleMap::setSmooth(bool checked)
{
    ui->btnSmooth->setChecked(checked);
}

void RibbonTabBattleMap::setBrushSize(int size)
{
    ui->spinSize->setValue(size);
}

void RibbonTabBattleMap::setSelectFoW(bool checked)
{
    ui->btnBrushSelect->setChecked(checked);
}

void RibbonTabBattleMap::setPointerOn(bool checked)
{
    ui->btnPointer->setChecked(checked);
}

void RibbonTabBattleMap::setPointerFile(const QString& filename)
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

void RibbonTabBattleMap::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    int frameHeight = height();

    setStandardButtonSize(*ui->lblZoomIn, *ui->btnZoomIn, frameHeight);
    setStandardButtonSize(*ui->lblZoomOut, *ui->btnZoomOut, frameHeight);
    setStandardButtonSize(*ui->lblZoomFull, *ui->btnZoomFull, frameHeight);
    setStandardButtonSize(*ui->lblZoomSelect, *ui->btnZoomSelect, frameHeight);
    setStandardButtonSize(*ui->lblCameraCouple, *ui->btnCameraCouple, frameHeight);
    setStandardButtonSize(*ui->lblCameraFullMap, *ui->btnCameraFullMap, frameHeight);
    setStandardButtonSize(*ui->lblCameraFullMap, *ui->btnCameraFullMap, frameHeight);
    setStandardButtonSize(*ui->lblCameraSelect, *ui->btnCameraSelect, frameHeight);
    setStandardButtonSize(*ui->lblEditCamera, *ui->btnEditCamera, frameHeight);
    setStandardButtonSize(*ui->lblDistance, *ui->btnDistance, frameHeight);

    setStandardButtonSize(*ui->lblGrid, *ui->btnGrid, frameHeight);

    setStandardButtonSize(*ui->lblMapEdit, *ui->btnMapEdit, frameHeight);
    setStandardButtonSize(*ui->lblFoWErase, *ui->btnFoWErase, frameHeight);
    setStandardButtonSize(*ui->lblSmooth, *ui->btnSmooth, frameHeight);

    setStandardButtonSize(*ui->lblBrushSelect, *ui->btnBrushSelect, frameHeight);
    setStandardButtonSize(*ui->lblFillFoW, *ui->btnFillFoW, frameHeight);
    setStandardButtonSize(*ui->lblPointer, *ui->btnPointer, frameHeight);

    setLineHeight(*ui->line_2, frameHeight);
    setLineHeight(*ui->line_3, frameHeight);
    setLineHeight(*ui->line_4, frameHeight);
    setLineHeight(*ui->line_5, frameHeight);
    setLineHeight(*ui->line_6, frameHeight);
    setLineHeight(*ui->line_7, frameHeight);

    int labelHeight = getLabelHeight(*ui->lblDistance2, frameHeight);
    int iconDim = height() - labelHeight;
    QFontMetrics metrics = ui->lblDistance->fontMetrics();
    int textWidth = metrics.maxWidth();

    // Distance cluster
    setWidgetSize(*ui->edtDistance, (iconDim / 2) + (textWidth * 4), iconDim / 2);
    setButtonSize(*ui->btnHeight, iconDim / 2, iconDim / 2);
    setWidgetSize(*ui->edtHeightDiff, (textWidth * 4), iconDim / 2);
    setWidgetSize(*ui->lblDistance2, (iconDim / 2) + (textWidth * 4), labelHeight);

    // Grid size cluster
    int labelWidth = qMax(metrics.horizontalAdvance(ui->lblGridScale->text()),
                          qMax(metrics.horizontalAdvance(ui->lblSliderX->text()),
                               metrics.horizontalAdvance(ui->lblSliderY->text())));
    int sliderWidth = ui->btnGrid->width() * 3 / 2;
    setWidgetSize(*ui->lblGridScale, labelWidth, height() / 3);
    setWidgetSize(*ui->spinGridScale, sliderWidth, height() / 3);
    setWidgetSize(*ui->lblSliderX, labelWidth, height() / 3);
    setWidgetSize(*ui->sliderX, sliderWidth, height() / 3);
    setWidgetSize(*ui->lblSliderY, labelWidth, height() / 3);
    setWidgetSize(*ui->sliderY, sliderWidth, height() / 3);

    // Brush cluster
    setButtonSize(*ui->btnBrushCircle, iconDim / 2, iconDim / 2);
    setButtonSize(*ui->btnBrushSquare, iconDim / 2, iconDim / 2);
    int sizeWidth = metrics.horizontalAdvance(ui->lblSize->text());
    setWidgetSize(*ui->lblSize, sizeWidth, iconDim / 2);
    setWidgetSize(*ui->spinSize, sizeWidth*3, iconDim / 2);
    setWidgetSize(*ui->lblBrush, qMax((textWidth * 4), 2 * sizeWidth), labelHeight);
}

void RibbonTabBattleMap::setEraseMode()
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

void RibbonTabBattleMap::heightEdited()
{
    bool ok = false;
    qreal heightDiff = 0.0;
    heightDiff = ui->edtHeightDiff->text().toDouble(&ok);
    if(!ok)
        heightDiff = 0.0;

    emit heightChanged(ui->btnHeight->isChecked(), heightDiff);
}
