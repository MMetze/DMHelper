#ifndef PUBLISHWINDOW_H
#define PUBLISHWINDOW_H

#include <QMainWindow>

class PublishGLFrame;
class PublishGLRenderer;

class PublishWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit PublishWindow(const QString& title, QWidget *parent = nullptr);

    QUuid getObjectId() const;
    PublishGLRenderer* getRenderer() const;

signals:
    void windowVisible(bool visible);
    void publishMouseDown(const QPointF& position);
    void publishMouseMove(const QPointF& position);
    void publishMouseRelease(const QPointF& position);
    void frameResized(const QSize& newSize);
    void labelResized(const QSize& newSize);

public slots:
    void setImage(QImage img);
    void setImage(QImage img, const QColor& color);
    //void setImageNoScale(QImage img);
    void setBackgroundColor();
    void setBackgroundColor(const QColor& color);

    void setRenderer(PublishGLRenderer* renderer);

protected:
    virtual void keyPressEvent(QKeyEvent * event);
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent *event);

    void setBackgroundColorStyle(const QColor& color);

private:
    PublishGLFrame* _publishFrame;
    QColor _globalColor;
    bool _globalColorSet;
};

#endif // PUBLISHWINDOW_H
