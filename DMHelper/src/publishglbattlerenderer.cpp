#include "publishglbattlerenderer.h"
#include "battledialogmodel.h"
#include "battledialogmodelcharacter.h"
#include "battledialogmodeleffect.h"
#include "publishglbattlebackground.h"
#include "publishglbattletoken.h"
#include "publishglbattleeffect.h"
#include "publishglimage.h"
#include "battledialogmodelcombatant.h"
#include "map.h"
#include "character.h"
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QPainter>
#include <QApplication>
#include <QDebug>

const int MOVEMENT_TOKEN_SIZE = 512;

PublishGLBattleRenderer::PublishGLBattleRenderer(BattleDialogModel* model, QObject *parent) :
    PublishGLRenderer(parent),
    _initialized(false),
    _model(model),
    _scene(),
    _projectionMatrix(),
    _cameraRect(),
    _scissorRect(),
    _shaderProgram(0),
    _shaderModelMatrix(0),
    _shaderProjectionMatrix(0),
    _gridImage(),
    _gridObject(nullptr),
    _fowObject(nullptr),
    _combatantTokens(),
    _combatantNames(),
    _unknownToken(nullptr),
    _initiativeBackground(nullptr),
    _effectTokens(),
    _initiativeType(DMHelper::InitiativeType_ImageName),
    _initiativeTokenHeight(0.0),
    _movementVisible(false),
    _movementCombatant(nullptr),
    _movementPC(false),
    _movementToken(nullptr),
    _activeCombatant(nullptr),
    _activePC(false),
    _activeToken(nullptr),
    _selectionToken(nullptr),
    _updateFow(false),
    _recreateContent(false)
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

    disconnect(_model, &BattleDialogModel::effectListChanged, this, &PublishGLBattleRenderer::recreateContents);
    disconnect(_model, &BattleDialogModel::activeCombatantChanged, this, &PublishGLBattleRenderer::updateWidget);
    disconnect(_model, &BattleDialogModel::activeCombatantChanged, this, &PublishGLBattleRenderer::activeCombatantChanged);
    disconnect(_model, &BattleDialogModel::combatantListChanged, this, &PublishGLBattleRenderer::recreateContents);

    cleanupContents();

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
    _shaderProjectionMatrix = 0;

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
    _shaderProjectionMatrix = f->glGetUniformLocation(_shaderProgram, "projection");

    // Create the objects
    initializeBackground();

    if(isBackgroundReady())
        createContents();

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

    connect(_model, &BattleDialogModel::combatantListChanged, this, &PublishGLBattleRenderer::recreateContents);
    connect(_model, &BattleDialogModel::activeCombatantChanged, this, &PublishGLBattleRenderer::updateWidget);
    connect(_model, &BattleDialogModel::activeCombatantChanged, this, &PublishGLBattleRenderer::activeCombatantChanged);
    connect(_model, &BattleDialogModel::effectListChanged, this, &PublishGLBattleRenderer::recreateContents);
    _initialized = true;
}

void PublishGLBattleRenderer::resizeGL(int w, int h)
{
    QSize targetSize(w, h);
    if(_scene.getTargetSize() == targetSize)
        return;

    qDebug() << "[BattleGLRenderer] Resize to: " << targetSize;
    _scene.setTargetSize(targetSize);
    resizeBackground(w, h);

    updateInitiative();

    updateProjectionMatrix();
    emit updateWidget();
}

void PublishGLBattleRenderer::paintGL()
{
    if((!_model) || (!_targetWidget) || (!_targetWidget->context()))
        return;

    if(!isBackgroundReady())
    {
        updateBackground();
        if(isBackgroundReady())
            createContents();
        else
            return;
    }

    if(_recreateContent)
    {
        cleanupContents();
        createContents();
    }
    else
    {
        updateGrid();

        if(_updateFow)
            updateFoW();
    }

    evaluatePointer();

    QOpenGLFunctions *f = _targetWidget->context()->functions();
    QOpenGLExtraFunctions *e = _targetWidget->context()->extraFunctions();
    if((!f) || (!e))
        return;

    f->glUniformMatrix4fv(_shaderProjectionMatrix, 1, GL_FALSE, _projectionMatrix.constData());

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

    paintBackground(f);

    if(_gridObject)
    {
        f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, _gridObject->getMatrixData());
        _gridObject->paintGL();
    }

    for(PublishGLBattleObject* effectToken : _effectTokens)
    {
        f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, effectToken->getMatrixData());
        effectToken->paintGL();
    }

    QList<PublishGLBattleToken*> tokens = _combatantTokens.values();
    for(PublishGLBattleToken* enemyToken : tokens)
    {
        if(!enemyToken->isPC())
        {
            f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, enemyToken->getMatrixData());
            enemyToken->paintGL();
            enemyToken->paintEffects(_shaderModelMatrix);
        }
    }

    paintTokens(f, false);

    if(_fowObject)
    {
        f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, _fowObject->getMatrixData());
        _fowObject->paintGL();
    }

    for(PublishGLBattleToken* pcToken : tokens)
    {
        if(pcToken->isPC())
        {
            f->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, pcToken->getMatrixData());
            pcToken->paintGL();
            pcToken->paintEffects(_shaderModelMatrix);
        }
    }

    paintTokens(f, true);

    if(!_scissorRect.isEmpty())
        f->glDisable(GL_SCISSOR_TEST);

    paintInitiative(f);

    f->glUniformMatrix4fv(_shaderProjectionMatrix, 1, GL_FALSE, _projectionMatrix.constData());
    paintPointer(f, getBackgroundSize().toSize(), _shaderModelMatrix);
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

