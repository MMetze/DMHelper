#include "layerseditdialog.h"
#include "ui_layerseditdialog.h"
#include "layer.h"
#include "layerscene.h"
#include "layerfow.h"
#include "layerimage.h"
#include "layertokens.h"
#include "layervideo.h"
#include "layervideoeffect.h"
#include "layerblank.h"
#include "layerframe.h"
#include "layergrid.h"
#include "ribbonframe.h"
#include "publishglrenderer.h"
#include "mapblankdialog.h"
#include <QImageReader>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>

//#define DEBUG_LAYERSEDITDIALOG

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
    int currentSelectedIndex = _layerLayout->count() - 1 - currentSelected;
    int newFrameOrder = frame->getLayer().getOrder();

    if(newFrameOrder == currentSelected)
        return;

#ifdef DEBUG_LAYERSEDITDIALOG
    qDebug() << "[LayersEditDialog] Current selected: " << currentSelected << " of " << _layerLayout->count() << ", new selection order: " << newFrameOrder;
#endif

    if((currentSelectedIndex >= 0) && (currentSelectedIndex < _layerLayout->count()) && (_layerLayout->itemAt(currentSelectedIndex)))
    {
        if(LayerFrame* currentFrame = dynamic_cast<LayerFrame*>(_layerLayout->itemAt(currentSelectedIndex)->widget()))
        {
            currentFrame->setSelected(false);
#ifdef DEBUG_LAYERSEDITDIALOG
            qDebug() << "[LayersEditDialog] Deactivating item " << currentSelectedIndex << ", with order " << currentFrame->getLayer().getOrder();
#endif
        }
    }

    _scene.setSelectedLayerIndex(newFrameOrder);
    frame->setSelected(true);
}

void LayersEditDialog::moveUp()
{
    int currentSelected = _scene.getSelectedLayerIndex();
    _scene.moveLayer(currentSelected, currentSelected + 1);
    resetLayout();
}

void LayersEditDialog::moveDown()
{
    int currentSelected = _scene.getSelectedLayerIndex();
    _scene.moveLayer(currentSelected, currentSelected - 1);
    resetLayout();
}

void LayersEditDialog::addLayer()
{
    QStringList items;
    items << tr("Image") << tr("Video") << tr("Effect Video") << tr("FoW");
    if(_model)
        items << tr("Tokens") ;
    items << tr("Grid") << tr("Blank");

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

        QFileInfo fileInfo(newFileName);
        newLayer = new LayerImage(QString("Image: ") + fileInfo.fileName(), newFileName);
    }
    else if(selectedItem == tr("Video"))
    {
        QString newFileName = QFileDialog::getOpenFileName(nullptr, QString("DMHelper New Video File"));
        if(newFileName.isEmpty())
            return;

        QFileInfo fileInfo(newFileName);
        newLayer = new LayerVideo(QString("Video: ") + fileInfo.fileName(), newFileName);
    }
    else if(selectedItem == tr("Effect Video"))
    {
        QString newFileName = QFileDialog::getOpenFileName(nullptr, QString("DMHelper New Effect Video File"));
        if(newFileName.isEmpty())
            return;
        
        newLayer = new LayerVideoEffect(QString("Effect Video: ") + newFileName, newFileName);
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
    }
    else if(selectedItem == tr("Blank"))
    {
        MapBlankDialog blankDlg;
        int result = blankDlg.exec();
        if(result != QDialog::Accepted)
            return;

        LayerBlank* blankLayer = new LayerBlank(QString("Blank Layer"), blankDlg.getMapColor());
        blankLayer->setSize(blankDlg.getMapSize());
        newLayer = blankLayer;
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
    // Ask the user to confirm the deletion of the layer
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Remove Layer"), tr("Are you sure you want to remove the selected layer?"), QMessageBox::Yes|QMessageBox::No);
    if(reply != QMessageBox::Yes)
        return;

    int currentSelected = _scene.getSelectedLayerIndex();
    _scene.removeLayer(currentSelected);
    resetLayout();
}

void LayersEditDialog::updateSceneSize()
{
    QSizeF currentSize = _scene.sceneSize();
    ui->edtSceneWidth->setText(QString::number(currentSize.width()));
    ui->edtSceneHeight->setText(QString::number(currentSize.height()));
}

void LayersEditDialog::updateDMVisibility(LayerFrame* frame)
{
    if(!frame)
        return;

    if(frame->isLayerVisibleDM())
    {
        selectFrame(frame);
    }
    else
    {
        int currentIndex = _layerLayout->indexOf(frame);
        int nextFrameIndex = (currentIndex == (_layerLayout->count() - 1)) ? 0 : currentIndex + 1;
        LayerFrame* nextFrame = nullptr;
        while(((!nextFrame) || (!nextFrame->isLayerVisibleDM())) &&
              (nextFrameIndex != currentIndex))
        {
            QLayoutItem* nextItem = _layerLayout->itemAt(nextFrameIndex);
            nextFrame = dynamic_cast<LayerFrame*>(nextItem->widget());

            if(++nextFrameIndex >= _layerLayout->count())
                nextFrameIndex = 0;
        }

        if((nextFrame) && (nextFrame->isLayerVisibleDM()))
            selectFrame(nextFrame);
    }
}

