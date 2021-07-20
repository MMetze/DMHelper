#ifndef PUBLISHGLFRAME_H
#define PUBLISHGLFRAME_H

#include <QOpenGLWidget>

class PublishGLRenderer;

class PublishGLFrame : public QOpenGLWidget
{
    Q_OBJECT
public:
    PublishGLFrame(QWidget *parent = nullptr);
    virtual ~PublishGLFrame() override;

    bool isInitialized() const;

public slots:
    void cleanup();
    void updateWidget();

    void setRenderer(PublishGLRenderer* renderer);

    void setImage(QImage img);
    void setImageNoScale(QImage img);
    void setArrowVisible(bool visible);
    void setArrowPosition(const QPointF& position);
    void setPointerFile(const QString& filename);

protected:
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

    void setOrthoProjection();

private:
    bool _initialized;
    QSize _targetSize;
    PublishGLRenderer* _renderer;
    unsigned int _shaderProgram;
};

#endif // PUBLISHGLFRAME_H
