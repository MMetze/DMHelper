#ifndef OVERLAYCOUNTER_H
#define OVERLAYCOUNTER_H

#include <QObject>
#include "overlay.h"

class PublishGLImage;

class OverlayCounter : public Overlay
{
    Q_OBJECT
public:
    explicit OverlayCounter(int counter = 0, const QString& name = QString("Counter"), QObject *parent = nullptr);

    // From Overlay
    virtual void inputXML(const QDomElement &element) override;
    virtual int getOverlayType() const override;
    virtual QSize getSize() const override;
    virtual void prepareFrame(OverlayFrame* frame) override;

    // Local Interface
    virtual int getCounterValue() const;

public slots:
    // From Overlay
    virtual void setX(int x) override;
    virtual void setY(int y) override;

    // Local Interface
    virtual void setCounterValue(int value);
    virtual void setCounterString(const QString& valueString);
    virtual void increase();
    virtual void decrease();

protected:
    // From Overlay
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory) override;
    virtual void doPaintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix) override;
    virtual void doResizeGL(int w, int h) override;
    virtual void createContentsGL() override;

    PublishGLImage* _counterImage;
    int _counter;
};

#endif // OVERLAYCOUNTER_H
