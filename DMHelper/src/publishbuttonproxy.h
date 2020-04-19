#ifndef PUBLISHBUTTONPROXY_H
#define PUBLISHBUTTONPROXY_H

#include <QObject>
#include <QColor>

class PublishButtonRibbon;

class PublishButtonProxy : public QObject
{
    Q_OBJECT
public:
    explicit PublishButtonProxy(QObject *parent = nullptr);

    void addPublishButton(PublishButtonRibbon* publishButton);
    void removePublishButton(PublishButtonRibbon* publishButton);

public slots:
    void setChecked(bool checked);
    void setCheckable(bool checkable);
    void setRotation(int rotation);
    void setColor(QColor color);
    void cancelPublish();
    void setPlayersWindow(bool checked);

signals:
    void clicked(bool checked = false);
//    void toggled(bool checked);
    void rotateCW();
    void rotateCCW();
    void rotationChanged(int rotation);
    void colorChanged(QColor color);
    void previewClicked();
    void playersWindowClicked(bool checked);

    void shareSetChecked(bool checked);
    void shareSetCheckable(bool checkable);
    void shareSetRotation(int rotation);
    void shareSetColor(QColor color);
    void shareCancelPublish();
    void shareSetPlayersWindow(bool checked);

};

#endif // PUBLISHBUTTONPROXY_H
