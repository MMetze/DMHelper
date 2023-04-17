#include "layerseditdialog.h"
#include "ui_layerseditdialog.h"
#include "layer.h"
#include "layerscene.h"
#include "layerfow.h"
#include "layerimage.h"
#include "layertokens.h"
#include "layervideo.h"
#include "layerframe.h"
#include "layergrid.h"
#include "ribbonframe.h"
#include <QImageReader>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

LayersEditDialog::LayersEditDialog(LayerScene& scene, BattleDialogModel* model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LayersEditDialog),
    _layerLayout(nullptr),
    _scene(scene),
    _model(model)
{
    ui->setupUi(this);

    _layerLayout = new QVBoxLayout;
    _layerLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    ui->scrollAreaWidgetContents->setLayout(_layerLayout);

    connect(ui->btnUp, &QAbstractButton::clicked, this, &LayersEditDialog::moveUp);
    connect(ui->btnDown, &QAbstractButton::clicked, this, &LayersEditDialog::moveDown);
    connect(ui->btnNewLayer, &QAbstractButton::clicked, this, &LayersEditDialog::addLayer);
    connect(ui->btnRemoveLayer, &QAbstractButton::clicked, this, &LayersEditDialog::removeLayer);
}

LayersEditDialog::~LayersEditDialog()
{
    delete ui;
}

void LayersEditDialog::selectFrame(LayerFrame* frame)
{
    if(!frame)
        return;

    int currentSelected = _scene.getSelectedLayerIndex();
    if((currentSelected >= 0) && (currentSelected < _layerLayout->count()) &&
       (frame->getLayer().getOrder() != currentSelected) && (_layerLayout->itemAt(currentSelected)))
    {
        LayerFrame* currentFrame = dynamic_cast<LayerFrame*>(_layerLayout->itemAt(currentSelected)->widget());
        if(currentFrame)
            currentFrame->setSelected(false);
    }
    _scene.setSelectedLayerIndex(frame->getLayer().getOrder());
    frame->setSelected(true);
}

void LayersEditDialog::moveUp()
{
    int currentSelected = _scene.getSelectedLayerIndex();
    _scene.moveLayer(currentSelected, currentSelected - 1);
    resetLayout();
}

void LayersEditDialog::moveDown()
{
    int currentSelected = _scene.getSelectedLayerIndex();
    _scene.moveLayer(currentSelected, currentSelected + 1);
    resetLayout();
}

void LayersEditDialog::addLayer()
{
    QStringList items;
    items << tr("Image") << tr("Video") << tr("FoW");
    if(_model)
        items << tr("Tokens") ;
    items << tr("Grid") << tr("Text");

    bool ok;
    QString selectedItem = QInputDialog::getItem(this, tr("New Layer"), tr("Select New Layer Type:"), items, 0, false, &ok);
    if((!ok) || (selectedItem.isEmpty()))
        return;

    Layer* newLayer = nullptr;
    if(selectedItem == tr("Image"))
    {
        QString newFileName = QFileDialog::getOpenFileName(nullptr, QString("DMHelper New Image File"));
        if(newFileName.isEmpty())
            return;
/*
        QImageReader reader(newFileName);
        QImage imgBackground = reader.read();
        if(imgBackground.isNull())
        {
            qDebug() << "[Map] Not able to read new image file " << newFileName << ": " << reader.error() <<", " << reader.errorString();
#if !defined(Q_OS_MAC)
            QMessageBox::critical(nullptr,
                                  QString("DMHelper Image File Read Error"),
                                  QString("The selected image """) + newFileName + QString(""" could not be read. It may be too high resolution for DMHelper!"));
#endif
            return;
        }

        if(imgBackground.format() != QImage::Format_ARGB32_Premultiplied)
            imgBackground.convertTo(QImage::Format_ARGB32_Premultiplied);
            */

        newLayer = new LayerImage(QString("Image"), newFileName);
        //qDebug() << "[LayersEditDialog] Trying to add Image layer which is not yet implemented!";
        //return;
    }
    else if(selectedItem == tr("Video"))
    {
        QString newFileName = QFileDialog::getOpenFileName(nullptr, QString("DMHelper New Video File"));
        if(newFileName.isEmpty())
            return;

        newLayer = new LayerVideo(QString("Video"), newFileName);
    }
    else if(selectedItem == tr("FoW"))
    {
        newLayer = new LayerFow(QString("FoW"), _scene.sceneSize().toSize());
    }
    else if(selectedItem == tr("Tokens"))
    {
        if(!_model)
        {
            qDebug() << "[LayersEditDialog] ERROR: Trying to add Token layer without a valid battle model!";
            return;
        }

        newLayer = new LayerTokens(_model, QString("Tokens"));
    }
    else if(selectedItem == tr("Grid"))
    {
        newLayer = new LayerGrid(QString("Grid"));
        //qDebug() << "[LayersEditDialog] Trying to add Grid layer which is not yet implemented!";
        //return;
    }
    else if(selectedItem == tr("Text"))
    {
        qDebug() << "[LayersEditDialog] Trying to add Text layer which is not yet implemented!";
        return;
    }
    else
    {
        qDebug() << "[LayersEditDialog] ERROR: Trying to add an unknown layer type!";
        return;
    }

    _scene.appendLayer(newLayer);
    resetLayout();
}

