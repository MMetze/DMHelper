#include "battleglrenderer.h"
#include "battledialogmodel.h"
#include "battledialogmodelcharacter.h"
#include "battledialogmodeleffect.h"
#include "battleglbackground.h"
#include "battlegltoken.h"
#include "battlegleffect.h"
#include "map.h"
#include "character.h"
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QDebug>

BattleGLRenderer::BattleGLRenderer(BattleDialogModel* model) :
    PublishGLRenderer(),
    _initialized(false),
    _model(model),
    _scene(),
    _shaderProgram(0),
    _backgroundObject(nullptr),
    _fowObject(nullptr),
    _pcTokens(),
    _enemyTokens(),
    _effectTokens()
{
}

BattleGLRenderer::~BattleGLRenderer()
{
    cleanup();
}

void BattleGLRenderer::cleanup()
{
    _initialized = false;

    delete _backgroundObject;
    _backgroundObject = nullptr;
    delete _fowObject;
    _fowObject = nullptr;

    qDeleteAll(_pcTokens);
    _pcTokens.clear();
    qDeleteAll(_enemyTokens);
    _enemyTokens.clear();
    qDeleteAll(_effectTokens);
    _effectTokens.clear();
}

void BattleGLRenderer::initializeGL()
{
    if((_initialized) || (!_model) || (_model->getBackgroundImage().isNull()) || (!_targetWidget))
        return;

    _scene.setGridScale(_model->getGridScale());

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

    f->glEnable(GL_TEXTURE_2D); // Enable texturing
    f->glEnable(GL_BLEND);// you enable blending function
    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //f->glEnable(GL_DEPTH_TEST);
    //f->glDepthFunc(GL_GREATER);

    const char *vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0\n"
        "layout (location = 1) in vec3 aColor; // the color variable has attribute position 1\n"
        "layout (location = 2) in vec2 aTexCoord;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "out vec3 ourColor; // output a color to the fragment shader\n"
        "out vec2 TexCoord;\n"
        "void main()\n"
        "{\n"
        "   // note that we read the multiplication from right to left\n"
        "   gl_Position = projection * view * model * vec4(aPos, 1.0); // gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "   ourColor = aColor; // set ourColor to the input color we got from the vertex data\n"
        "   TexCoord = aTexCoord;\n"
        "}\0";

    unsigned int vertexShader;
    vertexShader = f->glCreateShader(GL_VERTEX_SHADER);
    f->glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    f->glCompileShader(vertexShader);

    int  success;
    char infoLog[512];
    f->glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        qDebug() << "[BattleGLRenderer] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
        return;
    }

    const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec3 ourColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D texture1;\n"
        "void main()\n"
        "{\n"
        "    FragColor = texture(texture1, TexCoord); // FragColor = vec4(ourColor, 1.0f);\n"
        "}\0";

    unsigned int fragmentShader;
    fragmentShader = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    f->glCompileShader(fragmentShader);

    f->glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        qDebug() << "[BattleGLRenderer] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    _shaderProgram = f->glCreateProgram();

    f->glAttachShader(_shaderProgram, vertexShader);
    f->glAttachShader(_shaderProgram, fragmentShader);
    f->glLinkProgram(_shaderProgram);

    f->glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        f->glGetProgramInfoLog(_shaderProgram, 512, NULL, infoLog);
        qDebug() << "[BattleGLRenderer] ERROR::SHADER::PROGRAM::COMPILATION_FAILED: " << infoLog;
        return;
    }

    f->glUseProgram(_shaderProgram);
    f->glDeleteShader(vertexShader);
    f->glDeleteShader(fragmentShader);

    // Create the objects
    _scene.deriveSceneRectFromSize(_model->getBackgroundImage().size());
    _backgroundObject = new BattleGLBackground(&_scene, _model->getBackgroundImage(), GL_NEAREST);
    _fowObject = new BattleGLBackground(&_scene, _model->getMap()->getBWFoWImage(), GL_LINEAR);
    for(int i = 0; i < _model->getCombatantCount(); ++i)
    {
        BattleDialogModelCombatant* combatant = _model->getCombatant(i);
        if(combatant)
        {
            BattleGLObject* combatantToken = new BattleGLToken(&_scene, combatant);
            BattleDialogModelCharacter* characterCombatant = dynamic_cast<BattleDialogModelCharacter*>(combatant);
            if((characterCombatant) && (characterCombatant->getCharacter()) && (characterCombatant->getCharacter()->isInParty()))
                _pcTokens.append(combatantToken);
            else
                _enemyTokens.append(combatantToken);
            connect(combatantToken, &BattleGLObject::changed, this, &BattleGLRenderer::updateWidget);
        }
    }

    for(int i = 0; i < _model->getEffectCount(); ++i)
    {
        BattleDialogModelEffect* effect = _model->getEffect(i);
        if(effect)
        {
            BattleGLObject* effectToken = new BattleGLEffect(&_scene, effect);
            _effectTokens.append(effectToken);
            connect(effectToken, &BattleGLObject::changed, this, &BattleGLRenderer::updateWidget);
        }
    }

    // Matrices
    // Model
    QMatrix4x4 modelMatrix;
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "model"), 1, GL_FALSE, modelMatrix.constData());
    // View
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, 500.f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "view"), 1, GL_FALSE, viewMatrix.constData());
    // Projection - note, this is set later when resizing the window
    setOrthoProjection();

    f->glUseProgram(_shaderProgram);
    f->glUniform1i(f->glGetUniformLocation(_shaderProgram, "texture1"), 0); // set it manually

    _initialized = true;
}

