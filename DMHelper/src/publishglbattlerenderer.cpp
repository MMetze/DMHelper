#include "publishglbattlerenderer.h"
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

PublishGLBattleRenderer::PublishGLBattleRenderer(BattleDialogModel* model) :
    PublishGLRenderer(),
    _initialized(false),
    _model(model),
    _scene(),
    _projectionMatrix(),
    _cameraRect(),
    _scissorRect(),
    _shaderProgram(0),
    _shaderModelMatrix(0),
    _backgroundObject(nullptr),
    _fowObject(nullptr),
    _pcTokens(),
    _enemyTokens(),
    _effectTokens(),
    _updateFow(false)
{
}

PublishGLBattleRenderer::~PublishGLBattleRenderer()
{
    cleanup();
}

CampaignObjectBase* PublishGLBattleRenderer::getObject()
{
    return _model;
}

QColor PublishGLBattleRenderer::getBackgroundColor()
{
    return _model ? _model->getBackgroundColor() : QColor();
}

void PublishGLBattleRenderer::cleanup()
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

    _projectionMatrix.setToIdentity();

    if(_shaderProgram > 0)
    {
        if((_targetWidget) && (_targetWidget->context()))
        {
            QOpenGLFunctions *f = _targetWidget->context()->functions();
            if(f)
                f->glDeleteProgram(_shaderProgram);
        }
        _shaderProgram = 0;
    }
    _shaderModelMatrix = 0;

    PublishGLRenderer::cleanup();
}

bool PublishGLBattleRenderer::deleteOnDeactivation()
{
    return true;
}

void PublishGLBattleRenderer::setBackgroundColor(const QColor& color)
{
    if(_model)
    {
        _model->setBackgroundColor(color);
        emit updateWidget();
    }
}

void PublishGLBattleRenderer::initializeGL()
{
    if((_initialized) || (!_model) || (_model->getBackgroundImage().isNull()) || (!_targetWidget) || (!_targetWidget->context()))
        return;

    _scene.setGridScale(_model->getGridScale());

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

    //f->glEnable(GL_TEXTURE_2D); // Enable texturing
    //f->glEnable(GL_BLEND);// you enable blending function
    //f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
    _shaderModelMatrix = f->glGetUniformLocation(_shaderProgram, "model");

    // Create the objects
    _scene.deriveSceneRectFromSize(_model->getBackgroundImage().size());
    _backgroundObject = new BattleGLBackground(&_scene, _model->getBackgroundImage(), GL_LINEAR);

    if(_model->getMap())
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
            connect(combatantToken, &BattleGLObject::changed, this, &PublishGLBattleRenderer::updateWidget);
        }
    }

    for(int i = 0; i < _model->getEffectCount(); ++i)
    {
        BattleDialogModelEffect* effect = _model->getEffect(i);
        if(effect)
        {
            BattleGLObject* effectToken = new BattleGLEffect(&_scene, effect);
            _effectTokens.append(effectToken);
            connect(effectToken, &BattleGLObject::changed, this, &PublishGLBattleRenderer::updateWidget);
        }
    }

    // Check if we need a pointer
    evaluatePointer();

    // Matrices
    // Model
    QMatrix4x4 modelMatrix;
    f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, modelMatrix.constData());
    // View
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, 500.f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "view"), 1, GL_FALSE, viewMatrix.constData());
    // Projection - note, this is set later when resizing the window
    updateProjectionMatrix();

    f->glUseProgram(_shaderProgram);
    f->glUniform1i(f->glGetUniformLocation(_shaderProgram, "texture1"), 0); // set it manually

    _initialized = true;
}

void PublishGLBattleRenderer::resizeGL(int w, int h)
{
    QSize targetSize(w, h);
    if(_scene.getTargetSize() == targetSize)
        return;

    qDebug() << "[BattleGLRenderer] Resize to: " << targetSize;
    _scene.setTargetSize(targetSize);

    updateProjectionMatrix();
    emit updateWidget();
}

