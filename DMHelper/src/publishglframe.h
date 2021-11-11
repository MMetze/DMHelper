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
    PublishGLRenderer* getRenderer() const;

signals:
    void frameResized(const QSize& newSize);

public slots:
    void cleanup();
    void updateWidget();

    void setRenderer(PublishGLRenderer* renderer);
    void clearRenderer();

    void setBackgroundColor(const QColor& color);

    void setImage(QImage img, QColor color);
    void setImageNoScale(QImage img, QColor color);
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
