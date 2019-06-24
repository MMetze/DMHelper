#ifndef COLORPUSHBUTTON_H
#define COLORPUSHBUTTON_H

#include <QPushButton>
#include <QColor>

class ColorPushButton : public QPushButton
{
    Q_OBJECT
public:
    ColorPushButton(QWidget *parent = nullptr);

    QColor getColor() const;

signals:
    void colorChanged(QColor color);

protected slots:
    void selectColor();

protected:
    void paintEvent(QPaintEvent *event);

    QColor _color;
};

#endif // COLORPUSHBUTTON_H
