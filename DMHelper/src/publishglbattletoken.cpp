#include "publishglbattletoken.h"
#include "battledialogmodelcombatant.h"
#include "publishglimage.h"
#include "publishgleffect.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QImage>
#include <QPixmap>

PublishGLBattleToken::PublishGLBattleToken(PublishGLBattleScene* scene, BattleDialogModelCombatant* combatant, bool isPC) :
    PublishGLBattleObject(scene),
    _combatant(combatant),
    _VAO(0),
    _VBO(0),
    _EBO(0),
    _textureSize(),
    _isPC(isPC),
    _effectList()
{
    if(!QOpenGLContext::currentContext())
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    if(!_combatant)
        return;

    QPixmap pix = combatant->getIconPixmap(DMHelper::PixmapSize_Battle);
    if(combatant->hasCondition(Combatant::Condition_Unconscious))
    {
        QImage originalImage = pix.toImage();
        QImage grayscaleImage = originalImage.convertToFormat(QImage::Format_Grayscale8);
        pix = QPixmap::fromImage(grayscaleImage);
    }
    Combatant::drawConditions(&pix, combatant->getConditions());
    QImage textureImage = pix.toImage().convertToFormat(QImage::Format_RGBA8888).mirrored();
    _textureSize = textureImage.size();

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
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureImage.width(), textureImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureImage.bits());
    f->glGenerateMipmap(GL_TEXTURE_2D);

    // set the initial position matrix
    combatantMoved();

    connect(_combatant, &BattleDialogModelCombatant::combatantMoved, this, &PublishGLBattleToken::combatantMoved);
    connect(_combatant, &BattleDialogModelCombatant::combatantSelected, this, &PublishGLBattleToken::combatantSelected);
}

PublishGLBattleToken::~PublishGLBattleToken()
{
    PublishGLBattleToken::cleanup();
}

void PublishGLBattleToken::cleanup()
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

    qDeleteAll(_effectList);

    PublishGLBattleObject::cleanup();
}

void PublishGLBattleToken::paintGL()
{
    if(!QOpenGLContext::currentContext())
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    e->glBindVertexArray(_VAO);
    f->glBindTexture(GL_TEXTURE_2D, _textureID);
    f->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);    
}

void PublishGLBattleToken::paintEffects(int shaderModelMatrix)
{
    if(!QOpenGLContext::currentContext())
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

    for(PublishGLEffect* effect : _effectList)
    {
        if(effect)
        {
            f->glUniformMatrix4fv(shaderModelMatrix, 1, GL_FALSE, effect->getMatrixData());
            effect->getImage().paintGL();
        }
    }
}

BattleDialogModelCombatant* PublishGLBattleToken::getCombatant() const
{
    return _combatant;
}

QSizeF PublishGLBattleToken::getTextureSize() const
{
    return _textureSize;
}

bool PublishGLBattleToken::isPC() const
{
    return _isPC;
}

void PublishGLBattleToken::addEffect(PublishGLImage& effectImage)
{
    PublishGLEffect* newEffect = new PublishGLEffect(effectImage);

    QVector3D newPosition(sceneToWorld(_combatant->getPosition()));
    qreal sizeFactor = (static_cast<qreal>(_scene->getGridScale()-2)) * _combatant->getSizeFactor();
    newEffect->setPositionScale(newPosition, sizeFactor);

    _effectList.append(newEffect);
}

void PublishGLBattleToken::removeEffect(const PublishGLImage& effectImage)
{
    for(int i = 0; i < _effectList.count(); ++i)
    {
        if((_effectList.at(i)) && (_effectList.at(i)->getImage() == effectImage))
        {
            PublishGLEffect* removeEffect = _effectList.takeAt(i);
            delete removeEffect;
            return;
        }
    }
}

void PublishGLBattleToken::combatantMoved()
{
    if((!_scene) || (_textureSize.isEmpty()))
        return;

    QVector3D newPosition(sceneToWorld(_combatant->getPosition()));
    qreal sizeFactor = (static_cast<qreal>(_scene->getGridScale()-2)) * _combatant->getSizeFactor();
    qreal scaleFactor = sizeFactor / qMax(_textureSize.width(), _textureSize.height());

    _modelMatrix.setToIdentity();
    _modelMatrix.translate(newPosition);
    _modelMatrix.scale(scaleFactor, scaleFactor);

    for(PublishGLEffect* effect : _effectList)
        effect->setPositionScale(newPosition, sizeFactor);

    emit changed();
}

void PublishGLBattleToken::combatantSelected()
{
    emit selectionChanged(this);
}

void PublishGLBattleToken::setPC(bool isPC)
{
    if(isPC != _isPC)
    {
        _isPC = isPC;
        emit changed();
    }
}