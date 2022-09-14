#include "layerframe.h"
#include "ui_layerframe.h"
#include "layer.h"

LayerFrame::LayerFrame(Layer& layer, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::LayerFrame),
    _layer(layer)
{
    ui->setupUi(this);

    setLayerVisible(layer.getLayerVisible());
    setIcon(layer.getLayerIcon());
    setName(layer.getName());

    connect(ui->chkVisible, &QAbstractButton::clicked, this, &LayerFrame::visibleChanged);
    connect(ui->edtName, &QLineEdit::editingFinished, this, &LayerFrame::handleNameChanged);

    connect(this, &LayerFrame::nameChanged, &layer, &Layer::setName);
    connect(this, &LayerFrame::visibleChanged, &layer, &Layer::setLayerVisible);

    ui->edtName->installEventFilter(this);

    setLineWidth(2);
    setAutoFillBackground(true);
    setStyleSheet(getStyleString(false));
}

LayerFrame::~LayerFrame()
{
    delete ui;
}

void LayerFrame::setLayerVisible(bool visible)
{
    ui->chkVisible->setChecked(visible);
}

void LayerFrame::setIcon(const QImage& image)
{
    ui->lblIcon->setPixmap(QPixmap::fromImage(image));
}

void LayerFrame::setName(const QString& name)
{
    ui->edtName->setText(name);
}

void LayerFrame::setSelected(bool selected)
{
    setStyleSheet(getStyleString(selected));
}

const Layer& LayerFrame::getLayer() const
{
    return _layer;
}

Layer& LayerFrame::getLayer()
{
    return _layer;
}

void LayerFrame::handleNameChanged()
{
    ui->edtName->setReadOnly(true);
    emit selectMe(this);
    emit nameChanged(ui->edtName->text());
}

bool LayerFrame::eventFilter(QObject *obj, QEvent *event)
 {
    if(obj == ui->edtName)
    {
        if((event->type() == QEvent::MouseButtonDblClick) && (ui->edtName->isReadOnly()))
        {
            ui->edtName->setReadOnly(false);
            ui->edtName->selectAll();
            return true;
        }
        else if(event->type() == QEvent::MouseButtonRelease)
        {
            emit selectMe(this);
        }
        else if(event->type() == QEvent::FocusOut)
        {
            ui->edtName->setReadOnly(true);
        }
    }

    return QFrame::eventFilter(obj, event);
}

QString LayerFrame::getStyleString(bool selected)
{
    if(selected)
        return QString("LayerFrame{ background-image: url(); background-color: rgb(64, 64, 64); }");
    else
        return QString("LayerFrame{ background-image: url(); background-color: none; }");
}
