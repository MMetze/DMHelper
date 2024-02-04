#ifndef PUBLISHGLRECT_H
#define PUBLISHGLRECT_H

#include <QObject>
#include <QMatrix4x4>
#include <QColor>

class PublishGLRect : public QObject
{
    Q_OBJECT
public:
    explicit PublishGLRect(const QColor& color, const QRectF& rect, QObject *parent = nullptr);
    virtual ~PublishGLRect() override;

    virtual void cleanup();
    virtual void paintGL();

    virtual void setColor(const QColor& color);
    virtual void setRect(const QRectF& rect);
    virtual void setSize(const QSize& size);
    virtual void setPosition(const QPoint& position);

    virtual QColor getColor() const;
    virtual QRectF getRect() const;
    virtual QSize getSize() const;
    virtual QPoint getPosition() const;

    QMatrix4x4 getMatrix() const;
    const float * getMatrixData() const;

signals:
    void changed();

protected:
    void prepareObjectsGL();

    unsigned int _VAO;
    unsigned int _VBO;
    unsigned int _EBO;

    QMatrix4x4 _modelMatrix;
    QPoint _position;
    QColor _color;
    QRectF _rect;

    bool _rebuildShape;
};

#endif // PUBLISHGLRECT_H
