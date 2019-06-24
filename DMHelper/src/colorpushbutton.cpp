#include "colorpushbutton.h"
#include <QPainter>
#include <QColorDialog>

ColorPushButton::ColorPushButton(QWidget *parent) :
    QPushButton(parent),
    _color(Qt::black)
{
    connect(this, SIGNAL(clicked(bool)), this, SLOT(selectColor()));
}

QColor ColorPushButton::getColor() const
{
    return _color;
}

void ColorPushButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    QRect rect(6, 6, width()-12, height()-12);
    QPainter painter(this);
    painter.setBrush(_color);
    painter.setPen(Qt::black);
    painter.drawRect(rect);
}

void ColorPushButton::selectColor()
{
    QColor result = QColorDialog::getColor(_color);
    if(result.isValid())
    {
        _color = result;
        update();
        emit colorChanged(_color);
    }
}