void PublishGLBattleRenderer::setGrid(QImage gridImage)
{
    if(gridImage == _gridImage)
        return;

    _gridImage = gridImage;
    delete _gridObject;
    _gridObject = nullptr;

    emit updateWidget();
}

void PublishGLBattleRenderer::setInitiativeType(int initiativeType)
{
    if(_initiativeType != initiativeType)
    {
        _initiativeType = initiativeType;
        recreateContents();
    }
}

void PublishGLBattleRenderer::movementChanged(bool visible, BattleDialogModelCombatant* combatant, qreal remaining)
{
    if(!_movementToken)
        return;

    if(!combatant)
    {
        _movementVisible = false;
        _movementCombatant = nullptr;
        _movementPC = false;
    }
    else
    {
        _movementVisible = visible;
        if(combatant != _movementCombatant)
        {
            _movementCombatant = combatant;
            PublishGLBattleToken* combatantToken = _combatantTokens.value(combatant);
            _movementPC = combatantToken ? combatantToken->isPC() : false;
        }

        _movementToken->setPositionScale(PublishGLBattleObject::sceneToWorld(_scene.getSceneRect(), combatant->getPosition()), remaining / MOVEMENT_TOKEN_SIZE);
    }

    emit updateWidget();
}

void PublishGLBattleRenderer::activeCombatantChanged(BattleDialogModelCombatant* activeCombatant)
{
    if(_activeCombatant == activeCombatant)
        return;

    disconnect(_activeCombatant, &BattleDialogModelCombatant::combatantMoved, this, &PublishGLBattleRenderer::activeCombatantMoved);

    _activeCombatant = activeCombatant;
    if(_activeCombatant)
    {
        PublishGLBattleToken* combatantToken = _combatantTokens.value(_activeCombatant);
        _activePC = combatantToken ? combatantToken->isPC() : false;
        activeCombatantMoved();
        connect(_activeCombatant, &BattleDialogModelCombatant::combatantMoved, this, &PublishGLBattleRenderer::activeCombatantMoved);
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
    QSizeF backgroundMiddle = getBackgroundSize() / 2.0;

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
}

void PublishGLBattleRenderer::paintTokens(QOpenGLFunctions* functions, bool drawPCs)
{
    if((_activePC == drawPCs) && (_activeCombatant) && (_activeToken))
    {
        functions->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, _activeToken->getMatrixData());
        _activeToken->paintGL();
    }

    if((_movementPC == drawPCs) && (_movementVisible) && (_movementToken) && (_model->getShowMovement()))
    {
        functions->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, _movementToken->getMatrixData());
        _movementToken->paintGL();
    }
}

void PublishGLBattleRenderer::updateBackground()
{
}

void PublishGLBattleRenderer::updateGrid()
{
    if((_gridObject) || (_gridImage.isNull()))
        return;

    _gridObject = new PublishGLImage(_gridImage);
}

void PublishGLBattleRenderer::updateFoW()
{
    if((!_model) || (!_model->getMap()))
        return;

    QSize backgroundSize = getBackgroundSize().toSize();

    if(!backgroundSize.isEmpty())
    {
        if(!_fowObject)
            _fowObject = new PublishGLBattleBackground(nullptr, _model->getMap()->getBWFoWImage(backgroundSize), GL_NEAREST);
        else
            _fowObject->setImage(_model->getMap()->getBWFoWImage(backgroundSize));

        _updateFow = false;
    }
}

