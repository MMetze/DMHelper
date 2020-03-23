#ifndef PUBLISHBUTTONRIBBON_H
#define PUBLISHBUTTONRIBBON_H

#include <QFrame>

namespace Ui {
class PublishButtonRibbon;
}

class PublishButtonRibbon : public QFrame
{
    Q_OBJECT

public:
    explicit PublishButtonRibbon(QWidget *parent = nullptr);
    virtual ~PublishButtonRibbon();

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
    void setDefaults();

    Ui::PublishButtonRibbon *ui;
};

#endif // PUBLISHBUTTONRIBBON_H
