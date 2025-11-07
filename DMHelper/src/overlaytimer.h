#ifndef OVERLAYTIMER_H
#define OVERLAYTIMER_H

#include <QObject>
#include <QImage>
#include "overlay.h"

class PublishGLImage;
class QTimer;

class OverlayTimer : public Overlay
{
    Q_OBJECT
public:
    explicit OverlayTimer(int seconds = 0, const QString& name = QString("Timer"), QObject *parent = nullptr);

    // From Overlay
    virtual void inputXML(const QDomElement &element) override;
    virtual int getOverlayType() const override;
    virtual QSize getSize() const override;
    virtual void prepareFrame(QBoxLayout* frameLayout, int insertIndex) override;

    // Local Interface
    virtual int getTimerValue() const;

public slots:
    // From Overlay
    virtual void setX(int x) override;
    virtual void setY(int y) override;

    // Local Interface
    virtual void setTimerValue(int seconds);
    virtual void setTimerString(const QString& seconds);
    virtual void toggle(bool play);
    virtual void start();
    virtual void stop();

signals:
    void timerValueChanged(int seconds);
    void timerExpired();
    void timerStatusChanged(bool running);

protected:
    // From QObject
    virtual void timerEvent(QTimerEvent *event) override;

    // From Overlay
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory) override;
    virtual void doPaintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix) override;
    virtual void doResizeGL(int w, int h) override;
    virtual void createContentsGL() override;
    virtual void updateContentsGL() override;

    // Local Interface
    void createTimerImage();

    PublishGLImage* _timerPublishImage;
    QImage _timerImage;
    int _seconds;
    int _timerId;
};

#endif // OVERLAYTIMER_H
