#include "publishglrect.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>

PublishGLRect::PublishGLRect(const QColor& color, const QRectF& rect, QObject *parent) :
    QObject{parent},
    _VAO(0),
    _VBO(0),
    _EBO(0),
    _modelMatrix(),
    _position(),
    _color(color),
    _rect(rect),
    _rebuildShape(false)
{
    prepareObjects();
}

PublishGLRect::~PublishGLRect()
{
    PublishGLRect::cleanup();
}

void PublishGLRect::cleanup()
{
    if(QOpenGLContext::currentContext())
    {
        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
        QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();

        if(_VAO > 0)
        {
            if(e)
                e->glDeleteVertexArrays(1, &_VAO);
            _VAO = 0;
        }

        if(_VBO > 0)
        {
            if(f)
                f->glDeleteBuffers(1, &_VBO);
            _VBO = 0;
        }

        if(_EBO > 0)
        {
            if(f)
                f->glDeleteBuffers(1, &_EBO);
            _EBO = 0;
        }
    }
}

void PublishGLRect::paintGL()
{
    if(!QOpenGLContext::currentContext())
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    if((_rebuildShape) || (_VAO == 0) || (_VBO == 0) || (_EBO == 0))
    {
        cleanup();
        prepareObjects();
        _rebuildShape = false;
    }

    e->glBindVertexArray(_VAO);
    f->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void PublishGLRect::setColor(const QColor& color)
{
    _color = color;
    _rebuildShape = true;
    emit changed();
}

void PublishGLRect::setRect(const QRectF& rect)
{
    _rect = rect;
    _rebuildShape = true;
    emit changed();
}

void PublishGLRect::setSize(const QSize& size)
{
    _rect.setSize(size);
    _rebuildShape = true;
    emit changed();
}

void PublishGLRect::setPosition(const QPoint& position)
{
    if(_position == position)
        return;

    _position = position;
    _modelMatrix.setToIdentity();
    _modelMatrix.translate(_position.x(),
                           _position.y());

    emit changed();
}

QColor PublishGLRect::getColor() const
{
    return _color;
}

QRectF PublishGLRect::getRect() const
{
    return _rect;
}

QSize PublishGLRect::getSize() const
{
    return getRect().size().toSize();
}

QPoint PublishGLRect::getPosition() const
{
    return _position;
}

QMatrix4x4 PublishGLRect::getMatrix() const
{
    return _modelMatrix;
}

const float * PublishGLRect::getMatrixData() const
{
    return _modelMatrix.constData();
}

void PublishGLRect::prepareObjects()
{
    if(_rect.isEmpty())
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    float vertices[] = {
        // positions                                           // colors           // texture coords
        (float)_rect.width(),                   0.0f,            0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,   // top right
        (float)_rect.width(), -(float)_rect.height(),            0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,   // bottom right
        0.0f,                 -(float)_rect.height(),            0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        0.0f,                                   0.0f,            0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f    // top left
    };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    e->glGenVertexArrays(1, &_VAO);
    f->glGenBuffers(1, &_VBO);
    f->glGenBuffers(1, &_EBO);

    e->glBindVertexArray(_VAO);

    f->glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    f->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    f->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    f->glEnableVertexAttribArray(0);
    // color attribute
    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    f->glEnableVertexAttribArray(1);
    // texture attribute
    f->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    f->glEnableVertexAttribArray(2);
}