void PublishGLBattleRenderer::createContents()
{
    if(!_model)
        return;

    updateFoW();
    updateGrid();

    QImage selectImage;
    selectImage.load(QString(":/img/data/selected.png"));
    _selectionToken = new PublishGLImage(selectImage);

    QFontMetrics fm(qApp->font());
    for(int i = 0; i < _model->getCombatantCount(); ++i)
    {
        BattleDialogModelCombatant* combatant = _model->getCombatant(i);
        if(combatant)
        {
            PublishGLBattleToken* combatantToken = new PublishGLBattleToken(&_scene, combatant);
            BattleDialogModelCharacter* characterCombatant = dynamic_cast<BattleDialogModelCharacter*>(combatant);
            if((characterCombatant) && (characterCombatant->getCharacter()) && (characterCombatant->getCharacter()->isInParty()))
                combatantToken->setPC(true);
            if(combatant->getSelected())
                combatantToken->addEffect(*_selectionToken);
            _combatantTokens.insert(combatant, combatantToken);

            if(_initiativeType == DMHelper::InitiativeType_ImageName)
            {
                QRect nameBounds = fm.boundingRect(combatant->getName());
                QImage nameImage(nameBounds.size(), QImage::Format_RGBA8888);
                nameImage.fill(Qt::transparent);
                QPainter namePainter;
                namePainter.begin(&nameImage);
                    namePainter.setPen(QPen(Qt::white));
                    namePainter.drawText(0, -nameBounds.top(), combatant->getName());
                namePainter.end();
                PublishGLImage* combatantName = new PublishGLImage(nameImage, false);
                _combatantNames.insert(combatant, combatantName);
            }

            connect(combatantToken, &PublishGLBattleObject::changed, this, &PublishGLBattleRenderer::updateWidget);
            connect(combatantToken, &PublishGLBattleToken::selectionChanged, this, &PublishGLBattleRenderer::tokenSelectionChanged);
        }
    }

    updateInitiative();

    _unknownToken = new PublishGLImage(ScaledPixmap::defaultPixmap()->getPixmap(DMHelper::PixmapSize_Animate).toImage());

    QImage movementImage(QSize(MOVEMENT_TOKEN_SIZE, MOVEMENT_TOKEN_SIZE), QImage::Format_RGBA8888);
    movementImage.fill(Qt::transparent);
    QPainter movementPainter;
    movementPainter.begin(&movementImage);
        movementPainter.setPen(QPen(QColor(23,23,23,200), 3, Qt::DashDotLine));
        movementPainter.setBrush(QBrush(QColor(255,255,255,25)));
        movementPainter.drawEllipse(0, 0, 512, 512);
    movementPainter.end();
    _movementToken = new PublishGLImage(movementImage);

    QImage activeImage;
    activeImage.load(QString(":/img/data/active.png"));
    _activeToken = new PublishGLImage(activeImage);
    activeCombatantChanged(_model->getActiveCombatant());

    for(int i = 0; i < _model->getEffectCount(); ++i)
    {
        BattleDialogModelEffect* effect = _model->getEffect(i);
        if(effect)
        {
            PublishGLBattleObject* effectToken = new PublishGLBattleEffect(&_scene, effect);
            _effectTokens.append(effectToken);
            connect(effectToken, &PublishGLBattleObject::changed, this, &PublishGLBattleRenderer::updateWidget);
        }
    }

    // Check if we need a pointer
    evaluatePointer();

    _recreateContent = false;
}

void PublishGLBattleRenderer::cleanupContents()
{
    delete _gridObject; _gridObject = nullptr;
    delete _fowObject; _fowObject = nullptr;
    delete _selectionToken; _selectionToken = nullptr;
    delete _unknownToken; _unknownToken = nullptr;
    delete _initiativeBackground; _initiativeBackground = nullptr;
    delete _movementToken; _movementToken = nullptr;

    qDeleteAll(_combatantTokens); _combatantTokens.clear();
    qDeleteAll(_combatantNames); _combatantNames.clear();
    qDeleteAll(_effectTokens); _effectTokens.clear();

    _initiativeTokenHeight = 0.0;
    _movementVisible = false;
    _movementCombatant = nullptr;
    _movementPC = false;

    activeCombatantChanged(nullptr);
}

