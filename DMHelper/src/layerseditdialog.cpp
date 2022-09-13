#include "layerseditdialog.h"
#include "ui_layerseditdialog.h"
#include "layer.h"
#include "layerscene.h"
#include "layerframe.h"
#include <QVBoxLayout>

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

void LayersEditDialog::readScene()
{
    for(int i = 0; i < _scene.layerCount(); ++i )
    {
        Layer* layer = _scene.layerAt(i);
        if(layer)
        {
            LayerFrame* newFrame = new LayerFrame(true, *layer, ui->scrollAreaWidgetContents);
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
        delete child;
}