void PublishGLBattleRenderer::paintGL()
{
    if((!_model) || (!_targetWidget) || (!_targetWidget->context()) || (!_backgroundObject))
        return;

    evaluatePointer();

    QOpenGLFunctions *f = _targetWidget->context()->functions();
    QOpenGLExtraFunctions *e = _targetWidget->context()->extraFunctions();
    if((!f) || (!e))
        return;

    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "projection"), 1, GL_FALSE, _projectionMatrix.constData());

    if(!_scissorRect.isEmpty())
    {
        f->glEnable(GL_SCISSOR_TEST);
        f->glScissor(_scissorRect.x(), _scissorRect.y(), _scissorRect.width(), _scissorRect.height());
    }

    // Draw the scene:
    f->glClearColor(_model->getBackgroundColor().redF(), _model->getBackgroundColor().greenF(), _model->getBackgroundColor().blueF(), 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    f->glUseProgram(_shaderProgram);
    f->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture

    if(_backgroundObject)
    {
        f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, _backgroundObject->getMatrixData());
        _backgroundObject->paintGL();
    }

    for(BattleGLObject* enemyToken : _enemyTokens)
    {
        f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, enemyToken->getMatrixData());
        enemyToken->paintGL();
    }

    for(BattleGLObject* effectToken : _effectTokens)
    {
        f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, effectToken->getMatrixData());
        effectToken->paintGL();
    }

    if(_fowObject)
    {
        if((_updateFow) && (_model->getMap()))
        {
            _fowObject->setImage(_model->getMap()->getBWFoWImage());
            _updateFow = false;
        }
        f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, _fowObject->getMatrixData());
        _fowObject->paintGL();
    }

    for(BattleGLObject* pcToken : _pcTokens)
    {
        f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, pcToken->getMatrixData());
        pcToken->paintGL();
    }

    if(!_scissorRect.isEmpty())
        f->glDisable(GL_SCISSOR_TEST);

    paintPointer(f, _backgroundObject->getSize(), _shaderModelMatrix);
}

void PublishGLBattleRenderer::fowChanged()
{
    _updateFow = true;
    emit updateWidget();
}

void PublishGLBattleRenderer::setCameraRect(const QRectF& cameraRect)
{
    if(_cameraRect != cameraRect)
    {
        _cameraRect = cameraRect;
        updateProjectionMatrix();
        emit updateWidget();
    }
}

void PublishGLBattleRenderer::updateProjectionMatrix()
{
    if((!_model) || (_scene.getTargetSize().isEmpty()) || (_shaderProgram == 0) || (!_targetWidget) || (!_targetWidget->context()))
        return;

    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

    // Update projection matrix and other size related settings:
    QRectF transformedCamera = _cameraRect;
    QSizeF transformedTarget = _scene.getTargetSize();
    if((_rotation == 90) || (_rotation == 270))
    {
        transformedCamera = transformedCamera.transposed();
        transformedCamera.moveTo(transformedCamera.topLeft().transposed());
        transformedTarget.transpose();
    }

    QSizeF rectSize = transformedTarget.scaled(_cameraRect.size(), Qt::KeepAspectRatioByExpanding);
    QSizeF halfRect = rectSize / 2.0;
    QPointF cameraTopLeft((rectSize.width() - _cameraRect.width()) / 2.0, (rectSize.height() - _cameraRect.height()) / 2);
    QPointF cameraMiddle(_cameraRect.x() + (_cameraRect.width() / 2.0), _cameraRect.y() + (_cameraRect.height() / 2.0));
    QSizeF backgroundMiddle = _scene.getSceneRect().size() / 2.0;

    //qDebug() << "[PublishGLMapImageRenderer] camera rect: " << _cameraRect << ", transformed camera: " << transformedCamera << ", target size: " << _scene.getTargetSize() << ", transformed target: " << transformedTarget;
    //qDebug() << "[PublishGLMapImageRenderer] rectSize: " << rectSize << ", camera top left: " << cameraTopLeft << ", camera middle: " << cameraMiddle << ", background middle: " << backgroundMiddle;

    _projectionMatrix.setToIdentity();
    _projectionMatrix.rotate(_rotation, 0.0, 0.0, -1.0);
    _projectionMatrix.ortho(cameraMiddle.x() - backgroundMiddle.width() - halfRect.width(), cameraMiddle.x() - backgroundMiddle.width() + halfRect.width(),
                            backgroundMiddle.height() - cameraMiddle.y() - halfRect.height(), backgroundMiddle.height() - cameraMiddle.y() + halfRect.height(),
                            0.1f, 1000.f);

    setPointerScale(rectSize.width() / transformedTarget.width());

    QSizeF scissorSize = transformedCamera.size().scaled(_scene.getTargetSize(), Qt::KeepAspectRatio);
    //qDebug() << "[PublishGLMapImageRenderer] scissor size: " << scissorSize;
    _scissorRect.setX((_scene.getTargetSize().width() - scissorSize.width()) / 2.0);
    _scissorRect.setY((_scene.getTargetSize().height() - scissorSize.height()) / 2.0);
    _scissorRect.setWidth(scissorSize.width());
    _scissorRect.setHeight(scissorSize.height());

    /*
    // Update projection matrix and other size related settings:
    QSizeF rectSize = QSizeF(_scene.getTargetSize()).scaled(_scene.getSceneRect().size(), Qt::KeepAspectRatioByExpanding);
    QMatrix4x4 projectionMatrix;
    projectionMatrix.ortho(-rectSize.width() / 2, rectSize.width() / 2, -rectSize.height() / 2, rectSize.height() / 2, 0.1f, 1000.f);
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgram, "projection"), 1, GL_FALSE, projectionMatrix.constData());

    setPointerScale(rectSize.width() / _scene.getTargetSize().width());
    */
}
