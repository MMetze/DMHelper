#include "publishglimage.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QDebug>

PublishGLImage::PublishGLImage(const QImage& image, bool centered, QObject *parent) :
    PublishGLObject(parent),
    _centered(centered),
    _textureParam(GL_NEAREST),
    _VAO(0),
    _VBO(0),
    _EBO(0),
    _scaleFactor(1.f),
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

void PublishGLImage::paintGL()
{
    if(!QOpenGLContext::currentContext())
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    // qDebug() << "[PublishGLImage] Painting image. VAO: " << _VAO << ", texture: " << _textureID;

    e->glBindVertexArray(_VAO);
    f->glBindTexture(GL_TEXTURE_2D, _textureID);
    f->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void PublishGLImage::setImage(const QImage& image)
{
    cleanup();
    createImageObjects(image);
}

void PublishGLImage::setScale(float scaleFactor)
{
    qDebug() << "[PublishGLImage] Image scale factor set to: " << scaleFactor;

    if(scaleFactor != _scaleFactor)
    {
        _scaleFactor = scaleFactor;
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

QSize PublishGLImage::getSize() const
{
    return _imageSize * _scaleFactor;
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
         (float)image.width() / 2,  (float)image.height() / 2, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
         (float)image.width() / 2, -(float)image.height() / 2, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -(float)image.width() / 2, -(float)image.height() / 2, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -(float)image.width() / 2,  (float)image.height() / 2, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
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
    _modelMatrix.scale(_scaleFactor, _scaleFactor);
}