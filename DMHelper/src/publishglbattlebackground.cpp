#include "publishglbattlebackground.h"
#include "dmh_opengl.h"
#include <QOpenGLContext>
#include <QImage>
#include <QDebug>

PublishGLBattleBackground::PublishGLBattleBackground(PublishGLScene* scene, const QImage& image, int textureParam) :
    PublishGLBattleObject(scene),
    _imageSize(),
    _position(),
    _targetSize(),
    _textureParam(textureParam),
    _VAO(0),
    _VBO(0),
    _EBO(0)
{
    createImageObjects(image);
}

PublishGLBattleBackground::~PublishGLBattleBackground()
{
    PublishGLBattleBackground::cleanup();
}

void PublishGLBattleBackground::cleanup()
{
    _imageSize = QSize();

    qDebug() << "[PublishGLBattleBackground] Cleaning up image object. VAO: " << _VAO << ", VBO: " << _VBO << ", EBO: " << _EBO << ", texture: " << _textureID;

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

    PublishGLBattleObject::cleanup();
}

void PublishGLBattleBackground::paintGL(QOpenGLFunctions* functions, const GLfloat* projectionMatrix)
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

void PublishGLBattleBackground::setImage(const QImage& image)
{
    cleanup();
    createImageObjects(image);
    updateModelMatrix();
}

QSize PublishGLBattleBackground::getSize() const
{
    return _imageSize;
}

void PublishGLBattleBackground::updateImage(const QImage& image)
{
    if((_imageSize.isEmpty()) || (_imageSize != image.size()))
    {
        setImage(image);
    }
    else
    {
        loadTexture(image);
        updateModelMatrix();
    }
}

void PublishGLBattleBackground::setPosition(const QPoint& position)
{
    if(_position == position)
        return;

    _position = position;
    updateModelMatrix();
}

void PublishGLBattleBackground::setTargetSize(const QSize& size)
{
    if(_targetSize == size)
        return;

    _targetSize = size;
    updateModelMatrix();
}

void PublishGLBattleBackground::createImageObjects(const QImage& image)
{
    if(!QOpenGLContext::currentContext())
        return;

    // qDebug() << "[BattleGLBackground] Creating background image objects";

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    _imageSize = image.size();

    float vertices[] = {
        // positions                                                   // colors           // texture coords
//         (float)image.width() / 2,  (float)image.height() / 2, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,   // top right
//         (float)image.width() / 2, -(float)image.height() / 2, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,   // bottom right
//        -(float)image.width() / 2, -(float)image.height() / 2, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,   // bottom left
//        -(float)image.width() / 2,  (float)image.height() / 2, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f    // top left
        (float)image.width(),                   0.0f,            0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,   // top right
        (float)image.width(), -(float)image.height(),            0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,   // bottom right
        0.0f,                 -(float)image.height(),            0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,   // bottom left
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

    // Texture
    f->glGenTextures(1, &_textureID);
    loadTexture(image);

    qDebug() << "[PublishGLBattleBackground] Created image object. w: " << image.width() << ", h: " << image.height() << ", VAO: " << _VAO << ", VBO: " << _VBO << ", EBO: " << _EBO << ", texture: " << _textureID << ", context: " << QOpenGLContext::currentContext();

}

void PublishGLBattleBackground::updateModelMatrix()
{
    _modelMatrix.setToIdentity();
    _modelMatrix.translate(_position.x(),
                           _position.y());
    if(_targetSize.isValid())
        _modelMatrix.scale(static_cast<qreal>(_targetSize.width()) / static_cast<qreal>(_imageSize.width()),
                           static_cast<qreal>(_targetSize.height()) / static_cast<qreal>(_imageSize.height()));
}

void PublishGLBattleBackground::loadTexture(const QImage& image)
{
    if((_textureID == 0) || (!QOpenGLContext::currentContext()))
        return;

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

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
}
