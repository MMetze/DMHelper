#ifndef PUBLISHBUTTONFRAME_H
#define PUBLISHBUTTONFRAME_H

#include <QFrame>

namespace Ui {
class PublishButtonFrame;
}

class PublishButtonFrame : public QFrame
{
    Q_OBJECT

public:
    explicit PublishButtonFrame(QWidget *parent = nullptr);
    ~PublishButtonFrame();

public:
    bool isChecked();
    bool isCheckable();
    void setCheckable(bool checkable);
    QColor getColor() const;
    int getRotation();

public slots:
    void setChecked(bool checked);
    void setRotation(int rotation);
    void setColor(QColor color);
    void cancelPublish();

signals:
    void clicked(bool checked = false);
    void toggled(bool checked);
    void rotateCW();
    void rotateCCW();
    void colorChanged(QColor color);

private slots:
    void handleToggle(bool checked);

private:
    Ui::PublishButtonFrame *ui;
};

#endif // PUBLISHBUTTONFRAME_H