void BattleGLRenderer::resizeGL(int w, int h)
{
    _scene.setTargetSize(QSize(w, h));
    qDebug() << "[BattleGLRenderer] Resize w: " << w << ", h: " << h;

    setOrthoProjection();
    emit updateWidget();
}

void BattleGLRenderer::paintGL()
{
    if((!_model) || (!_targetWidget) || (!_targetWidget->context()))
        return;

    //QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    QOpenGLFunctions *f = _targetWidget->context()->functions();
    QOpenGLExtraFunctions *e = _targetWidget->context()->extraFunctions();
    if((!f) || (!e))
        return;

    // Draw the scene:
    f->glClearColor(_model->getBackgroundColor().redF(), _model->getBackgroundColor().greenF(), _model->getBackgroundColor().blueF(), 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    f->glUseProgram(_shaderProgram);
    f->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture

    if(_backgroundObject)
    {
        f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "model"), 1, GL_FALSE, _backgroundObject->getMatrixData());
        _backgroundObject->paintGL();
    }

    for(BattleGLObject* enemyToken : _enemyTokens)
    {
        f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "model"), 1, GL_FALSE, enemyToken->getMatrixData());
        enemyToken->paintGL();
    }

    for(BattleGLObject* effectToken : _effectTokens)
    {
        f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "model"), 1, GL_FALSE, effectToken->getMatrixData());
        effectToken->paintGL();
    }

    if(_fowObject)
    {
        f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "model"), 1, GL_FALSE, _fowObject->getMatrixData());
        _fowObject->paintGL();
    }

    for(BattleGLObject* pcToken : _pcTokens)
    {
        f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "model"), 1, GL_FALSE, pcToken->getMatrixData());
        pcToken->paintGL();
    }
}

void BattleGLRenderer::setOrthoProjection()
{
    if((!_model) || (_scene.getTargetSize().isEmpty()) || (_shaderProgram == 0) || (!_targetWidget) || (!_targetWidget->context()))
        return;

    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

    // Update projection matrix and other size related settings:
    QSizeF rectSize = QSizeF(_scene.getTargetSize()).scaled(_scene.getSceneRect().size(), Qt::KeepAspectRatioByExpanding);
    QMatrix4x4 projectionMatrix;
    projectionMatrix.ortho(-rectSize.width() / 2, rectSize.width() / 2, -rectSize.height() / 2, rectSize.height() / 2, 0.1f, 1000.f);
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "projection"), 1, GL_FALSE, projectionMatrix.constData());
}


/*
// 0. copy our vertices array in a buffer for OpenGL to use
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
// 1. then set the vertex attributes pointers
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);
// 2. use our shader program when we want to render an object
glUseProgram(shaderProgram);
// 3. now draw the object
someOpenGLFunctionThatDrawsOurTriangle();
*/
