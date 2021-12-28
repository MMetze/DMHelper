#include "ribbontabbattleview.h"
#include "ui_ribbontabbattleview.h"
#include <QDoubleValidator>

RibbonTabBattleView::RibbonTabBattleView(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::RibbonTabBattleView)
{
    ui->setupUi(this);

    connect(ui->btnZoomIn, SIGNAL(clicked()), this, SIGNAL(zoomInClicked()));
    connect(ui->btnZoomOut, SIGNAL(clicked()), this, SIGNAL(zoomOutClicked()));
    connect(ui->btnZoomFull, SIGNAL(clicked()), this, SIGNAL(zoomFullClicked()));
    connect(ui->btnZoomSelect, SIGNAL(clicked(bool)), this, SIGNAL(zoomSelectClicked(bool)));

    connect(ui->btnCameraCouple, SIGNAL(clicked(bool)), this, SIGNAL(cameraCoupleClicked(bool)));
    connect(ui->btnCameraFullMap, SIGNAL(clicked(bool)), this, SIGNAL(cameraZoomClicked()));
    connect(ui->btnCameraVisible, SIGNAL(clicked(bool)), this, SIGNAL(cameraVisibleClicked()));
    connect(ui->btnCameraSelect, SIGNAL(clicked(bool)), this, SIGNAL(cameraSelectClicked(bool)));
    connect(ui->btnEditCamera, SIGNAL(clicked(bool)), this, SIGNAL(cameraEditClicked(bool)));

    connect(ui->btnDistance, SIGNAL(clicked(bool)), this, SIGNAL(distanceClicked(bool)));
    connect(ui->btnHeight, SIGNAL(clicked()), this, SLOT(heightEdited()));
    connect(ui->edtHeightDiff, SIGNAL(editingFinished()), this, SLOT(heightEdited()));

    ui->edtHeightDiff->setValidator(new QDoubleValidator(-9999, 9999, 2, this));
    ui->edtHeightDiff->setText(QString::number(0.0));

    connect(ui->btnPointer, SIGNAL(clicked(bool)), this, SIGNAL(pointerClicked(bool)));
}

RibbonTabBattleView::~RibbonTabBattleView()
{
    delete ui;
}

PublishButtonRibbon* RibbonTabBattleView::getPublishRibbon()
{
    return ui->framePublish;
}

void RibbonTabBattleView::setZoomSelect(bool checked)
{
    ui->btnZoomSelect->setChecked(checked);
}

void RibbonTabBattleView::setDistanceOn(bool checked)
{
    ui->btnDistance->setChecked(checked);
}

void RibbonTabBattleView::setDistance(const QString& distance)
{
    ui->edtDistance->setText(distance);
}

void RibbonTabBattleView::setCameraSelect(bool checked)
{
    ui->btnCameraSelect->setChecked(checked);
}

void RibbonTabBattleView::setCameraEdit(bool checked)
{
    ui->btnEditCamera->setChecked(checked);
}

void RibbonTabBattleView::setPointerOn(bool checked)
{
    ui->btnPointer->setChecked(checked);
}

void RibbonTabBattleView::setPointerFile(const QString& filename)
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

void RibbonTabBattleView::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    int frameHeight = height();

    setStandardButtonSize(*ui->lblZoomIn, *ui->btnZoomIn, frameHeight);
    setStandardButtonSize(*ui->lblZoomOut, *ui->btnZoomOut, frameHeight);
    setStandardButtonSize(*ui->lblZoomFull, *ui->btnZoomFull, frameHeight);
    setStandardButtonSize(*ui->lblZoomSelect, *ui->btnZoomSelect, frameHeight);
    setStandardButtonSize(*ui->lblCameraCouple, *ui->btnCameraCouple, frameHeight);
    setStandardButtonSize(*ui->lblCameraFullMap, *ui->btnCameraFullMap, frameHeight);
    setStandardButtonSize(*ui->lblCameraVisible, *ui->btnCameraVisible, frameHeight);
    setStandardButtonSize(*ui->lblCameraSelect, *ui->btnCameraSelect, frameHeight);
    setStandardButtonSize(*ui->lblEditCamera, *ui->btnEditCamera, frameHeight);
    setStandardButtonSize(*ui->lblDistance, *ui->btnDistance, frameHeight);
    setStandardButtonSize(*ui->lblPointer, *ui->btnPointer, frameHeight);

    setLineHeight(*ui->line_1, frameHeight);
    setLineHeight(*ui->line_2, frameHeight);
    setLineHeight(*ui->line_3, frameHeight);
    setLineHeight(*ui->line_4, frameHeight);

    int labelHeight = getLabelHeight(*ui->lblDistance2, frameHeight);
    int iconDim = height() - labelHeight;
    QFontMetrics metrics = ui->lblDistance->fontMetrics();
    int textWidth = metrics.maxWidth();

    // Distance cluster
    setWidgetSize(*ui->edtDistance, (iconDim / 2) + (textWidth * 4), iconDim / 2);
    setButtonSize(*ui->btnHeight, iconDim / 2, iconDim / 2);
    setWidgetSize(*ui->edtHeightDiff, (textWidth * 4), iconDim / 2);
    setWidgetSize(*ui->lblDistance2, (iconDim / 2) + (textWidth * 4), labelHeight);
}

void RibbonTabBattleView::heightEdited()
{
    bool ok = false;
    qreal heightDiff = 0.0;
    heightDiff = ui->edtHeightDiff->text().toDouble(&ok);
    if(!ok)
        heightDiff = 0.0;

    emit heightChanged(ui->btnHeight->isChecked(), heightDiff);
}
