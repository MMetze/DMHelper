#include "publishglbattlebackground.h"
#include "dmh_opengl.h"
#include <QOpenGLContext>
#include <QImage>
#include <QDebug>

// Bytes per pixel for the GL_RGBA / GL_UNSIGNED_BYTE layout expected by
// glTexImage2D / glTexSubImage2D. Used to convert bytesPerLine → pixel stride
// in updateImageRegion's GL_UNPACK_ROW_LENGTH call.
static constexpr int BYTES_PER_PIXEL_RGBA = 4;

// Returns true when 'minFilter' requires mipmap levels to be generated.
// Used by loadTexture() to guard glGenerateMipmap; reused by updateImageRegion()
// without re-deriving the filter set (chunk 1 predicate).
static constexpr bool isMipmapMinFilter(int minFilter)
{
    return minFilter == GL_NEAREST_MIPMAP_NEAREST ||
           minFilter == GL_NEAREST_MIPMAP_LINEAR  ||
           minFilter == GL_LINEAR_MIPMAP_NEAREST  ||
           minFilter == GL_LINEAR_MIPMAP_LINEAR;
}

PublishGLBattleBackground::PublishGLBattleBackground(PublishGLScene* scene, const QImage& image, int textureParam,
                                                     bool sourceIsRgba8888, bool sourceNeedsVerticalFlip) :
    PublishGLBattleObject(scene),
    _imageSize(),
    _position(),
    _targetSize(),
    _textureParam(textureParam),
    _VAO(0),
    _VBO(0),
    _EBO(0),
    _sourceIsRgba8888(sourceIsRgba8888),
    _sourceNeedsVerticalFlip(sourceNeedsVerticalFlip)
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

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    _imageSize = image.size();

    // V-coordinate convention:
    // When _sourceNeedsVerticalFlip=true (default), loadTexture() CPU-flips the image before
    // uploading so image row 0 (top) ends up at GL texture T=1 (top in GL space). V=1.0 at
    // the screen-top vertex and V=0.0 at the screen-bottom vertex → image renders right-side-up.
    //
    // GL bottom-left origin: when skipping the CPU vertical flip (_sourceNeedsVerticalFlip=false,
    // FOW path), image row 0 (top) maps directly to GL texture T=0 (bottom in GL space).
    // Inverting V here compensates, so the image still appears right-side-up on screen.
    const float vTop    = _sourceNeedsVerticalFlip ? 1.0f : 0.0f; // texcoord at screen-top edge
    const float vBottom = _sourceNeedsVerticalFlip ? 0.0f : 1.0f; // texcoord at screen-bottom edge

    float vertices[] = {
        // positions                                                   // colors           // texture coords
//         (float)image.width() / 2,  (float)image.height() / 2, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, vTop,    // top right
//         (float)image.width() / 2, -(float)image.height() / 2, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, vBottom, // bottom right
//        -(float)image.width() / 2, -(float)image.height() / 2, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, vBottom, // bottom left
//        -(float)image.width() / 2,  (float)image.height() / 2, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, vTop     // top left
        (float)image.width(),                   0.0f,            0.0f,   1.0f, 1.0f, 1.0f,   1.0f, vTop,    // top right
        (float)image.width(), -(float)image.height(),            0.0f,   1.0f, 1.0f, 1.0f,   1.0f, vBottom, // bottom right
        0.0f,                 -(float)image.height(),            0.0f,   1.0f, 1.0f, 1.0f,   0.0f, vBottom, // bottom left
        0.0f,                                   0.0f,            0.0f,   1.0f, 1.0f, 1.0f,   0.0f, vTop     // top left
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
    // convertToFormat is a guarded fast-path: skipped when _sourceIsRgba8888=true (FOW path)
    // because the image is already Format_RGBA8888, avoiding a full-image deep copy on
    // every update. Non-FOW callers continue to take the convert path.
    QImage glBackgroundImage = _sourceIsRgba8888 ? image : image.convertToFormat(QImage::Format_RGBA8888);
    // CPU vertical flip is a guarded fast-path: skipped when _sourceNeedsVerticalFlip=false
    // (FOW path). In that case, V-coordinates in createImageObjects() are inverted to
    // compensate for GL's bottom-left origin, so the image renders right-side-up without
    // the per-upload copy. Non-FOW callers continue to take the flip path.
    if(_sourceNeedsVerticalFlip)
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
        glBackgroundImage = glBackgroundImage.flipped(Qt::Vertical);
#else
        glBackgroundImage = glBackgroundImage.mirrored(false, true);
#endif
    }
    // Stride-safe upload: pitch-padded sources (video decode buffers) have bytesPerLine > width*4.
    // For tightly packed sources the row length equals the width, leaving behaviour unchanged.
    f->glPixelStorei(GL_UNPACK_ROW_LENGTH, glBackgroundImage.bytesPerLine() / BYTES_PER_PIXEL_RGBA);
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glBackgroundImage.width(), glBackgroundImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glBackgroundImage.bits());
    f->glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    if(isMipmapMinFilter(_textureParam))
        f->glGenerateMipmap(GL_TEXTURE_2D);
}

void PublishGLBattleBackground::updateImageRegion(const QImage& image, const QRect& region)
{
    // Callable only from a *GL* function (applyGLPendingUpdates in playerGLPaint).
    // The caller must guarantee image.format() == Format_RGBA8888 (sourceIsRgba8888=true).
    if(!QOpenGLContext::currentContext())
        return;

    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

    f->glBindTexture(GL_TEXTURE_2D, _textureID);

    // Set the unpacking row stride so the driver reads the correct sub-row from
    // the full-image scan-line buffer without requiring a pixel-pack copy.
    f->glPixelStorei(GL_UNPACK_ROW_LENGTH, image.bytesPerLine() / BYTES_PER_PIXEL_RGBA);

    // No-CPU-flip path: loadTexture() uploaded image.bits() directly via glTexImage2D,
    // so QImage scanLine(R) lives at GL texel row R. glTexSubImage2D addresses the same
    // row directly (yoffset = region.y()). The V-coordinate inversion in
    // createImageObjects() compensates at sample time, not storage time.
    f->glTexSubImage2D(GL_TEXTURE_2D, 0,
                       region.x(), region.y(), region.width(), region.height(),
                       GL_RGBA, GL_UNSIGNED_BYTE,
                       image.constScanLine(region.y()) + region.x() * BYTES_PER_PIXEL_RGBA);

    // GL_UNPACK_ROW_LENGTH must be restored to 0 to avoid bleeding state into
    // subsequent callers' glTexImage2D calls.
    f->glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    if(isMipmapMinFilter(_textureParam))
        f->glGenerateMipmap(GL_TEXTURE_2D);
}
