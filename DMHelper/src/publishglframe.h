#ifndef PUBLISHGLFRAME_H
#define PUBLISHGLFRAME_H

#include "overlayrenderer.h"
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
    OverlayRenderer* getOverlayRenderer() const;

signals:
    void publishMouseDown(const QPointF& position);
    void publishMouseMove(const QPointF& position);
    void publishMouseRelease(const QPointF& position);
    void frameResized(const QSize& newSize);
    void labelResized(const QSize& newSize);

public slots:
    void cleanup();
    void updateWidget();

    void setRenderer(PublishGLRenderer* renderer);
    void clearRenderer();

    void setBackgroundColor(const QColor& color);

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;

    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

    bool convertMousePosition(QMouseEvent& event, const QRect& scissorRect, QPointF& result);

protected slots:
    void initializeRenderer();

private:
    bool _initialized;
    QSize _targetSize;
    PublishGLRenderer* _renderer;
    OverlayRenderer* _overlayRenderer;

};

#endif // PUBLISHGLFRAME_H
