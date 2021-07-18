#include "battlegleffect.h"
#include "battledialogmodeleffect.h"
#include "battledialogmodeleffectobject.h"
#include "scaledpixmap.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QImage>
#include <QPixmap>
#include <QPainter>

BattleGLEffect::BattleGLEffect(BattleGLScene& scene, BattleDialogModelEffect* effect) :
    BattleGLObject(scene),
    _effect(effect),
    _childEffect(nullptr),
    _VAO(0),
    _VBO(0),
    _EBO(0),
    _textureSize(),
    _imageScaleFactor(1.0)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    if(!_effect)
        return;

    QList<CampaignObjectBase*> childEffects = _effect->getChildObjects();
    if(childEffects.count() == 1)
        _childEffect = dynamic_cast<BattleDialogModelEffectObject*>(childEffects.at(0));

    QSize effectSize(DMHelper::PixmapSizes[DMHelper::PixmapSize_Battle][0] * (_effect->getSize() / 5),
                     DMHelper::PixmapSizes[DMHelper::PixmapSize_Battle][1] * (_effect->getSize() / 5));
    QImage effectImage(effectSize, QImage::Format_RGBA8888);
    effectImage.fill(QColor(0, 0, 0, 0));
    QPainter painter;
    painter.begin(&effectImage);
        painter.setPen(QPen(QColor(_effect->getColor().red(), _effect->getColor().green(), _effect->getColor().blue(), 255), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(QBrush(_effect->getColor()));
        painter.drawEllipse(effectImage.rect());

        if(_childEffect)
        {
            QPixmap itemPixmap(_childEffect->getImageFile());
            if(!itemPixmap.isNull())
            {
                //_imageScaleFactor = 100.0 / itemPixmap.width();
                if(_childEffect->getImageRotation() != 0)
                    itemPixmap = itemPixmap.transformed(QTransform().rotate(_childEffect->getImageRotation()));//.scale(_imageScaleFactor, _imageScaleFactor));
            }
            painter.drawPixmap(effectImage.rect(), itemPixmap);
        }

    painter.end();
    _textureSize = effectImage.size();

    float vertices[] = {
        // positions    // colors           // texture coords
         (float)_textureSize.width() / 2.f,  (float)_textureSize.height() / 2.f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
         (float)_textureSize.width() / 2.f, -(float)_textureSize.height() / 2.f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -(float)_textureSize.width() / 2.f, -(float)_textureSize.height() / 2.f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -(float)_textureSize.width() / 2.f,  (float)_textureSize.height() / 2.f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
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
    f->glBindTexture(GL_TEXTURE_2D, _textureID);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load and generate the background texture
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, effectImage.width(), effectImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, effectImage.bits());
    f->glGenerateMipmap(GL_TEXTURE_2D);

    // set the initial position matrix
    effectMoved();

    connect(_effect, &BattleDialogModelEffect::effectMoved, this, &BattleGLEffect::effectMoved);
    if(_childEffect)
        connect(_childEffect, &BattleDialogModelEffect::effectMoved, this, &BattleGLEffect::effectMoved);
}

BattleGLEffect::~BattleGLEffect()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    if(_VAO > 0)
        e->glDeleteVertexArrays(1, &_VAO);

    if(_VBO > 0)
        f->glDeleteBuffers(1, &_VBO);

    if(_EBO > 0)
        f->glDeleteBuffers(1, &_EBO);
}

void BattleGLEffect::paintGL()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    e->glBindVertexArray(_VAO);
    f->glBindTexture(GL_TEXTURE_2D, _textureID);
    f->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void BattleGLEffect::effectMoved()
{
    BattleDialogModelEffect* effect = _childEffect ? _childEffect : _effect;

    QPointF effectPos = effect->getPosition();
    QRectF sceneRect = _scene.getSceneRect();
    qreal sizeFactor = effect->getSize() / 5;
    qreal scaleFactor = (static_cast<qreal>(_scene.getGridScale())) * sizeFactor / qMax(_textureSize.width(), _textureSize.height());

    _modelMatrix.setToIdentity();
    _modelMatrix.translate(effectPos.x() - (sceneRect.width() / 2), (sceneRect.height() / 2) - effectPos.y());
    _modelMatrix.scale(scaleFactor, scaleFactor);

    emit changed();
}
