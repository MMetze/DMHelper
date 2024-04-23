#include "publishglbattletoken.h"
#include "battledialogmodelcombatant.h"
#include "battledialogmodeleffect.h"
#include "publishglimage.h"
#include "publishgltokenhighlighteffect.h"
#include "publishgltokenhighlightref.h"
#include "layertokens.h"
#include "dmh_opengl.h"
#include <QOpenGLContext>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QDebug>

PublishGLBattleToken::PublishGLBattleToken(PublishGLScene* scene, BattleDialogModelCombatant* combatant, bool isPC) :
    PublishGLBattleObject(scene),
    _combatant(combatant),
    _VAO(0),
    _VBO(0),
    _EBO(0),
    _textureSize(),
    _isPC(isPC),
    _highlightList(),
    _recreateToken(false)
{
    if((!QOpenGLContext::currentContext()) || (!_combatant))
        return;

    createTokenObjects();

    connect(_combatant, &BattleDialogModelObject::objectMoved, this, &PublishGLBattleToken::combatantMoved);
    connect(_combatant, &BattleDialogModelCombatant::combatantSelected, this, &PublishGLBattleToken::combatantSelected);
    connect(_combatant, &BattleDialogModelCombatant::conditionsChanged, this, &PublishGLBattleToken::recreateToken);
    connect(_combatant, &BattleDialogModelCombatant::visibilityChanged, this, &PublishGLBattleToken::changed);
}

PublishGLBattleToken::~PublishGLBattleToken()
{
    PublishGLBattleToken::cleanup();
}

void PublishGLBattleToken::cleanup()
{
//    qDebug() << "[PublishGLBattleToken] Cleaning up image object. VAO: " << _VAO << ", VBO: " << _VBO << ", EBO: " << _EBO << ", texture: " << _textureID;

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

    qDeleteAll(_highlightList);
    _highlightList.clear();

    PublishGLBattleObject::cleanup();
}

void PublishGLBattleToken::paintGL(QOpenGLFunctions* functions, const GLfloat* projectionMatrix)
{
    Q_UNUSED(projectionMatrix);

    if((!QOpenGLContext::currentContext()) || (!functions))
        return;

    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if(!e)
        return;

    if(_recreateToken)
    {
        _recreateToken = false;
        cleanup();
        createTokenObjects();
    }

    e->glBindVertexArray(_VAO);
    functions->glBindTexture(GL_TEXTURE_2D, _textureID);
    functions->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void PublishGLBattleToken::paintEffects(int shaderModelMatrix)
{
    if(!QOpenGLContext::currentContext())
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    if(!f)
        return;

    foreach(PublishGLTokenHighlight* effect, _highlightList)
    {
        if(effect)
            effect->paintGL(f, shaderModelMatrix);
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

void PublishGLBattleToken::addHighlight(PublishGLImage& highlightImage)
{
    if((!_combatant) || (!_combatant->getLayer()))
        return;

    PublishGLTokenHighlightRef* newHighlight = new PublishGLTokenHighlightRef(highlightImage);

    QVector3D newPosition(sceneToWorld(_combatant->getPosition()));
    qreal sizeFactor = (static_cast<qreal>(_combatant->getLayer()->getScale()-2)) * _combatant->getSizeFactor();
    newHighlight->setPositionScale(newPosition, sizeFactor);

    _highlightList.append(newHighlight);
}

void PublishGLBattleToken::removeHighlight(const PublishGLImage& highlightImage)
{
    for(int i = 0; i < _highlightList.count(); ++i)
    {
        PublishGLTokenHighlightRef* highlight = dynamic_cast<PublishGLTokenHighlightRef*>(_highlightList.at(i));
        if((highlight) && (highlight->getImage() == highlightImage))
        {
            PublishGLTokenHighlight* removeHighlight = _highlightList.takeAt(i);
            delete removeHighlight;
            return;
        }
    }
}

void PublishGLBattleToken::addEffectHighlight(BattleDialogModelEffect* effect)
{
    if((!effect) || (!_combatant) || (!_combatant->getLayer()))
        return;

    PublishGLTokenHighlightEffect* newEffect = new PublishGLTokenHighlightEffect(nullptr, effect);

    QVector3D newPosition(sceneToWorld(_combatant->getPosition()));
    qreal sizeFactor = (static_cast<qreal>(_combatant->getLayer()->getScale()-2)) * _combatant->getSizeFactor();
    newEffect->setPositionScale(newPosition, sizeFactor);

    _highlightList.append(newEffect);
}

void PublishGLBattleToken::removeEffectHighlight(BattleDialogModelEffect* effect)
{
    for(int i = 0; i < _highlightList.count(); ++i)
    {
        PublishGLTokenHighlightEffect* highlight = dynamic_cast<PublishGLTokenHighlightEffect*>(_highlightList.at(i));
        if((highlight) && (highlight->getEffect() == effect))
        {
            PublishGLTokenHighlight* removeHighlight = _highlightList.takeAt(i);
            delete removeHighlight;
        }
    }
}

bool PublishGLBattleToken::hasEffectHighlight(BattleDialogModelEffect* effect)
{
    for(int i = 0; i < _highlightList.count(); ++i)
    {
        PublishGLTokenHighlightEffect* highlight = dynamic_cast<PublishGLTokenHighlightEffect*>(_highlightList.at(i));
        if((highlight) && (highlight->getEffect() == effect))
        {
            return true;
        }
    }

    return false;
}

void PublishGLBattleToken::combatantMoved()
{
    if((!_combatant) || (!_combatant->getLayer()) || (_textureSize.isEmpty()))
        return;

    QVector3D newPosition(sceneToWorld(_combatant->getPosition()));
    qreal sizeFactor = (static_cast<qreal>(_combatant->getLayer()->getScale()-2)) * _combatant->getSizeFactor();
    qreal scaleFactor = sizeFactor / qMax(_textureSize.width(), _textureSize.height());

    _modelMatrix.setToIdentity();
    _modelMatrix.translate(newPosition);
    _modelMatrix.rotate(_combatant->getRotation(), 0.f, 0.f, -1.f);
    _modelMatrix.scale(scaleFactor, scaleFactor);

    foreach(PublishGLTokenHighlight* effect, _highlightList)
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

void PublishGLBattleToken::recreateToken()
{
    _recreateToken = true;
    emit changed();
}

void PublishGLBattleToken::createTokenObjects()
{
    if(!_combatant)
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    QPixmap pix = _combatant->getIconPixmap(DMHelper::PixmapSize_Battle);
    if(_combatant->hasCondition(Combatant::Condition_Unconscious))
    {
        QImage originalImage = pix.toImage();
        QImage grayscaleImage = originalImage.convertToFormat(QImage::Format_Grayscale8);
        pix = QPixmap::fromImage(grayscaleImage);
    }
    Combatant::drawConditions(&pix, _combatant->getConditions());
    QImage textureImage = pix.toImage().convertToFormat(QImage::Format_RGBA8888).mirrored();
    _textureSize = textureImage.size();

    float vertices[] = {
        // positions    // colors           // texture coords
         (float)_textureSize.width() / 2.f,  (float)_textureSize.height() / 2.f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,   // top right
         (float)_textureSize.width() / 2.f, -(float)_textureSize.height() / 2.f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,   // bottom right
        -(float)_textureSize.width() / 2.f, -(float)_textureSize.height() / 2.f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -(float)_textureSize.width() / 2.f,  (float)_textureSize.height() / 2.f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f    // top left
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
}
