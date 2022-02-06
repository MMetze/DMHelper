#include "ribbontabbattleview.h"
#include "ui_ribbontabbattleview.h"
#include <QDoubleValidator>
#include <QMenu>

RibbonTabBattleView::RibbonTabBattleView(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::RibbonTabBattleView),
    _isBattle(true)
{
    ui->setupUi(this);
    setIsBattle(true);

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
    connect(ui->btnFreeDistance, SIGNAL(clicked(bool)), this, SIGNAL(freeDistanceClicked(bool)));

    ui->btnLineColor->setRotationVisible(false);
    ui->btnLineColor->setColor(Qt::yellow);

    connect(ui->btnHeight, SIGNAL(clicked()), this, SLOT(heightEdited()));
    connect(ui->edtDistanceScale, &QLineEdit::textChanged, this, &RibbonTabBattleView::freeScaleEdited);
    connect(ui->edtHeightDiff, SIGNAL(editingFinished()), this, SLOT(heightEdited()));
    ui->edtHeightDiff->setValidator(new QDoubleValidator(-9999, 9999, 2, this));
    ui->edtHeightDiff->setText(QString::number(0.0));
    ui->edtDistanceScale->setValidator(new QIntValidator(this));

    QMenu* lineTypeMenu = new QMenu(this);
    RibbonTabBattleView_LineTypeAction* solidAction = new RibbonTabBattleView_LineTypeAction(QIcon(":/img/data/icon_pensolid.png"), QString("Solid Line"), Qt::SolidLine);
    lineTypeMenu->addAction(solidAction);
    ui->btnLineType->setIcon(solidAction->icon());
    lineTypeMenu->addAction(new RibbonTabBattleView_LineTypeAction(QIcon(":/img/data/icon_pendash.png"), QString("Dashed Line"), Qt::DashLine));
    lineTypeMenu->addAction(new RibbonTabBattleView_LineTypeAction(QIcon(":/img/data/icon_pendot.png"), QString("Dotted Line"), Qt::DotLine));
    lineTypeMenu->addAction(new RibbonTabBattleView_LineTypeAction(QIcon(":/img/data/icon_pendashdot.png"), QString("Dash Dot Line"), Qt::DashDotLine));
    lineTypeMenu->addAction(new RibbonTabBattleView_LineTypeAction(QIcon(":/img/data/icon_pendashdotdot.png"), QString("Dash Dot Dot Line"), Qt::DashDotDotLine));
    ui->btnLineType->setMenu(lineTypeMenu);
    connect(lineTypeMenu, &QMenu::triggered, this, &RibbonTabBattleView::selectLineTypeAction);
    connect(ui->btnLineColor, &ColorPushButton::colorChanged, this, &RibbonTabBattleView::distanceLineColorChanged);
    connect(ui->spinLineWidth, SIGNAL(valueChanged(int)), this, SIGNAL(distanceLineWidthChanged(int)));

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

bool RibbonTabBattleView::getIsBattle() const
{
    return _isBattle;
}

void RibbonTabBattleView::setIsBattle(bool isBattle)
{
    if(_isBattle == isBattle)
        return;

    ui->btnHeight->setVisible(isBattle);
    ui->edtHeightDiff->setVisible(isBattle);
    ui->lblDistanceScale->setVisible(!isBattle);
    ui->edtDistanceScale->setVisible(!isBattle);

    _isBattle = isBattle;
}

void RibbonTabBattleView::setZoomSelect(bool checked)
{
    ui->btnZoomSelect->setChecked(checked);
}

void RibbonTabBattleView::setCameraSelect(bool checked)
{
    ui->btnCameraSelect->setChecked(checked);
}

void RibbonTabBattleView::setCameraEdit(bool checked)
{
    ui->btnEditCamera->setChecked(checked);
}

void RibbonTabBattleView::setDistanceOn(bool checked)
{
    ui->btnDistance->setChecked(checked);
}

void RibbonTabBattleView::setFreeDistanceOn(bool checked)
{
    ui->btnFreeDistance->setChecked(checked);
}

void RibbonTabBattleView::setDistance(const QString& distance)
{
    ui->edtMeasurement->setText(distance);
}

void RibbonTabBattleView::setDistanceScale(int scale)
{
    ui->edtDistanceScale->setText(QString::number(scale));
}

void RibbonTabBattleView::setDistanceLineType(int lineType)
{
    if(!ui->btnLineType->menu())
        return;

    int newLineType = lineType - 1;
    if((newLineType < 0) || (newLineType >= ui->btnLineType->menu()->actions().count()))
        newLineType = 0;

    selectLineTypeAction(ui->btnLineType->menu()->actions().at(newLineType));
}

void RibbonTabBattleView::setDistanceLineColor(const QColor& color)
{
    ui->btnLineColor->setColor(color);
}

void RibbonTabBattleView::setDistanceLineWidth(int lineWidth)
{
    ui->spinLineWidth->setValue(lineWidth);
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

    setButtonSizeOnly(*ui->lblZoom, *ui->btnZoomIn, frameHeight);
    setButtonSizeOnly(*ui->lblZoom, *ui->btnZoomOut, frameHeight);
    setButtonSizeOnly(*ui->lblZoom, *ui->btnZoomFull, frameHeight);
    setButtonSizeOnly(*ui->lblZoom, *ui->btnZoomSelect, frameHeight);
    setLabelHeight(*ui->lblZoom, frameHeight);

    setButtonSizeOnly(*ui->lblCamera, *ui->btnCameraCouple, frameHeight);
    setButtonSizeOnly(*ui->lblCamera, *ui->btnCameraFullMap, frameHeight);
    setButtonSizeOnly(*ui->lblCamera, *ui->btnCameraVisible, frameHeight);
    setButtonSizeOnly(*ui->lblCamera, *ui->btnCameraSelect, frameHeight);
    setButtonSizeOnly(*ui->lblCamera, *ui->btnEditCamera, frameHeight);
    setLabelHeight(*ui->lblCamera, frameHeight);

    setStandardButtonSize(*ui->lblDistance, *ui->btnDistance, frameHeight);
    setStandardButtonSize(*ui->lblFreeDistance, *ui->btnFreeDistance, frameHeight);

    QFontMetrics metrics = ui->lblDistance->fontMetrics();
    int labelHeight = getLabelHeight(*ui->lblDistance, frameHeight);
    int iconDim = height() - labelHeight;
    int textWidth = metrics.maxWidth();

    setStandardButtonSize(*ui->lblLineColor, *ui->btnLineColor, frameHeight);
    int squareButtonSize = qMin(ui->btnLineColor->width(), iconDim);
    int colorButtonSize = squareButtonSize * 3 / 4;

    setWidgetSize(*ui->btnLineColor, colorButtonSize, colorButtonSize);
    setWidgetSize(*ui->spinLineWidth, iconDim, iconDim);
    setLabelHeight(*ui->lblLineWidth, frameHeight);
    setStandardButtonSize(*ui->lblLineType, *ui->btnLineType, frameHeight);
    setWidgetSize(*ui->btnLineType, squareButtonSize, squareButtonSize);

    setStandardButtonSize(*ui->lblPointer, *ui->btnPointer, frameHeight);

    setLineHeight(*ui->line_1, frameHeight);
    setLineHeight(*ui->line_2, frameHeight);
    setLineHeight(*ui->line_3, frameHeight);
    setLineHeight(*ui->line_4, frameHeight);

    // Distance cluster
    int halfIconSize = iconDim / 2;
    int distanceWidth = qMax(metrics.horizontalAdvance(ui->lblDistanceScale->text()), halfIconSize);
    setWidgetSize(*ui->lblMeasurement, distanceWidth, halfIconSize);
    setWidgetSize(*ui->edtMeasurement, distanceWidth + (textWidth * 4), halfIconSize);
    setButtonSize(*ui->btnHeight, distanceWidth, halfIconSize);
    setWidgetSize(*ui->edtHeightDiff, distanceWidth + (textWidth * 4), halfIconSize);
    setWidgetSize(*ui->lblDistanceScale, distanceWidth, halfIconSize);
    setWidgetSize(*ui->edtDistanceScale, distanceWidth + (textWidth * 4), halfIconSize);
    setWidgetSize(*ui->lblDistance2, distanceWidth + (textWidth * 4), labelHeight);
}

void RibbonTabBattleView::freeScaleEdited(const QString &text)
{
    bool ok = false;
    int result = text.toInt(&ok);
    if(ok)
        emit distanceScaleChanged(result);
}

void RibbonTabBattleView::selectLineTypeAction(QAction* action)
{
    if(!action)
        return;

    if(!action->icon().isNull())
        ui->btnLineType->setIcon(action->icon());

    RibbonTabBattleView_LineTypeAction* lineTypeAction = dynamic_cast<RibbonTabBattleView_LineTypeAction*>(action);
    if(!lineTypeAction)
        return;

    emit distanceLineTypeChanged(lineTypeAction->getLineType());
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






RibbonTabBattleView_LineTypeAction::RibbonTabBattleView_LineTypeAction(const QIcon &icon, const QString &text, int lineType, QObject *parent) :
    QAction(icon, text, parent),
    _lineType(lineType)
{
}

RibbonTabBattleView_LineTypeAction::~RibbonTabBattleView_LineTypeAction()
{
}

int RibbonTabBattleView_LineTypeAction::getLineType() const
{
    return _lineType;
}