void LayersEditDialog::updateVisibility(LayerFrame* frame)
{
    if((!frame) || (!_layerLayout))
        return;

    int frameIndex = _layerLayout->indexOf(frame);
    if(frameIndex + 1 >= _layerLayout->count())
        return;

    QLayoutItem* nextItem = _layerLayout->itemAt(frameIndex + 1);
    LayerFrame* nextFrame = dynamic_cast<LayerFrame*>(nextItem->widget());

    if((!nextFrame) || (!nextFrame->isLinkedUp()))
        return;

    nextFrame->setLayerVisible(frame->isLayerVisible());
    nextFrame->setLayerVisibleDM(frame->isLayerVisibleDM());
    nextFrame->setLayerVisiblePlayer(frame->isLayerVisiblePlayer());
}

void LayersEditDialog::linkedUp(LayerFrame* frame)
{
    if((!frame) || (!_layerLayout))
        return;

    int frameIndex = _layerLayout->indexOf(frame);
    if(frameIndex == 0 )
        return;

    QLayoutItem* previousItem = _layerLayout->itemAt(frameIndex - 1);
    LayerFrame* previousFrame = dynamic_cast<LayerFrame*>(previousItem->widget());

    if(!previousFrame)
        return;

    frame->setLayerVisible(previousFrame->isLayerVisible());
    frame->setLayerVisibleDM(previousFrame->isLayerVisibleDM());
    frame->setLayerVisiblePlayer(previousFrame->isLayerVisiblePlayer());
}

void LayersEditDialog::updateRenderer()
{
    if(_scene.getRenderer())
        _scene.getRenderer()->updateRender();
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

    ui->line->setMinimumHeight(labelHeight);
    ui->line->setMaximumHeight(labelHeight);

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

    RibbonFrame::setButtonSize(*ui->btnClose, buttonSize, buttonSize);
    ui->btnClose->setIconSize(iconSize);

    QDialog::showEvent(event);
}

bool LayersEditDialog::eventFilter(QObject *obj, QEvent *event)
{
    if((event->type() == QEvent::MouseButtonRelease) && (_layerLayout))
        selectFrame(dynamic_cast<LayerFrame*>(obj));

    if(event->type() == QEvent::KeyPress)
    {
        if(QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event))
        {
            if((keyEvent->key() == Qt::Key_Return) || (keyEvent->key() == Qt::Key_Enter))
                return true;
        }
    }

    return QDialog::eventFilter(obj, event);
}

void LayersEditDialog::resetLayout()
{
    clearLayout();
    readScene();
    updateRenderer();
}

void LayersEditDialog::readScene()
{
    for(int i = 0; i < _scene.layerCount(); ++i)
    {
        Layer* layer = _scene.layerAt(i);
        if(layer)
        {
            LayerFrame* newFrame = new LayerFrame(*layer, ui->scrollAreaWidgetContents);
            newFrame->installEventFilter(this);
            newFrame->setSelected(_scene.getSelectedLayerIndex() == i);
            connect(newFrame, &LayerFrame::selectMe, this, &LayersEditDialog::selectFrame);
            connect(newFrame, &LayerFrame::refreshPlayer, this, &LayersEditDialog::updateRenderer);
            connect(newFrame, &LayerFrame::positionChanged, this, &LayersEditDialog::updateSceneSize);
            connect(newFrame, &LayerFrame::sizeChanged, this, &LayersEditDialog::updateSceneSize);
            connect(newFrame, &LayerFrame::linkedUp, this, &LayersEditDialog::linkedUp);
            connect(newFrame, &LayerFrame::visibilityChanged, this, &LayersEditDialog::updateVisibility);
            connect(newFrame, &LayerFrame::dmVisibilityChanged, this, &LayersEditDialog::updateDMVisibility);
            _layerLayout->insertWidget(0, newFrame);
        }
    }

#ifdef DEBUG_LAYERSEDITDIALOG
    qDebug() << "[LayersEditDialog] Layer Frame overview:";
#endif

    // Update the linked status
    for(int i = 0; i < _layerLayout->count(); ++i)
    {
        QLayoutItem* item = _layerLayout->itemAt(i);
        if(item)
        {
            LayerFrame* frame = dynamic_cast<LayerFrame*>(item->widget());
            if((frame) && (frame->getLayer().getLinkedUp()))
                linkedUp(frame);

#ifdef DEBUG_LAYERSEDITDIALOG
            if(frame)
                qDebug() << "[LayersEditDialog]     Layer layout entry " << i << ": order = " << frame->getLayer().getOrder() << ", name = " << frame->getLayer().getName();
#endif
        }
    }

    updateSceneSize();
}

void LayersEditDialog::clearLayout()
{
    if(!_layerLayout)
        return;

    QLayoutItem *child;
    while((child = _layerLayout->takeAt(0)) != nullptr)
    {
        if(child->widget())
            child->widget()->deleteLater();
        delete child;
    }
}
