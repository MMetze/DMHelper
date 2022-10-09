#include "layerseditdialog.h"
#include "ui_layerseditdialog.h"
#include "layer.h"
#include "layerscene.h"
#include "layerfow.h"
#include "layerimage.h"
#include "layerframe.h"
#include <QImageReader>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

LayersEditDialog::LayersEditDialog(LayerScene& scene, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LayersEditDialog),
    _layerLayout(nullptr),
    _scene(scene)
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
    items << tr("Image") << tr("FoW") << tr("Tokens");

    bool ok;
    QString selectedItem = QInputDialog::getItem(this, tr("New Layer"), tr("Select New Layer Type:"), items, 0, false, &ok);
    if((!ok) || (selectedItem.isEmpty()))
        return;

    Layer* newLayer = nullptr;
    if(selectedItem == tr("Image"))
    {
        QString newFileName = QFileDialog::getOpenFileName(nullptr, QString("DMHelper New Map File"));
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
    else if(selectedItem == tr("FoW"))
    {
        newLayer = new LayerFow(QString("FoW"), _scene.sceneSize().toSize());
    }
    else if(selectedItem == tr("Tokens"))
    {
        qDebug() << "[LayersEditDialog] Trying to add Token layer which is not yet implemented!";
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

    QDialog::showEvent(event);
}

bool LayersEditDialog::eventFilter(QObject *obj, QEvent *event)
{
    if((event->type() == QEvent::MouseButtonRelease) && (_layerLayout))
        selectFrame(dynamic_cast<LayerFrame*>(obj));

    return QDialog::eventFilter(obj,event);
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
