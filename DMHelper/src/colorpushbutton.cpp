#include "colorpushbutton.h"
#include <QPainter>
#include <QColorDialog>

ColorPushButton::ColorPushButton(QWidget *parent) :
    QPushButton(parent),
    _color(Qt::black),
    _rotation(0)
{
    connect(this, SIGNAL(clicked(bool)), this, SLOT(selectColor()));
}

QColor ColorPushButton::getColor() const
{
    return _color;
}

int ColorPushButton::getRotation() const
{
    return _rotation;
}

void ColorPushButton::rotateCW()
{
    _rotation += 90;
    if(_rotation >= 360)
        _rotation -= 360;

    update();
}

void ColorPushButton::rotateCCW()
{
    _rotation -= 90;
    if(_rotation < 0)
        _rotation += 360;

    update();
}

void ColorPushButton::setRotation(int rotation)
{
    if(rotation != _rotation)
    {
        _rotation = rotation;
        update();
        emit rotationChanged(_rotation);
    }
}

void ColorPushButton::setRotationVisible(bool rotationVisible)
{
    if(_rotationVisible != rotationVisible)
    {
        _rotationVisible = rotationVisible;
        update();
    }
}

void ColorPushButton::setColor(const QColor& color)
{
    if(color != _color)
    {
        _color = color;
        update();
        emit colorChanged(_color);
    }
}

void ColorPushButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    QRect rect(2, 2, width()-5, height()-5);
    QPainter painter(this);
    if(isEnabled())
    {
        painter.setBrush(_color);
        painter.setPen(Qt::black);
    }
    else
    {
        painter.setBrush(QColor(196, 196, 196));
        painter.setPen(Qt::darkGray);
    }
    painter.drawRect(rect);

    if(_rotationVisible)
    {
        QImage arrowImg(":/img/data/icon_arrow_xor.png");
        painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
        if(_rotation != 0)
        {
            arrowImg = arrowImg.transformed(QTransform().rotate(_rotation), Qt::SmoothTransformation);
        }

        painter.drawImage(rect, arrowImg);
    }
}

void ColorPushButton::selectColor()
{
    QColor result = QColorDialog::getColor(_color);
    if(result.isValid())
        setColor(result);
}
