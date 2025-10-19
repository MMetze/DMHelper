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

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;

    virtual int getOverlayType() const override;

    virtual int getCounterValue() const;

public slots:
    virtual void setCounterValue(int value);
    virtual void increase();
    virtual void decrease();

protected:
    // From CampaignObjectBase
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    virtual void doPaintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix) override;

    virtual void createContentsGL() override;
    virtual void updateContentsScale(int w, int h) override;

    PublishGLImage* _counterImage;
    int _counter;
};

#endif // OVERLAYCOUNTER_H
