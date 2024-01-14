#include "layerframe.h"
#include "ui_layerframe.h"
#include "layer.h"
#include "layervideo.h"

LayerFrame::LayerFrame(Layer& layer, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::LayerFrame),
    _layer(layer),
    _opacity()
{
    ui->setupUi(this);

    updateLayerData();

    connect(ui->chkVisibleDM, &QAbstractButton::toggled, this, &LayerFrame::visibleDMChanged);
    connect(ui->chkVisiblePlayer, &QAbstractButton::toggled, this, &LayerFrame::visiblePlayerChanged);
    connect(ui->chkLinkUp, &QAbstractButton::toggled, this, &LayerFrame::linkedUpChanged);
    connect(ui->chkVisibleDM, &QAbstractButton::clicked, this, &LayerFrame::handleVisibleChanged);
    connect(ui->chkVisiblePlayer, &QAbstractButton::clicked, this, &LayerFrame::handleVisibleChanged);
    connect(ui->chkVisible, &QAbstractButton::clicked, this, &LayerFrame::handleVisibleClicked);
    connect(ui->chkLinkUp, &QAbstractButton::clicked, this, &LayerFrame::handleLinkUp);
    connect(ui->edtName, &QLineEdit::editingFinished, this, &LayerFrame::handleNameChanged);

    connect(this, &LayerFrame::nameChanged, &layer, &Layer::setName);
    connect(this, &LayerFrame::visibleDMChanged, &layer, &Layer::setLayerVisibleDM);
    connect(this, &LayerFrame::visiblePlayerChanged, &layer, &Layer::setLayerVisiblePlayer);
    connect(this, &LayerFrame::visibleDMChanged, this, [=](){ emit visibilityChanged(this); });
    connect(this, &LayerFrame::visibleDMChanged, this, [=](){ emit dmVisibilityChanged(this); });
    connect(this, &LayerFrame::visiblePlayerChanged, this, [=](){ emit visibilityChanged(this); });
    connect(this, &LayerFrame::linkedUpChanged, &layer, &Layer::setLinkedUp);
    connect(this, &LayerFrame::opacityChanged, &layer, &Layer::setOpacity);
    connect(this, &LayerFrame::positionChanged, &layer, QOverload<const QPoint&>::of(&Layer::setPosition));
    connect(this, &LayerFrame::sizeChanged, &layer, QOverload<const QSize&>::of(&Layer::setSize));

    connect(ui->sliderOpacity, &QSlider::valueChanged, this, &LayerFrame::handleOpacityChanged);
    connect(ui->spinOpacity, &QSpinBox::editingFinished, this, &LayerFrame::handleOpacityChanged);
    connect(ui->btnSettings, &QAbstractButton::clicked, &layer, &Layer::editSettings);
    connect(ui->spinX, &QSpinBox::editingFinished, this, &LayerFrame::handleXChanged);
    connect(ui->spinY, &QSpinBox::editingFinished, this, &LayerFrame::handleYChanged);
    connect(ui->spinWidth, &QSpinBox::editingFinished, this, &LayerFrame::handleWidthChanged);
    connect(ui->spinHeight, &QSpinBox::editingFinished, this, &LayerFrame::handleHeightChanged);
    connect(ui->btnLockRatio, &QAbstractButton::clicked, this, &LayerFrame::handleLockClicked);

    ui->edtName->installEventFilter(this);
    ui->btnSettings->setVisible(layer.getType() == DMHelper::LayerType_VideoEffect);

    setLineWidth(5);
    setAutoFillBackground(true);
    setStyleSheet(getStyleString(false));

    if(layer.getType() == DMHelper::LayerType_Reference)
    {
        ui->spinX->setEnabled(false);
        ui->spinY->setEnabled(false);
        ui->spinWidth->setEnabled(false);
        ui->spinHeight->setEnabled(false);
    }
    else if(layer.getFinalType() == DMHelper::LayerType_Tokens)
    {
        ui->spinWidth->setEnabled(false);
        ui->spinHeight->setEnabled(false);
    }
    else if((layer.getFinalType() == DMHelper::LayerType_Video) ||
            (layer.getFinalType() == DMHelper::LayerType_VideoEffect))
    {
        LayerVideo* layerVideo = dynamic_cast<LayerVideo*>(&layer);
        if(layerVideo)
            connect(layerVideo, &LayerVideo::screenshotAvailable, this, &LayerFrame::updateLayerData);
    }
}

LayerFrame::~LayerFrame()
{
    delete ui;
}

void LayerFrame::setLayerVisible(bool visible)
{
    if(ui->chkVisible->isChecked() != visible)
        ui->chkVisible->setChecked(visible);
}

void LayerFrame::setLayerVisibleDM(bool visible)
{
    if(ui->chkVisibleDM->isChecked() != visible)
        ui->chkVisibleDM->setChecked(visible);
}

void LayerFrame::setLayerVisiblePlayer(bool visible)
{
    if(ui->chkVisiblePlayer->isChecked() != visible)
        ui->chkVisiblePlayer->setChecked(visible);
}

void LayerFrame::setIcon(const QImage& image)
{
    ui->lblIcon->setPixmap(QPixmap::fromImage(image));
}

void LayerFrame::setLinkedUp(bool linkUp)
{
    if(ui->chkLinkUp->isChecked() != linkUp)
        ui->chkLinkUp->setChecked(linkUp);
}

void LayerFrame::setName(const QString& name)
{
    if(ui->edtName->text() != name)
        ui->edtName->setText(name);
}

