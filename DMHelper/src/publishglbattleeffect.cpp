#include "publishglbattleeffect.h"
#include "battledialogmodeleffect.h"
#include "battledialogmodeleffectobject.h"
#include "scaledpixmap.h"
#include "layertokens.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QImage>
#include <QPixmap>
#include <QPainter>

PublishGLBattleEffect::PublishGLBattleEffect(PublishGLScene* scene, BattleDialogModelEffect* effect) :
    PublishGLBattleObject(scene),
    _effect(effect),
    _childEffect(nullptr),
    _VAO(0),
    _VBO(0),
    _EBO(0),
    _textureSize(),
    _imageScaleFactor(1.0),
    _recreateEffect(false)
{
    if((!QOpenGLContext::currentContext()) || (!_effect))
        return;

    QList<CampaignObjectBase*> childEffects = _effect->getChildObjects();
    if(childEffects.count() == 1)
        _childEffect = dynamic_cast<BattleDialogModelEffectObject*>(childEffects.at(0));

    prepareObjects();

    connect(_effect, &BattleDialogModelObject::objectMoved, this, &PublishGLBattleEffect::effectMoved);
    connect(_effect, &BattleDialogModelEffect::effectChanged, this, &PublishGLBattleEffect::effectChanged);
    if(_childEffect)
    {
        connect(_childEffect, &BattleDialogModelObject::objectMoved, this, &PublishGLBattleEffect::effectMoved);
        connect(_childEffect, &BattleDialogModelEffect::effectChanged, this, &PublishGLBattleEffect::effectChanged);
    }
}

PublishGLBattleEffect::~PublishGLBattleEffect()
{
    PublishGLBattleEffect::cleanup();
}

void PublishGLBattleEffect::cleanup()
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

    PublishGLBattleObject::cleanup();
}