void LayersEditDialog::removeLayer()
{
    int currentSelected = _scene.getSelectedLayerIndex();
    _scene.removeLayer(currentSelected);
    resetLayout();
}

void LayersEditDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
}

void LayersEditDialog::showEvent(QShowEvent *event)
{
    if((_layerLayout) && (_layerLayout->count() == 0))
        readScene();

    int ribbonHeight = RibbonFrame::getRibbonHeight();
    QFontMetrics metrics = ui->lblSceneHeight->fontMetrics();
    int labelHeight = RibbonFrame::getLabelHeight(metrics, ribbonHeight);
    int buttonSize = ribbonHeight - labelHeight;
    QSize iconSize(buttonSize * 4 / 5, buttonSize * 4 / 5);

    RibbonFrame::setButtonSize(*ui->btnScaleScene, buttonSize, buttonSize);
    ui->btnScaleScene->setIconSize(iconSize);
    RibbonFrame::setButtonSize(*ui->btnMinimumExpand, buttonSize, buttonSize);
    ui->btnMinimumExpand->setIconSize(iconSize);
    RibbonFrame::setButtonSize(*ui->btnMaximumExpand, buttonSize, buttonSize);
    ui->btnMaximumExpand->setIconSize(iconSize);
    RibbonFrame::setButtonSize(*ui->btnScaleOriginal, buttonSize, buttonSize);
    ui->btnScaleOriginal->setIconSize(iconSize);
    RibbonFrame::setButtonSize(*ui->btnUp, buttonSize, buttonSize);
    ui->btnUp->setIconSize(iconSize);
    RibbonFrame::setButtonSize(*ui->btnDown, buttonSize, buttonSize);
    ui->btnDown->setIconSize(iconSize);
    RibbonFrame::setButtonSize(*ui->btnNewLayer, buttonSize, buttonSize);
    ui->btnNewLayer->setIconSize(iconSize);
    RibbonFrame::setButtonSize(*ui->btnRemoveLayer, buttonSize, buttonSize);
    ui->btnRemoveLayer->setIconSize(iconSize);

    RibbonFrame::setWidgetSize(*ui->lblSceneWidth, buttonSize / 2, buttonSize / 2);
    //int labelHeight = RibbonFrame::getLabelHeight(metrics, frameHeight);
    //setButtonSize(*ui->btnRemoveLayer, ribbonHeight, buttonWidth);
    RibbonFrame::setWidgetSize(*ui->lblSceneHeight, buttonSize / 2, buttonSize / 2);
    //setButtonSize(*ui->btnRemoveLayer, ribbonHeight, buttonWidth);
    //setStandardButtonSize(*ui->lblEditFile, *ui->btnEditFile, frameHeight);

    QDialog::showEvent(event);
}

bool LayersEditDialog::eventFilter(QObject *obj, QEvent *event)
{
    if((event->type() == QEvent::MouseButtonRelease) && (_layerLayout))
        selectFrame(dynamic_cast<LayerFrame*>(obj));

    return QDialog::eventFilter(obj, event);
}

void LayersEditDialog::resetLayout()
{
    clearLayout();
    readScene();
}

void LayersEditDialog::readScene()
{
    for(int i = 0; i < _scene.layerCount(); ++i )
    {
        Layer* layer = _scene.layerAt(i);
        if(layer)
        {
            LayerFrame* newFrame = new LayerFrame(*layer, ui->scrollAreaWidgetContents);
            newFrame->installEventFilter(this);
            newFrame->setSelected(_scene.getSelectedLayerIndex() == i);
            connect(newFrame, &LayerFrame::selectMe, this, &LayersEditDialog::selectFrame);
            _layerLayout->addWidget(newFrame);
        }
    }
}

void LayersEditDialog::clearLayout()
{
    if(!_layerLayout)
        return;

    QLayoutItem *child;
    while ((child = _layerLayout->takeAt(0)) != nullptr)
    {
        if(child->widget())
            child->widget()->deleteLater();
        delete child;
    }
}