void LayerFrame::setOpacity(int opacity)
{
    if(_opacity == opacity)
        return;

    _opacity = opacity;

    if(ui->sliderOpacity->value() != opacity)
        ui->sliderOpacity->setValue(opacity);

    if(ui->spinOpacity->value() != opacity)
        ui->spinOpacity->setValue(opacity);
}

void LayerFrame::setPosition(const QPoint& position)
{
    setX(position.x());
    setY(position.y());
}

void LayerFrame::setX(int x)
{
    if(ui->spinX->value() != x)
        ui->spinX->setValue(x);
}

void LayerFrame::setY(int y)
{
    if(ui->spinY->value() != y)
        ui->spinY->setValue(y);
}

void LayerFrame::setSize(const QSize& size)
{
    setWidth(size.width());
    setHeight(size.height());
}

void LayerFrame::setWidth(int width)
{
    if(width < 0)
        return;

    if(ui->spinWidth->value() != width)
        ui->spinWidth->setValue(width);
}

void LayerFrame::setHeight(int height)
{
    if(height < 0)
        return;

    if(ui->spinHeight->value() != height)
        ui->spinHeight->setValue(height);
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

bool LayerFrame::isLinkedUp() const
{
    return ui->chkLinkUp->isChecked();
}

bool LayerFrame::isLayerVisible() const
{
    return ui->chkVisible->isChecked();
}

bool LayerFrame::isLayerVisibleDM() const
{
    return ui->chkVisibleDM->isChecked();
}

bool LayerFrame::isLayerVisiblePlayer() const
{
    return ui->chkVisiblePlayer->isChecked();
}

void LayerFrame::handleLinkUp(bool checked)
{
    ui->chkVisible->setEnabled(!checked);
    ui->chkVisibleDM->setEnabled(!checked);
    ui->chkVisiblePlayer->setEnabled(!checked);

    if(checked)
        emit linkedUp(this);
}

void LayerFrame::handleVisibleClicked(bool checked)
{
    ui->chkVisibleDM->setChecked(checked);
    ui->chkVisiblePlayer->setChecked(checked);
}

void LayerFrame::handleVisibleChanged()
{
    ui->chkVisible->setChecked(_layer.getLayerVisibleDM() && _layer.getLayerVisiblePlayer());
}

void LayerFrame::handleNameChanged()
{
    ui->edtName->setReadOnly(true);
    emit selectMe(this);
    emit nameChanged(ui->edtName->text());
}

void LayerFrame::handleOpacityChanged()
{
    int newOpacity = ui->spinOpacity->value();
    if(_opacity == newOpacity)
    {
        newOpacity = ui->sliderOpacity->value();
        if(_opacity == newOpacity)
            return;
    }

    setOpacity(newOpacity);

    emit selectMe(this);
    emit opacityChanged(static_cast<qreal>(newOpacity) / 100.0);
    emit refreshPlayer();
}

void LayerFrame::handleXChanged()
{
    updatePosition(ui->spinX->value(), ui->spinY->value());
    emit selectMe(this);
    emit refreshPlayer();
}

void LayerFrame::handleYChanged()
{
    updatePosition(ui->spinX->value(), ui->spinY->value());
    emit selectMe(this);
    emit refreshPlayer();
}

void LayerFrame::handleWidthChanged()
{
    int newHeight;
    if((ui->btnLockRatio->isChecked()) && (ui->spinWidth->value() != 0))
        newHeight = (ui->spinHeight->value() * ui->spinWidth->value()) / _layer.getSize().width();
    else
        newHeight = ui->spinHeight->value();

    updateSize(ui->spinWidth->value(), newHeight);
    emit selectMe(this);
    emit refreshPlayer();
}

void LayerFrame::handleHeightChanged()
{
    int newWidth;
    if((ui->btnLockRatio->isChecked()) && (ui->spinHeight->value() != 0))
        newWidth = (ui->spinWidth->value() * ui->spinHeight->value()) / _layer.getSize().height();
    else
        newWidth = ui->spinWidth->value();

    updateSize(newWidth, ui->spinHeight->value());
    emit selectMe(this);
    emit refreshPlayer();
}

void LayerFrame::handleLockClicked()
{
    emit selectMe(this);
}

void LayerFrame::updateLayerData()
{
    setLayerVisibleDM(_layer.getLayerVisibleDM());
    setLayerVisiblePlayer(_layer.getLayerVisiblePlayer());
    handleVisibleChanged();
    setLinkedUp(_layer.getLinkedUp());
    handleLinkUp(_layer.getLinkedUp());
    setName(_layer.getName());
    setOpacity(_layer.getOpacity() * 100.0);
    setPosition(_layer.getPosition());
    setIcon(_layer.getLayerIcon());
    setSize(_layer.getSize());
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
        return QString("LayerFrame{ background-color: rgb(64, 64, 64); }");
    else
        return QString("LayerFrame{ background-color: none; }");
}

void LayerFrame::updatePosition(int x, int y)
{
    if(ui->spinX->value() != x)
        ui->spinX->setValue(x);

    if(ui->spinY->value() != y)
        ui->spinY->setValue(y);

    emit positionChanged(QPoint(x, y));
}

void LayerFrame::updateSize(int width, int height)
{
    if(ui->spinWidth->value() != width)
        ui->spinWidth->setValue(width);

    if(ui->spinHeight->value() != height)
        ui->spinHeight->setValue(height);

    emit sizeChanged(QSize(width, height));
}
