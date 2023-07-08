#ifndef PUBLISHGLTOKENHIGHLIGHTREF_H
#define PUBLISHGLTOKENHIGHLIGHTREF_H

#include "publishgltokenhighlight.h"

class PublishGLImage;

class PublishGLTokenHighlightRef : public PublishGLTokenHighlight
{
    Q_OBJECT
public:
    explicit PublishGLTokenHighlightRef(PublishGLImage& referenceImage, QObject *parent = nullptr);
    virtual ~PublishGLTokenHighlightRef() override;

    virtual void paintGL(QOpenGLFunctions *f, int shaderModelMatrix) override;

    virtual int getWidth() const override;
    virtual int getHeight() const override;

    PublishGLImage& getImage() const;

protected:
    PublishGLImage& _referenceImage;
};

#endif // PUBLISHGLTOKENHIGHLIGHTREF_H
