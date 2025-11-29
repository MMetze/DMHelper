#include "publishglimage.h"
#include "dmh_opengl.h"
#include <QOpenGLContext>
#include <QDebug>

PublishGLImage::PublishGLImage(const QImage& image, bool centered, QObject *parent) :
    PublishGLObject(parent),
    _centered(centered),
    _textureParam(GL_NEAREST),
    _VAO(0),
    _VBO(0),
    _EBO(0),
    _scaleFactor(1.f),
    _scaleX(1.f),
    _scaleY(1.f),
    _x(0.f),
    _y(0.f),
    _imageSize()
{
    createImageObjects(image);
}

PublishGLImage::PublishGLImage(const QImage& image, int textureParam, bool centered, QObject *parent) :
    PublishGLObject(parent),
    _centered(centered),
    _textureParam(textureParam),
    _VAO(0),
    _VBO(0),
    _EBO(0),
    _scaleFactor(1.f),
    _scaleX(1.f),
    _scaleY(1.f),
    _x(0.f),
    _y(0.f),
    _imageSize()
{
    createImageObjects(image);
}

PublishGLImage::~PublishGLImage()
{
    PublishGLImage::cleanup();
}

void PublishGLImage::cleanup()
{
    // qDebug() << "[PublishGLImage] Cleaning up image object. VAO: " << _VAO << ", VBO: " << _VBO << ", EBO: " << _EBO << ", texture: " << _textureID;

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

    _imageSize = QSize();

    PublishGLObject::cleanup();
}

void PublishGLImage::paintGL(QOpenGLFunctions* functions, const GLfloat* projectionMatrix)
{
    Q_UNUSED(projectionMatrix);

    if((!QOpenGLContext::currentContext()) || (!functions))
        return;

    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if(!e)
        return;

    e->glBindVertexArray(_VAO);
    functions->glBindTexture(GL_TEXTURE_2D, _textureID);
    functions->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void PublishGLImage::setImage(const QImage& image)
{
    cleanup();
    createImageObjects(image);
}

void PublishGLImage::updateImage(const QImage& image)
{
    if(image.size() != _imageSize)
    {
        qDebug() << "[PublishGLImage] ERROR: Trying to update image to something of a different size!";
        return;
    }

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

    // update and generate the background texture
    QImage glBackgroundImage = image.convertToFormat(QImage::Format_RGBA8888).mirrored();
    f->glBindTexture(GL_TEXTURE_2D, _textureID);
    f->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, glBackgroundImage.width(), glBackgroundImage.height(), GL_RGBA, GL_UNSIGNED_BYTE, glBackgroundImage.bits());
    f->glGenerateMipmap(GL_TEXTURE_2D);
}

void PublishGLImage::setScale(float scaleFactor)
{
    if(scaleFactor != _scaleFactor)
    {
        _scaleFactor = scaleFactor;
        updateMatrix();
    }
}

void PublishGLImage::setScaleX(float scaleFactor)
{
    if(scaleFactor != _scaleX)
    {
        _scaleX = scaleFactor;
        updateMatrix();
    }
}

void PublishGLImage::setScaleY(float scaleFactor)
{
    if(scaleFactor != _scaleY)
    {
        _scaleY = scaleFactor;
        updateMatrix();
    }
}

void PublishGLImage::setX(float x)
{
    if(x != _x)
    {
        _x = x;
        updateMatrix();
    }
}

void PublishGLImage::setY(float y)
{
    if(y != _y)
    {
        _y = y;
        updateMatrix();
    }
}

void PublishGLImage::setPosition(float x, float y)
{
    if((x != _x) || (y != _y))
    {
        _x = x;
        _y = y;
        updateMatrix();
    }
}

void PublishGLImage::setPosition(const QPointF& pos)
{
    setPosition(pos.x(), pos.y());
}

void PublishGLImage::setPositionScaleX(float x, float scaleFactor)
{
    if((x != _x) || (scaleFactor != _scaleX))
    {
        _x = x;
        _scaleX = scaleFactor;
        updateMatrix();
    }
}

void PublishGLImage::setPositionScaleY(float y, float scaleFactor)
{
    if((y != _y) || (scaleFactor != _scaleY))
    {
        _y = y;
        _scaleY = scaleFactor;
        updateMatrix();
    }
}

void PublishGLImage::setPositionScale(float x, float y, float scaleFactor)
{
    if((x != _x) || (y != _y) || (scaleFactor != _scaleFactor))
    {
        _x = x;
        _y = y;
        _scaleFactor = scaleFactor;
        updateMatrix();
    }
}

void PublishGLImage::setPositionScale(const QPointF& pos, float scaleFactor)
{
    setPositionScale(pos.x(), pos.y(), scaleFactor);
}

float PublishGLImage::getX() const
{
    return _x;
}

float PublishGLImage::getY() const
{
    return _y;
}

QPointF PublishGLImage::getPosition() const
{
    return QPointF(_x, _y);
}

QSize PublishGLImage::getSize() const
{
    return _imageSize * _scaleFactor;
}

QSize PublishGLImage::getImageSize() const
{
    return _imageSize;
}

float PublishGLImage::getScale() const
{
    return _scaleFactor;
}

float PublishGLImage::getScaleX() const
{
    return _scaleX;
}

float PublishGLImage::getScaleY() const
{
    return _scaleY;
}

void PublishGLImage::createImageObjects(const QImage& image)
{
    if(!QOpenGLContext::currentContext())
        return;

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    // qDebug() << "[PublishGLImage] Creating images objects for image";

    float vertices[] = {
        // positions                                                   // colors           // texture coords
         (float)image.width() / 2,  (float)image.height() / 2, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,   // top right
         (float)image.width() / 2, -(float)image.height() / 2, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,   // bottom right
        -(float)image.width() / 2, -(float)image.height() / 2, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -(float)image.width() / 2,  (float)image.height() / 2, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f    // top left
    };

    if(!_centered)
    {
        vertices[0]  = (float)image.width(); vertices[1]  = (float)image.height();
        vertices[8]  = (float)image.width(); vertices[9]  = 0.0f;
        vertices[16] = 0.0f;                 vertices[17] = 0.0f;
        vertices[24] = 0.0f;                 vertices[25] = (float)image.height();
    }

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

    // Texture
    f->glGenTextures(1, &_textureID);
    f->glBindTexture(GL_TEXTURE_2D, _textureID);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _textureParam);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _textureParam);

    // load and generate the background texture
    QImage glBackgroundImage = image.convertToFormat(QImage::Format_RGBA8888).mirrored();
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glBackgroundImage.width(), glBackgroundImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glBackgroundImage.bits());
    f->glGenerateMipmap(GL_TEXTURE_2D);

    _imageSize = glBackgroundImage.size();
}

void PublishGLImage::updateMatrix()
{
    _modelMatrix.setToIdentity();
    _modelMatrix.translate(_x, _y);
    _modelMatrix.scale(_scaleFactor * _scaleX, _scaleFactor * _scaleY);
}
