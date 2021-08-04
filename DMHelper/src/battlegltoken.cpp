#include "battlegltoken.h"
#include "battledialogmodelcombatant.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QImage>
#include <QPixmap>

BattleGLToken::BattleGLToken(BattleGLScene* scene, BattleDialogModelCombatant* combatant) :
    BattleGLObject(scene),
    _combatant(combatant),
    _VAO(0),
    _VBO(0),
    _EBO(0),
    _textureSize()
{
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

    connect(_combatant, &BattleDialogModelCombatant::combatantMoved, this, &BattleGLToken::combatantMoved);
}

BattleGLToken::~BattleGLToken()
{
    BattleGLToken::cleanup();
}

void BattleGLToken::cleanup()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    if(_VAO > 0)
    {
        e->glDeleteVertexArrays(1, &_VAO);
        _VAO = 0;
    }

    if(_VBO > 0)
    {
        f->glDeleteBuffers(1, &_VBO);
        _VBO = 0;
    }

    if(_EBO > 0)
    {
        f->glDeleteBuffers(1, &_EBO);
        _EBO = 0;
    }

    BattleGLObject::cleanup();
}

void BattleGLToken::paintGL()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    if((!f) || (!e))
        return;

    e->glBindVertexArray(_VAO);
    f->glBindTexture(GL_TEXTURE_2D, _textureID);
    f->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void BattleGLToken::combatantMoved()
{
    if(!_scene)
        return;

    QPointF combatantPos = _combatant->getPosition();
    QRectF sceneRect = _scene->getSceneRect();
    qreal sizeFactor = _combatant->getSizeFactor();
    qreal scaleFactor = (static_cast<qreal>(_scene->getGridScale()-2)) * sizeFactor / qMax(_textureSize.width(), _textureSize.height());

    _modelMatrix.setToIdentity();
    _modelMatrix.translate(combatantPos.x() - (sceneRect.width() / 2), (sceneRect.height() / 2) - combatantPos.y());
    _modelMatrix.scale(scaleFactor, scaleFactor);

    emit changed();
}
