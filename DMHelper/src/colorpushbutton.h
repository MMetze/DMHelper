#ifndef COLORPUSHBUTTON_H
#define COLORPUSHBUTTON_H

#include <QPushButton>
#include <QColor>

class ColorPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit ColorPushButton(QWidget *parent = nullptr);

    QColor getColor() const;
    int getRotation() const;

public slots:
    void rotateCW();
    void rotateCCW();
    void setRotation(int rotation);
    void setColor(QColor color);

signals:
    void rotationChanged(int rotation);
    void colorChanged(QColor color);

protected slots:
    void selectColor();

protected:
    void paintEvent(QPaintEvent *event);

    QColor _color;
    int _rotation;
};

#endif // COLORPUSHBUTTON_H
