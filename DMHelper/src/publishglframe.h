#ifndef PUBLISHGLFRAME_H
#define PUBLISHGLFRAME_H

#include <QOpenGLWidget>

class PublishGLRenderer;
class PublishGLImage;

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

protected:
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

protected slots:
    void initializeRenderer();

private:
    bool _initialized;
    QSize _targetSize;
    PublishGLRenderer* _renderer;

};

#endif // PUBLISHGLFRAME_H