void PublishGLBattleRenderer::updateInitiative()
{
    delete _initiativeBackground;
    _initiativeBackground = nullptr;

    QList<PublishGLImage*> nameTokens = _combatantNames.values();

    _initiativeTokenHeight = static_cast<qreal>(_scene.getTargetSize().height()) / 24.0;
    QSize initiativeArea;
    initiativeArea.setWidth((_initiativeTokenHeight * 1.2) + 5);

    if(_initiativeType == DMHelper::InitiativeType_ImageName)
    {
        for(PublishGLImage* nameToken : nameTokens)
        {
            if(nameToken)
            {
                if(initiativeArea.width() < (_initiativeTokenHeight * 1.2) + nameToken->getSize().width() + 5)
                    initiativeArea.setWidth((_initiativeTokenHeight * 1.2) + nameToken->getSize().width() + 5);

                if(_initiativeTokenHeight < nameToken->getSize().height())
                    _initiativeTokenHeight = nameToken->getSize().height();
            }
        }
    }

    initiativeArea.setHeight((_model->getCombatantCount() * _initiativeTokenHeight * 1.1) + 5);

    QImage initiativeAreaImage(initiativeArea, QImage::Format_RGBA8888);
    initiativeAreaImage.fill(QColor(0, 0, 0, 128));
    _initiativeBackground = new PublishGLImage(initiativeAreaImage, false);
    _initiativeBackground->setPosition(0, _scene.getTargetSize().height() - initiativeArea.height());
}

void PublishGLBattleRenderer::paintInitiative(QOpenGLFunctions* functions)
{
    if((!functions) || (!_model) || (_model->getCombatantCount() <= 0) || (_initiativeType == DMHelper::InitiativeType_None))
        return;

    // Initiative timeline test
    QMatrix4x4 screenCoords;
    screenCoords.ortho(0.f, _scene.getTargetSize().width(), 0.f, _scene.getTargetSize().height(), 0.1f, 1000.f);
    functions->glUniformMatrix4fv(_shaderProjectionMatrix, 1, GL_FALSE, screenCoords.constData());
    QMatrix4x4 tokenScreenCoords;
    qreal tokenSize = static_cast<qreal>(_scene.getTargetSize().height()) / 24.0;
    qreal tokenY = _scene.getTargetSize().height() - tokenSize / 2.0 - 5.0;

    if(_initiativeBackground)
    {
        functions->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, _initiativeBackground->getMatrixData());
        _initiativeBackground->paintGL();
    }

    int activeCombatant = _model->getActiveCombatantIndex();
    int currentCombatant = activeCombatant;
    do
    {
        BattleDialogModelCombatant* combatant = _model->getCombatant(currentCombatant);
        if((combatant) && (combatant->getHitPoints() > 0) && (combatant->getKnown()))
        {
            PublishGLObject* tokenObject = nullptr;
            QSizeF textureSize;
            if(combatant->getShown())
            {
                PublishGLBattleToken* combatantToken = _combatantTokens.value(combatant);
                tokenObject = combatantToken;
                if(combatantToken)
                    textureSize = combatantToken->getTextureSize();
            }
            else
            {
                tokenObject = _unknownToken;
                textureSize = _unknownToken->getImageSize();
            }

            if(tokenObject)
            {
                qreal scaleFactor = tokenSize / qMax(textureSize.width(), textureSize.height());
                tokenScreenCoords.setToIdentity();
                tokenScreenCoords.translate(tokenSize / 2.0, tokenY);
                tokenScreenCoords.scale(scaleFactor, scaleFactor);
                functions->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, tokenScreenCoords.constData());
                tokenObject->paintGL();
            }

            if((_initiativeType == DMHelper::InitiativeType_ImageName) && (combatant->getShown()))
            {
                PublishGLImage* combatantName = _combatantNames.value(combatant);
                if(combatantName)
                {
                    tokenScreenCoords.setToIdentity();
                    tokenScreenCoords.translate(tokenSize * 1.2, tokenY - (static_cast<qreal>(combatantName->getImageSize().height()) / 2.0));
                    functions->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, tokenScreenCoords.constData());
                    combatantName->paintGL();
                }
            }

            tokenY -= (_initiativeTokenHeight * 1.1);
        }

        if(++currentCombatant >= _model->getCombatantCount())
            currentCombatant = 0;

    } while(currentCombatant != activeCombatant);
}

void PublishGLBattleRenderer::recreateContents()
{
    _recreateContent = true;
    emit updateWidget();
}

void PublishGLBattleRenderer::activeCombatantMoved()
{
    if((!_activeCombatant) || (!_activeToken))
        return;

    QSize textureSize = _activeToken->getImageSize();
    qreal scaleFactor = (static_cast<qreal>(_scene.getGridScale()-2)) * _activeCombatant->getSizeFactor() / qMax(textureSize.width(), textureSize.height());

    _activeToken->setPositionScale(PublishGLBattleObject::sceneToWorld(_scene.getSceneRect(), _activeCombatant->getPosition()), scaleFactor);
    emit updateWidget();
}

void PublishGLBattleRenderer::tokenSelectionChanged(PublishGLBattleToken* token)
{
    if((!token) || (!token->getCombatant()) || (!_selectionToken))
        return;

    if(token->getCombatant()->getSelected())
        token->addEffect(*_selectionToken);
    else
        token->removeEffect(*_selectionToken);

    emit updateWidget();
}