void PublishGLBattleEffect::paintGL()
{
    if(!QOpenGLContext::currentContext())
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    if(_recreateEffect)
    {
        _recreateEffect = false;
        cleanup();
        prepareObjects();
    }

    e->glBindVertexArray(_VAO);
    f->glBindTexture(GL_TEXTURE_2D, _textureID);
    f->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

BattleDialogModelEffect* PublishGLBattleEffect::getEffect() const
{
    return _effect;
}

qreal PublishGLBattleEffect::getEffectAlpha() const
{
    if(_childEffect)
        return _childEffect->getColor().alphaF();

    if((_effect) && (_effect->getEffectType() == BattleDialogModelEffect::BattleDialogModelEffect_Object))
        return _effect->getColor().alphaF();
    else
        return 1.0;
}

void PublishGLBattleEffect::effectMoved()
{
    if(!_scene)
        return;

    BattleDialogModelEffect* effect = _childEffect ? _childEffect : _effect;
    if((!_effect) || (!_effect->getLayer()))
        return;

    QPointF effectPos = effect->getPosition();
    qreal sizeFactor = static_cast<qreal>(effect->getSize()) / 5.0;
    if(effect->getEffectType() == BattleDialogModelEffect::BattleDialogModelEffect_Radius)
        sizeFactor *= 2.0; // Convert radius to diameter
    qreal scaleFactor = (static_cast<qreal>(_effect->getLayer()->getScale()-2)) * sizeFactor / qMax(_textureSize.width(), _textureSize.height());

    _modelMatrix.setToIdentity();
    _modelMatrix.translate(QVector3D(sceneToWorld(effectPos)));
    _modelMatrix.rotate(effect->getRotation(), 0.f, 0.f, -1.f);
    _modelMatrix.scale(scaleFactor, scaleFactor);

    emit changed();
}

void PublishGLBattleEffect::effectChanged()
{
    _recreateEffect = true;
    emit changed();
}

void PublishGLBattleEffect::prepareObjects()
{
    if(!_effect)
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    int effectSize = DMHelper::PixmapSizes[DMHelper::PixmapSize_Battle][0] * _effect->getSize() / 5; // Primary dimension
    int effectWidth = DMHelper::PixmapSizes[DMHelper::PixmapSize_Battle][0] * _effect->getWidth() / 5; // Secondary dimension
    if(_effect->getEffectType() == BattleDialogModelEffect::BattleDialogModelEffect_Radius)
        effectSize *= 2; // Convert radius to diameter

    QImage effectImage;
    if(_effect->getEffectType() == BattleDialogModelEffect::BattleDialogModelEffect_Line)
        effectImage = QImage(QSize(effectWidth, effectSize), QImage::Format_RGBA8888);
    else
        effectImage = QImage(QSize(effectSize, effectSize), QImage::Format_RGBA8888);

    effectImage.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&effectImage);
        painter.setPen(QPen(QColor(_effect->getColor().red(), _effect->getColor().green(), _effect->getColor().blue(), 255), 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(QBrush(_effect->getColor()));
        drawShape(painter, _effect, effectSize, effectWidth);

        if(_childEffect)
        {
            QImage itemImage(_childEffect->getImageFile());
            if(!itemImage.isNull())
            {
                if((_effect->getEffectType() == BattleDialogModelEffect::BattleDialogModelEffect_Cone) ||
                   (_effect->getEffectType() == BattleDialogModelEffect::BattleDialogModelEffect_Line))
                    itemImage = itemImage.mirrored(true, false); // mirror horizontally
                else
                    itemImage = itemImage.mirrored(false, true); // mirror vertically

                if(_childEffect->getImageRotation() != 0)
                    itemImage = itemImage.transformed(QTransform().rotate(_childEffect->getImageRotation()));

                if(_effect->getEffectType() == BattleDialogModelEffect::BattleDialogModelEffect_Line)
                    painter.drawImage(QRect(0, 0, effectWidth, effectSize), itemImage);
                else
                    painter.drawImage(effectImage.rect(), itemImage);
            }
        }
    painter.end();

    _textureSize = effectImage.size();

    float vertices[] = {
        // positions                                                                     // colors           // texture coords
         (float)_textureSize.width() / 2.f,  (float)_textureSize.height() / 2.f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,   // top right
         (float)_textureSize.width() / 2.f, -(float)_textureSize.height() / 2.f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,   // bottom right
        -(float)_textureSize.width() / 2.f, -(float)_textureSize.height() / 2.f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -(float)_textureSize.width() / 2.f,  (float)_textureSize.height() / 2.f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f    // top left
    };

    if(_effect->getEffectType() == BattleDialogModelEffect::BattleDialogModelEffect_Cube)
    {
        vertices[0]  = (float)_textureSize.width(); vertices[1]  = 0.0f;
        vertices[8]  = (float)_textureSize.width(); vertices[9]  = (float)-_textureSize.height();
        vertices[16] = 0.0f;                        vertices[17] = (float)-_textureSize.height();
        vertices[24] = 0.0f;                        vertices[25] = 0.0f;
    }
    else if((_effect->getEffectType() == BattleDialogModelEffect::BattleDialogModelEffect_Cone) ||
            (_effect->getEffectType() == BattleDialogModelEffect::BattleDialogModelEffect_Line))
    {
        /*vertices[0]  = (float)_textureSize.width(); */ vertices[1]  = 0.0f;
        /*vertices[8]  = (float)_textureSize.width(); */ vertices[9]  = (float)-_textureSize.height();
        /*vertices[16] = 0.0f;                        */ vertices[17] = (float)-_textureSize.height();
        /*vertices[24] = 0.0f;                        */ vertices[25] = 0.0f;
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
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load and generate the background texture
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, effectImage.width(), effectImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, effectImage.bits());
    f->glGenerateMipmap(GL_TEXTURE_2D);

    effectMoved();
}

void PublishGLBattleEffect::drawShape(QPainter& painter, BattleDialogModelEffect* effect, int effectSize, int effectWidth)
{
    if(!effect)
        return;

    switch(effect->getEffectType())
    {
        case BattleDialogModelEffect::BattleDialogModelEffect_Radius:
            painter.drawEllipse(QRectF(0, 0, effectSize, effectSize));
            break;
        case BattleDialogModelEffect::BattleDialogModelEffect_Cone:
            {
                QPolygonF poly;
                poly << QPointF(effectSize / 2, effectSize) << QPointF(0, 0) << QPoint(effectSize, 0) << QPoint(effectSize / 2, effectSize);
                painter.drawPolygon(poly);
            }
            break;
        case BattleDialogModelEffect::BattleDialogModelEffect_Cube:
            painter.drawRect(QRectF(0, 0, effectSize, effectSize));
            break;
        case BattleDialogModelEffect::BattleDialogModelEffect_Line:
            {
                painter.drawRect(QRect(0, 0, effectWidth, effectSize));
                break;
            }
        case BattleDialogModelEffect::BattleDialogModelEffect_Object:
            drawObject(painter, dynamic_cast<BattleDialogModelEffectObject*>(effect), effectSize, effectWidth);
            break;
        default:
            break;
    }
}

void PublishGLBattleEffect::drawObject(QPainter& painter, BattleDialogModelEffectObject* effectObject, int effectSize, int effectWidth)
{
    if(!effectObject)
        return;

    QImage itemImage(effectObject->getImageFile());
    if(itemImage.isNull())
        return;

    itemImage = itemImage.mirrored(false, true); // mirror vertically
    if(effectObject->getImageRotation() != 0)
        itemImage = itemImage.transformed(QTransform().rotate(effectObject->getImageRotation()));

    painter.drawImage(QRect(0, 0, effectWidth, effectSize), itemImage);
}

