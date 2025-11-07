#include "publishglbattlerenderer.h"
#include "battledialogmodel.h"
#include "battledialogmodelcharacter.h"
#include "publishglbattletoken.h"
#include "publishglbattleeffect.h"
#include "publishglimage.h"
#include "battledialogmodelcombatant.h"
#include "layer.h"
#include "layertokens.h"
#include "characterv2.h"
#include "campaign.h"
#include "dmh_opengl.h"
#include <QOpenGLWidget>
#include <QMatrix4x4>
#include <QPainter>
#include <QApplication>
#include <QGraphicsLineItem>
#include <QStyleOptionGraphicsItem>
#include <QDebug>

// #define DEBUG_BATTLE_RENDERER

const int MOVEMENT_TOKEN_SIZE = 512;

PublishGLBattleRenderer::PublishGLBattleRenderer(BattleDialogModel* model, QObject *parent) :
    PublishGLRenderer(parent),
    _initialized(false),
    _model(model),
    _scene(),
    _projectionMatrix(),
    _cameraRect(),
    _scissorRect(),
    _shaderProgramRGB(0),
    _shaderModelMatrixRGB(0),
    _shaderProjectionMatrixRGB(0),
    _shaderProgramRGBA(0),
    _shaderModelMatrixRGBA(0),
    _shaderProjectionMatrixRGBA(0),
    _shaderAlphaRGBA(0),
    _shaderProgramRGBColor(0),
    _shaderModelMatrixRGBColor(0),
    _shaderProjectionMatrixRGBColor(0),
    _shaderRGBColor(0),
    _combatantTokens(),
    _combatantNames(),
    _unknownToken(nullptr),
    _initiativeBackground(nullptr),
    _effectTokens(),
    _initiativeType(DMHelper::InitiativeType_ImageName),
    _initiativeScale(1.0),
    _initiativeTokenHeight(0.0),
    _movementVisible(false),
    _movementType(DMHelper::MovementType_None),
    _movementRanges(),
    _movementCombatant(nullptr),
    _movementToken(nullptr),
    _tokenFrameFile(),
    _tokenFrame(nullptr),
    _countdownFrameFile(),
    _countdownFrame(nullptr),
    _countdownFill(nullptr),
    _showCountdown(false),
    _countdownScale(1.0),
    _countdownColor(Qt::white),
    _activeCombatant(nullptr),
    _activeTokenFile(),
    _activeToken(nullptr),
    _selectionTokenFile(),
    _selectionToken(nullptr),
    _recreateLine(false),
    _lineItem(nullptr),
    _lineText(nullptr),
    _lineImage(nullptr),
    _lineTextImage(nullptr),
    _updateSelectionTokens(false),
    _updateInitiative(false),
    _updateTokens(false),
    _recreateContent(false)
{
    if(_model)
    {
        connect(&_model->getLayerScene(), &LayerScene::layerAdded, this, &PublishGLBattleRenderer::layerAdded);
        connect(&_model->getLayerScene(), &LayerScene::layerRemoved, this, &PublishGLBattleRenderer::layerRemoved);
        connect(&_model->getLayerScene(), &LayerScene::layerRemoved, this, &PublishGLRenderer::updateWidget);
        connect(&_model->getLayerScene(), &LayerScene::layerVisibilityChanged, this, &PublishGLRenderer::updateWidget);
    }
}

PublishGLBattleRenderer::~PublishGLBattleRenderer()
{
}

CampaignObjectBase* PublishGLBattleRenderer::getObject()
{
    return _model;
}

QColor PublishGLBattleRenderer::getBackgroundColor()
{
    return _model ? _model->getBackgroundColor() : QColor();
}

void PublishGLBattleRenderer::rendererDeactivated()
{
    if(_model)
    {
        disconnect(&_model->getLayerScene(), &LayerScene::layerAdded, this, &PublishGLBattleRenderer::layerAdded);
        disconnect(&_model->getLayerScene(), &LayerScene::layerRemoved, this, &PublishGLBattleRenderer::layerRemoved);
        disconnect(&_model->getLayerScene(), &LayerScene::layerRemoved, this, &PublishGLRenderer::updateWidget);
    }

    PublishGLRenderer::rendererDeactivated();
}

bool PublishGLBattleRenderer::deleteOnDeactivation()
{
    return true;
}

QRect PublishGLBattleRenderer::getScissorRect()
{
    return _scissorRect;
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
    if((_initialized) || (!_model) || (!_targetWidget) || (!_targetWidget->context()))
        return;

    _scene.setGridScale(_model->getGridScale());

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

    qDebug() << "[PublishGLBattleRenderer] Initializing battle renderer";

    createShaders();
    _model->getLayerScene().playerSetShaders(_shaderProgramRGB, _shaderModelMatrixRGB, _shaderProjectionMatrixRGB, _shaderProgramRGBA, _shaderModelMatrixRGBA, _shaderProjectionMatrixRGBA, _shaderAlphaRGBA);

    // Store the movement type from the campaign ruleset
    Campaign* campaign = dynamic_cast<Campaign*>(_model->getParentByType(DMHelper::CampaignType_Campaign));
    _movementType = campaign ? campaign->getRuleset().getMovementType() : DMHelper::MovementType_None;
    _movementRanges = ((campaign) && (_movementType == DMHelper::MovementType_Range)) ? campaign->getRuleset().getMovementRanges() : QList<int>();

    // Create the objects
    _scene.deriveSceneRectFromSize(_model->getLayerScene().sceneSize());
    createContents();

    QList<Layer*> tokenLayers = _model->getLayerScene().getLayers(DMHelper::LayerType_Tokens);
    for(int i = 0; i < tokenLayers.count(); ++i)
    {
        LayerTokens* tokenLayer = dynamic_cast<LayerTokens*>(tokenLayers.at(i));
        if(tokenLayer)
        {
            connect(tokenLayer, &LayerTokens::postCombatantDrawGL, this, &PublishGLBattleRenderer::handleCombatantDrawnGL);
            tokenLayer->refreshEffects();
        }
    }

    QMatrix4x4 modelMatrix;
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, 500.f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGBColor);
    f->glUseProgram(_shaderProgramRGBColor);
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGBColor, f->glGetUniformLocation(_shaderProgramRGBColor, "texture1"), "texture1");
    DMH_DEBUG_OPENGL_glUniform1i(f->glGetUniformLocation(_shaderProgramRGBColor, "texture1"), 0); // set it manually
    f->glUniform1i(f->glGetUniformLocation(_shaderProgramRGBColor, "texture1"), 0); // set it manually
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrixRGBColor, 1, GL_FALSE, modelMatrix.constData(), modelMatrix);
    f->glUniformMatrix4fv(_shaderModelMatrixRGBColor, 1, GL_FALSE, modelMatrix.constData());
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGBColor, f->glGetUniformLocation(_shaderProgramRGBColor, "view"), "view");
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgramRGBColor, "view"), 1, GL_FALSE, viewMatrix.constData(), viewMatrix);
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgramRGBColor, "view"), 1, GL_FALSE, viewMatrix.constData());

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGBA);
    f->glUseProgram(_shaderProgramRGBA);
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGBA, f->glGetUniformLocation(_shaderProgramRGBA, "texture1"), "texture1");
    DMH_DEBUG_OPENGL_glUniform1i(f->glGetUniformLocation(_shaderProgramRGBA, "texture1"), 0); // set it manually
    f->glUniform1i(f->glGetUniformLocation(_shaderProgramRGBA, "texture1"), 0); // set it manually
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, modelMatrix.constData(), modelMatrix);
    f->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, modelMatrix.constData());
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGBA, f->glGetUniformLocation(_shaderProgramRGBA, "view"), "view");
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgramRGBA, "view"), 1, GL_FALSE, viewMatrix.constData(), viewMatrix);
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgramRGBA, "view"), 1, GL_FALSE, viewMatrix.constData());

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGB);
    f->glUseProgram(_shaderProgramRGB);
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGB, f->glGetUniformLocation(_shaderProgramRGB, "texture1"), "texture1");
    DMH_DEBUG_OPENGL_glUniform1i(f->glGetUniformLocation(_shaderProgramRGB, "texture1"), 0); // set it manually
    f->glUniform1i(f->glGetUniformLocation(_shaderProgramRGB, "texture1"), 0); // set it manually
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, modelMatrix.constData(), modelMatrix);
    f->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, modelMatrix.constData());
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGB, f->glGetUniformLocation(_shaderProgramRGB, "view"), "view");
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgramRGB, "view"), 1, GL_FALSE, viewMatrix.constData(), viewMatrix);
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgramRGB, "view"), 1, GL_FALSE, viewMatrix.constData());

    // Projection - note, this is set later when resizing the window
    updateProjectionMatrix();

    connect(_model, &BattleDialogModel::combatantListChanged, this, &PublishGLBattleRenderer::tokensChanged);
    connect(_model, &BattleDialogModel::initiativeOrderChanged, this, &PublishGLBattleRenderer::recreateContents);
    connect(_model, &BattleDialogModel::activeCombatantChanged, this, &PublishGLBattleRenderer::updateWidget);
    connect(_model, &BattleDialogModel::activeCombatantChanged, this, &PublishGLBattleRenderer::activeCombatantChanged);
    connect(_model, &BattleDialogModel::effectListChanged, this, &PublishGLBattleRenderer::recreateContents);
    connect(_model, &BattleDialogModel::showAliveChanged, this, &PublishGLBattleRenderer::updateWidget);
    connect(_model, &BattleDialogModel::showDeadChanged, this, &PublishGLBattleRenderer::updateWidget);
    connect(_model, &BattleDialogModel::showEffectsChanged, this, &PublishGLBattleRenderer::updateWidget);

    _initialized = true;
}

void PublishGLBattleRenderer::cleanupGL()
{
    qDebug() << "[PublishGLBattleRenderer] Cleaning up battle renderer";

    _initialized = false;

    disconnect(_model, &BattleDialogModel::effectListChanged, this, &PublishGLBattleRenderer::recreateContents);
    disconnect(_model, &BattleDialogModel::initiativeOrderChanged, this, &PublishGLBattleRenderer::recreateContents);
    disconnect(_model, &BattleDialogModel::activeCombatantChanged, this, &PublishGLBattleRenderer::updateWidget);
    disconnect(_model, &BattleDialogModel::activeCombatantChanged, this, &PublishGLBattleRenderer::activeCombatantChanged);
    disconnect(_model, &BattleDialogModel::combatantListChanged, this, &PublishGLBattleRenderer::tokensChanged);
    disconnect(_model, &BattleDialogModel::showAliveChanged, this, &PublishGLBattleRenderer::updateWidget);
    disconnect(_model, &BattleDialogModel::showDeadChanged, this, &PublishGLBattleRenderer::updateWidget);
    disconnect(_model, &BattleDialogModel::showEffectsChanged, this, &PublishGLBattleRenderer::updateWidget);

    cleanupContents();

    _movementType = DMHelper::MovementType_None;
    _movementRanges.clear();

    _projectionMatrix.setToIdentity();

    _model->getLayerScene().playerSetShaders(0, 0, 0, 0, 0, 0, 0);
    destroyShaders();

    PublishGLRenderer::cleanupGL();
}

void PublishGLBattleRenderer::resizeGL(int w, int h)
{
    QSize targetSize(w, h);

#ifdef DEBUG_BATTLE_RENDERER
    qDebug() << "[PublishGLBattleRenderer] Resize to: " << targetSize;
#endif

    _scene.setTargetSize(targetSize);
    if(_model)
        _model->getLayerScene().playerGLResize(w, h);

    _updateInitiative = true;

    updateProjectionMatrix();

    emit updateWidget();
}

void PublishGLBattleRenderer::paintGL()
{
    if((!_initialized) || (!_model) || (!_targetWidget) || (!_targetWidget->context()))
        return;

    if(_model->getLayerScene().playerGLUpdate())
        updateProjectionMatrix();

    DMH_DEBUG_OPENGL_PAINTGL();

    if(_recreateContent)
    {
        cleanupContents();
        createContents();
    }
    else
    {
        if(_updateSelectionTokens)
            updateSelectionTokens();

        if(_updateInitiative)
            updateInitiative();

        if(_recreateLine)
            createLineToken();

        if(_updateTokens)
            updateTokens();
    }

    evaluatePointer();

    QOpenGLFunctions *f = _targetWidget->context()->functions();
    QOpenGLExtraFunctions *e = _targetWidget->context()->extraFunctions();
    if((!f) || (!e))
        return;

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGB);
    f->glUseProgram(_shaderProgramRGB);
    f->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderProjectionMatrixRGB, 1, GL_FALSE, _projectionMatrix.constData(), _projectionMatrix);
    f->glUniformMatrix4fv(_shaderProjectionMatrixRGB, 1, GL_FALSE, _projectionMatrix.constData());

    if(!_scissorRect.isEmpty())
    {
        qreal pixelRatio = _targetWidget->devicePixelRatio();
        f->glEnable(GL_SCISSOR_TEST);
        f->glScissor(static_cast<GLint>(static_cast<qreal>(_scissorRect.x()) * pixelRatio),
                     static_cast<GLint>(static_cast<qreal>(_scissorRect.y()) * pixelRatio),
                     static_cast<GLsizei>(static_cast<qreal>(_scissorRect.width()) * pixelRatio),
                     static_cast<GLsizei>(static_cast<qreal>(_scissorRect.height()) * pixelRatio));
    }

    // Draw the scene:
    f->glClearColor(_model->getBackgroundColor().redF(), _model->getBackgroundColor().greenF(), _model->getBackgroundColor().blueF(), 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _model->getLayerScene().playerGLPaint(f, _shaderProgramRGB, _shaderModelMatrixRGB, _projectionMatrix.constData());

    if(_lineImage)
    {
        DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, _lineImage->getMatrixData(), _lineImage->getMatrix());
        f->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, _lineImage->getMatrixData());
        _lineImage->paintGL(f, nullptr);
    }

    if(_lineTextImage)
    {
        DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, _lineTextImage->getMatrixData(), _lineTextImage->getMatrix());
        f->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, _lineTextImage->getMatrixData());
        _lineTextImage->paintGL(f, nullptr);
    }

    if(!_scissorRect.isEmpty())
        f->glDisable(GL_SCISSOR_TEST);

    paintInitiative(f);

    if(_pointerImage)
        paintPointer(f, _model->getLayerScene().sceneSize().toSize(), _shaderModelMatrixRGB);
}

void PublishGLBattleRenderer::updateProjectionMatrix()
{
    if((!_model) || (_scene.getTargetSize().isEmpty()) || (_shaderProgramRGB == 0) || (!_targetWidget) || (!_targetWidget->context()))
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
    QSizeF backgroundMiddle = _model->getLayerScene().sceneSize() / 2.0;

#ifdef DEBUG_BATTLE_RENDERER
    qDebug() << "[PublishGLBattleRenderer] camera rect: " << _cameraRect << ", transformed camera: " << transformedCamera << ", target size: " << _scene.getTargetSize() << ", transformed target: " << transformedTarget;
    qDebug() << "[PublishGLBattleRenderer] rectSize: " << rectSize << ", camera top left: " << cameraTopLeft << ", camera middle: " << cameraMiddle << ", background middle: " << backgroundMiddle;
#endif

    _projectionMatrix.setToIdentity();
    _projectionMatrix.rotate(_rotation, 0.0, 0.0, -1.0);
    _projectionMatrix.ortho(cameraMiddle.x() - backgroundMiddle.width() - halfRect.width(), cameraMiddle.x() - backgroundMiddle.width() + halfRect.width(),
                            backgroundMiddle.height() - cameraMiddle.y() - halfRect.height(), backgroundMiddle.height() - cameraMiddle.y() + halfRect.height(),
                            0.1f, 1000.f);

    setPointerScale(rectSize.width() / transformedTarget.width());

    QSizeF scissorSize = transformedCamera.size().scaled(_scene.getTargetSize(), Qt::KeepAspectRatio);
#ifdef DEBUG_BATTLE_RENDERER
    qDebug() << "[PublishGLBattleRenderer] scissor size: " << scissorSize;
#endif
    _scissorRect.setX((_scene.getTargetSize().width() - scissorSize.width()) / 2.0);
    _scissorRect.setY((_scene.getTargetSize().height() - scissorSize.height()) / 2.0);
    _scissorRect.setWidth(scissorSize.width());
    _scissorRect.setHeight(scissorSize.height());
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

void PublishGLBattleRenderer::setInitiativeType(int initiativeType)
{
    if(_initiativeType == initiativeType)
        return;

    _initiativeType = initiativeType;
    _updateInitiative = true;
    emit updateWidget();
}

void PublishGLBattleRenderer::setInitiativeScale(qreal initiativeScale)
{
    if(_initiativeScale == initiativeScale)
        return;

    _initiativeScale = initiativeScale;
    _updateInitiative = true;
    emit updateWidget();
}

void PublishGLBattleRenderer::combatantTokenTypeChanged()
{
    tokensChanged();
}

void PublishGLBattleRenderer::distanceChanged(const QString& distance)
{
    Q_UNUSED(distance);

    _recreateLine = true;
    emit updateWidget();
}

void PublishGLBattleRenderer::distanceItemChanged(QGraphicsItem* shapeItem, QGraphicsSimpleTextItem* textItem)
{
    if((shapeItem == _lineItem) && (textItem == _lineText))
        return;

    _lineItem = shapeItem;
    _lineText = textItem;
    _recreateLine = true;
    emit updateWidget();
}

void PublishGLBattleRenderer::movementChanged(bool visible, BattleDialogModelCombatant* combatant, qreal remaining)
{
    if((_movementType == DMHelper::MovementType_None) || (!_movementToken))
        return;

    if(!combatant)
    {
        _movementVisible = false;
        _movementCombatant = nullptr;
    }
    else
    {
        _movementVisible = visible;
        if(combatant != _movementCombatant)
            _movementCombatant = combatant;

        _movementToken->setPositionScale(PublishGLBattleObject::sceneToWorld(_scene.getSceneRect(), combatant->getPosition()), remaining / MOVEMENT_TOKEN_SIZE);
    }

    emit updateWidget();
}

void PublishGLBattleRenderer::activeCombatantChanged(BattleDialogModelCombatant* activeCombatant)
{
    if(_activeCombatant == activeCombatant)
        return;

    disconnect(_activeCombatant, &BattleDialogModelObject::objectMoved, this, &PublishGLBattleRenderer::activeCombatantMoved);

    _activeCombatant = activeCombatant;
    if(_activeCombatant)
    {
        activeCombatantMoved();
        connect(_activeCombatant, &BattleDialogModelObject::objectMoved, this, &PublishGLBattleRenderer::activeCombatantMoved);
    }
}

void PublishGLBattleRenderer::setActiveToken(const QString& activeTokenFile)
{
    if(_activeTokenFile == activeTokenFile)
        return;

    _activeTokenFile = activeTokenFile;
    _updateSelectionTokens = true;
    emit updateWidget();
}

void PublishGLBattleRenderer::setSelectionToken(const QString& selectionTokenFile)
{
    if(_selectionTokenFile == selectionTokenFile)
        return;

    _selectionTokenFile = selectionTokenFile;
    _updateSelectionTokens = true;
    emit updateWidget();
}

void PublishGLBattleRenderer::setCombatantFrame(const QString& combatantFrame)
{
    if(_tokenFrameFile == combatantFrame)
        return;

    _tokenFrameFile = combatantFrame;
    _updateInitiative = true;
    emit updateWidget();
}

void PublishGLBattleRenderer::setCountdownFrame(const QString& countdownFrame)
{
    if(_countdownFrameFile == countdownFrame)
        return;

    _countdownFrameFile = countdownFrame;
    _updateInitiative = true;
    emit updateWidget();
}

void PublishGLBattleRenderer::setShowCountdown(bool showCountdown)
{
    if(_showCountdown == showCountdown)
        return;

    _showCountdown = showCountdown;
    emit updateWidget();
}

void PublishGLBattleRenderer::setCountdownValues(qreal countdown, const QColor& countdownColor)
{
    _countdownScale = countdown;
    _countdownColor = countdownColor;
    emit updateWidget();
}

void PublishGLBattleRenderer::updateBackground()
{
}

void PublishGLBattleRenderer::updateSelectionTokens()
{
#ifdef DEBUG_BATTLE_RENDERER
    qDebug() << "[PublishGLBattleRenderer] Updating Selection Tokens";
#endif

    QImage selectImage;
    if((_selectionTokenFile.isEmpty()) || (!selectImage.load(_selectionTokenFile)))
        selectImage.load(QString(":/img/data/selected.png"));
    PublishGLImage* newSelectionToken = new PublishGLImage(selectImage);
    QList<BattleDialogModelCombatant*> combatants = _combatantTokens.keys();
    foreach(BattleDialogModelCombatant* combatant, combatants)
    {
        if(combatant->getSelected())
        {
            PublishGLBattleToken* token = _combatantTokens.value(combatant);
            if(token)
            {
                token->removeHighlight(*_selectionToken);
                token->addHighlight(*newSelectionToken);
            }
        }
    }
    delete _selectionToken;
    _selectionToken = newSelectionToken;

    delete _activeToken; _activeToken = nullptr;
    QImage activeImage;
    if((_activeTokenFile.isEmpty()) || (!activeImage.load(_activeTokenFile)))
        activeImage.load(QString(":/img/data/active.png"));
    _activeToken = new PublishGLImage(activeImage);
    activeCombatantMoved();

    _updateSelectionTokens = false;
}

void PublishGLBattleRenderer::updateTokens()
{
    QList<Layer*> tokenLayers = _model->getLayerScene().getLayers(DMHelper::LayerType_Tokens);

    foreach(Layer* layer, tokenLayers)
    {
        if(LayerTokens* tokenLayer = dynamic_cast<LayerTokens*>(layer))
        {
            tokenLayer->playerGLUninitialize();
            tokenLayer->playerGLInitialize(this, &_scene);

            QList<BattleDialogModelCombatant*> combatants = tokenLayer->getCombatants();
            foreach(BattleDialogModelCombatant* combatant, combatants)
            {
                if(combatant)
                {
                    PublishGLBattleToken* token = tokenLayer->getCombatantToken(combatant);
                    if(token)
                    {
                        connect(token, &PublishGLBattleObject::changed, this, &PublishGLBattleRenderer::updateWidget);
                        connect(token, &PublishGLBattleToken::selectionChanged, this, &PublishGLBattleRenderer::tokenSelectionChanged);
                    }
                }
            }
        }
    }

    foreach(Layer* layer, tokenLayers)
    {
        if(LayerTokens* tokenLayer = dynamic_cast<LayerTokens*>(layer))
            tokenLayer->refreshEffects();
    }

    _updateTokens = false;
}

void PublishGLBattleRenderer::createContents()
{
    if(!_model)
        return;

#ifdef DEBUG_BATTLE_RENDERER
    qDebug() << "[PublishGLBattleRenderer] Creating all battle content";
#endif

    _model->getLayerScene().playerGLInitialize(this, &_scene);

    activeCombatantChanged(_model->getActiveCombatant());
    updateSelectionTokens();
    createLineToken();
    updateTokens();

    // Todo: move this into updateInitiative to avoid calling createContents when the init type is changed
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
        }
    }

    updateInitiative();

    _unknownToken = new PublishGLImage(ScaledPixmap::defaultPixmap()->getPixmap(DMHelper::PixmapSize_Animate).toImage());

    if(_movementType != DMHelper::MovementType_None)
    {
        QImage movementImage(QSize(MOVEMENT_TOKEN_SIZE, MOVEMENT_TOKEN_SIZE), QImage::Format_RGBA8888);
        movementImage.fill(Qt::transparent);
        QPainter movementPainter;
        movementPainter.begin(&movementImage);
            movementPainter.setPen(QPen(QColor(23, 23, 23, 200), 3, Qt::DashDotLine));
            movementPainter.setBrush(QBrush(QColor(255, 255, 255, 25)));
            movementPainter.drawEllipse(0, 0, 512, 512);
        movementPainter.end();
        _movementToken = new PublishGLImage(movementImage);
    }

    // Check if we need a pointer
    evaluatePointer();

    _recreateContent = false;
}

void PublishGLBattleRenderer::cleanupContents()
{
    delete _selectionToken; _selectionToken = nullptr;
    delete _activeToken; _activeToken = nullptr;
    delete _unknownToken; _unknownToken = nullptr;
    delete _tokenFrame; _tokenFrame = nullptr;
    delete _countdownFrame; _countdownFrame = nullptr;
    delete _countdownFill; _countdownFill = nullptr;
    delete _initiativeBackground; _initiativeBackground = nullptr;
    delete _movementToken; _movementToken = nullptr;
    delete _lineImage; _lineImage = nullptr;
    delete _lineTextImage; _lineTextImage = nullptr;

    qDeleteAll(_combatantTokens); _combatantTokens.clear();
    qDeleteAll(_combatantNames); _combatantNames.clear();
    qDeleteAll(_effectTokens); _effectTokens.clear();

    if(_model)
        _model->getLayerScene().playerGLUninitialize();

    _initiativeTokenHeight = 0.0;
    _movementVisible = false;
    _movementCombatant = nullptr;

    activeCombatantChanged(nullptr);
}

void PublishGLBattleRenderer::updateInitiative()
{
#ifdef DEBUG_BATTLE_RENDERER
    qDebug() << "[PublishGLBattleRenderer] Updating Initiative resources";
#endif

    delete _initiativeBackground;
    _initiativeBackground = nullptr;

    QList<PublishGLImage*> nameTokens = _combatantNames.values();

    _initiativeTokenHeight = static_cast<qreal>(_scene.getTargetSize().height()) * _initiativeScale / 24.0;
    QSize initiativeArea;
    initiativeArea.setWidth((_initiativeTokenHeight * 1.2) + 5);

    if((_initiativeType == DMHelper::InitiativeType_ImageName) || (_initiativeType == DMHelper::InitiativeType_ImagePCNames))
    {
        for(PublishGLImage* nameToken : nameTokens)
        {
            if(nameToken)
            {
                if(initiativeArea.width() < (_initiativeTokenHeight * 1.25) + nameToken->getSize().width() + 5)
                    initiativeArea.setWidth((_initiativeTokenHeight * 1.25) + nameToken->getSize().width() + 5);

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

    delete _tokenFrame; _tokenFrame = nullptr;
    QImage tokenFrameImage;
    if((_tokenFrameFile.isEmpty()) || (!tokenFrameImage.load(_tokenFrameFile)))
        tokenFrameImage.load(QString(":/img/data/combatant_frame.png"));
    _tokenFrame = new PublishGLImage(tokenFrameImage, false);
    _tokenFrame->setScale(_initiativeTokenHeight / static_cast<qreal>(tokenFrameImage.height()));

    delete _countdownFrame; _countdownFrame = nullptr;
    QImage countdownFrameImage;
    if((_countdownFrameFile.isEmpty()) || (!countdownFrameImage.load(_countdownFrameFile)))
        countdownFrameImage.load(QString(":/img/data/countdown_frame.png"));
    _countdownFrame = new PublishGLImage(countdownFrameImage, false);
    _countdownFrame->setX(_initiativeTokenHeight);
    _countdownFrame->setScale(_initiativeTokenHeight / static_cast<qreal>(countdownFrameImage.height()));

    QImage countdownFillImage(countdownFrameImage.size(), QImage::Format_RGBA8888);
    countdownFillImage.fill(Qt::white);
    _countdownFill = new PublishGLImage(countdownFillImage, false);
    _countdownFill->setX(_initiativeTokenHeight);
    _countdownFill->setScale(_initiativeTokenHeight / static_cast<qreal>(countdownFillImage.height()));

    _updateInitiative = false;
}

void PublishGLBattleRenderer::paintInitiative(QOpenGLFunctions* functions)
{
    if((!functions) || (!_model) || (_model->getCombatantCount() <= 0) || (_initiativeType == DMHelper::InitiativeType_None))
        return;

    // Initiative timeline test
    QMatrix4x4 screenCoords;
    screenCoords.ortho(0.f, _scene.getTargetSize().width(), 0.f, _scene.getTargetSize().height(), 0.1f, 1000.f);
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderProjectionMatrixRGB, 1, GL_FALSE, screenCoords.constData(), screenCoords);
    functions->glUniformMatrix4fv(_shaderProjectionMatrixRGB, 1, GL_FALSE, screenCoords.constData());
    QMatrix4x4 tokenScreenCoords;
    qreal tokenSize = static_cast<qreal>(_scene.getTargetSize().height()) * _initiativeScale / 24.0;
    qreal tokenY = _scene.getTargetSize().height() - tokenSize / 2.0 - 5.0;

    if(_initiativeBackground)
    {
        DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, _initiativeBackground->getMatrixData(), _initiativeBackground->getMatrix());
        functions->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, _initiativeBackground->getMatrixData());
        _initiativeBackground->paintGL(functions, nullptr);
    }

    int activeCombatant = _model->getCombatantIndex(_model->getActiveCombatant());
    int currentCombatant = activeCombatant;
    if(activeCombatant < 0)
        currentCombatant = 0;

    do
    {
        BattleDialogModelCombatant* combatant = _model->getCombatant(currentCombatant);
        bool layerVisible = ((combatant) && ((!combatant->getLayer()) || (combatant->getLayer()->getLayerVisiblePlayer())));
        if((combatant) && (layerVisible) && (combatant->getHitPoints() > 0) && (combatant->getKnown()))
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
                tokenScreenCoords.setToIdentity();
                tokenScreenCoords.translate(tokenSize / 2.0, tokenY);
                qreal scaleFactor = tokenSize / qMax(textureSize.width(), textureSize.height());
                tokenScreenCoords.scale(scaleFactor, scaleFactor);
                DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, tokenScreenCoords.constData(), tokenScreenCoords);
                functions->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, tokenScreenCoords.constData());
                tokenObject->paintGL(functions, nullptr);
                if(_tokenFrame)
                {
                    _tokenFrame->setY(tokenY - (_initiativeTokenHeight / 2.0));
                    DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, _tokenFrame->getMatrixData(), _tokenFrame->getMatrix());
                    functions->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, _tokenFrame->getMatrixData());
                   _tokenFrame->paintGL(functions, nullptr);
                   if((_countdownFrame) && (_countdownFill) && (_showCountdown) && (currentCombatant == activeCombatant))
                   {
                       DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGBColor);
                       functions->glUseProgram(_shaderProgramRGBColor);
                       DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderProjectionMatrixRGBColor, 1, GL_FALSE, screenCoords.constData(), screenCoords);
                       functions->glUniformMatrix4fv(_shaderProjectionMatrixRGBColor, 1, GL_FALSE, screenCoords.constData());
                       functions->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
                       DMH_DEBUG_OPENGL_glUniform4f(_shaderRGBColor, _countdownColor.redF(), _countdownColor.greenF(), _countdownColor.blueF(), 1.0);
                       functions->glUniform4f(_shaderRGBColor, _countdownColor.redF(), _countdownColor.greenF(), _countdownColor.blueF(), 1.0);
                       _countdownFill->setPositionScaleY(tokenY - (_initiativeTokenHeight / 2.0), _countdownScale);
                       DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrixRGBColor, 1, GL_FALSE, _countdownFill->getMatrixData(), _countdownFill->getMatrix());
                       functions->glUniformMatrix4fv(_shaderModelMatrixRGBColor, 1, GL_FALSE, _countdownFill->getMatrixData());
                       _countdownFill->paintGL(functions, nullptr);
                       DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGB);
                       functions->glUseProgram(_shaderProgramRGB);

                       DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, _countdownFrame->getMatrixData(), _countdownFrame->getMatrix());
                       functions->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, _countdownFrame->getMatrixData());
                       _countdownFrame->setY(tokenY - (_initiativeTokenHeight / 2.0));
                       _countdownFrame->paintGL(functions, nullptr);
                   }
                }
            }

            if((combatant->getShown()) && (((_initiativeType == DMHelper::InitiativeType_ImagePCNames) && (combatant->getCombatantType() == DMHelper::CombatantType_Character)) ||
                                           (_initiativeType == DMHelper::InitiativeType_ImageName)))
            {
                PublishGLImage* combatantName = _combatantNames.value(combatant);
                if(combatantName)
                {
                    tokenScreenCoords.setToIdentity();
                    tokenScreenCoords.translate(tokenSize * 1.25, tokenY - (static_cast<qreal>(combatantName->getImageSize().height()) / 2.0));
                    DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, tokenScreenCoords.constData(), tokenScreenCoords);
                    functions->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, tokenScreenCoords.constData());
                    combatantName->paintGL(functions, nullptr);
                }
            }

            tokenY -= (_initiativeTokenHeight * 1.1);
        }

        if(++currentCombatant >= _model->getCombatantCount())
            currentCombatant = activeCombatant <= 0 ? activeCombatant : 0;

    } while(currentCombatant != activeCombatant);

    DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderProjectionMatrixRGB, 1, GL_FALSE, _projectionMatrix.constData(), _projectionMatrix);
    functions->glUniformMatrix4fv(_shaderProjectionMatrixRGB, 1, GL_FALSE, _projectionMatrix.constData());
}

void PublishGLBattleRenderer::createShaders()
{
    int  success;
    char infoLog[512];

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

    const char *vertexShaderSourceRGB = "#version 410 core\n"
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

    unsigned int vertexShaderRGB;
    vertexShaderRGB = f->glCreateShader(GL_VERTEX_SHADER);
    f->glShaderSource(vertexShaderRGB, 1, &vertexShaderSourceRGB, NULL);
    f->glCompileShader(vertexShaderRGB);

    f->glGetShaderiv(vertexShaderRGB, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(vertexShaderRGB, 512, NULL, infoLog);
        qDebug() << "[PublishGLBattleRenderer] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
        return;
    }

    const char *fragmentShaderSourceRGB = "#version 410 core\n"
        "out vec4 FragColor;\n"
        "in vec3 ourColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D texture1;\n"
        "void main()\n"
        "{\n"
        "    FragColor = texture(texture1, TexCoord);\n"
        "}\0";

    unsigned int fragmentShaderRGB;
    fragmentShaderRGB = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShaderRGB, 1, &fragmentShaderSourceRGB, NULL);
    f->glCompileShader(fragmentShaderRGB);

    f->glGetShaderiv(fragmentShaderRGB, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShaderRGB, 512, NULL, infoLog);
        qDebug() << "[PublishGLBattleRenderer] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    _shaderProgramRGB = f->glCreateProgram();
    DMH_DEBUG_OPENGL_glCreateProgram(_shaderProgramRGB, "_shaderProgramRGB");

    f->glAttachShader(_shaderProgramRGB, vertexShaderRGB);
    f->glAttachShader(_shaderProgramRGB, fragmentShaderRGB);
    f->glLinkProgram(_shaderProgramRGB);

    f->glGetProgramiv(_shaderProgramRGB, GL_LINK_STATUS, &success);
    if(!success)
    {
        f->glGetProgramInfoLog(_shaderProgramRGB, 512, NULL, infoLog);
        qDebug() << "[PublishGLBattleRenderer] ERROR::SHADER::PROGRAM::COMPILATION_FAILED: " << infoLog;
        return;
    }

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGB);
    f->glUseProgram(_shaderProgramRGB);
    f->glDeleteShader(vertexShaderRGB);
    f->glDeleteShader(fragmentShaderRGB);
    _shaderModelMatrixRGB = f->glGetUniformLocation(_shaderProgramRGB, "model");
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGB, _shaderModelMatrixRGB, "model");
    _shaderProjectionMatrixRGB = f->glGetUniformLocation(_shaderProgramRGB, "projection");
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGB, _shaderProjectionMatrixRGB, "projection");

    const char *vertexShaderSourceRGBA = "#version 410 core\n"
        "layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0\n"
        "layout (location = 1) in vec3 aColor; // the color variable has attribute position 1\n"
        "layout (location = 2) in vec2 aTexCoord;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "uniform float alpha;\n"
        "out vec4 ourColor; // output a color to the fragment shader\n"
        "out vec2 TexCoord;\n"
        "void main()\n"
        "{\n"
        "   // note that we read the multiplication from right to left\n"
        "   gl_Position = projection * view * model * vec4(aPos, 1.0); // gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "   ourColor = vec4(aColor, alpha); // set ourColor to the input color we got from the vertex data\n"
        "   TexCoord = aTexCoord;\n"
        "}\0";

    unsigned int vertexShaderRGBA;
    vertexShaderRGBA = f->glCreateShader(GL_VERTEX_SHADER);
    f->glShaderSource(vertexShaderRGBA, 1, &vertexShaderSourceRGBA, NULL);
    f->glCompileShader(vertexShaderRGBA);

    f->glGetShaderiv(vertexShaderRGBA, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(vertexShaderRGBA, 512, NULL, infoLog);
        qDebug() << "[PublishGLBattleRenderer] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
        return;
    }

    const char *fragmentShaderSourceRGBA = "#version 410 core\n"
        "out vec4 FragColor;\n"
        "in vec4 ourColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D texture1;\n"
        "void main()\n"
        "{\n"
        "    FragColor = texture(texture1, TexCoord) * ourColor;\n"
        "}\0";

    unsigned int fragmentShaderRGBA;
    fragmentShaderRGBA = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShaderRGBA, 1, &fragmentShaderSourceRGBA, NULL);
    f->glCompileShader(fragmentShaderRGBA);

    f->glGetShaderiv(fragmentShaderRGBA, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShaderRGBA, 512, NULL, infoLog);
        qDebug() << "[PublishGLBattleRenderer] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    _shaderProgramRGBA = f->glCreateProgram();
    DMH_DEBUG_OPENGL_glCreateProgram(_shaderProgramRGBA, "_shaderProgramRGBA");

    f->glAttachShader(_shaderProgramRGBA, vertexShaderRGBA);
    f->glAttachShader(_shaderProgramRGBA, fragmentShaderRGBA);
    f->glLinkProgram(_shaderProgramRGBA);

    f->glGetProgramiv(_shaderProgramRGBA, GL_LINK_STATUS, &success);
    if(!success)
    {
        f->glGetProgramInfoLog(_shaderProgramRGBA, 512, NULL, infoLog);
        qDebug() << "[PublishGLBattleRenderer] ERROR::SHADER::PROGRAM::COMPILATION_FAILED: " << infoLog;
        return;
    }

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGBA);
    f->glUseProgram(_shaderProgramRGBA);
    f->glDeleteShader(vertexShaderRGBA);
    f->glDeleteShader(fragmentShaderRGBA);
    _shaderModelMatrixRGBA = f->glGetUniformLocation(_shaderProgramRGBA, "model");
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGBA, _shaderModelMatrixRGBA, "model");
    _shaderProjectionMatrixRGBA = f->glGetUniformLocation(_shaderProgramRGBA, "projection");
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGBA, _shaderProjectionMatrixRGBA, "projection");
    _shaderAlphaRGBA = f->glGetUniformLocation(_shaderProgramRGBA, "alpha");
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGBA, _shaderAlphaRGBA, "alpha");

    const char *vertexShaderSourceRGBColor = "#version 410 core\n"
        "layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0\n"
        "layout (location = 1) in vec3 aColor; // the color variable has attribute position 1\n"
        "layout (location = 2) in vec2 aTexCoord;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "uniform vec4 inColor;\n"
        "out vec4 ourColor; // output a color to the fragment shader\n"
        "out vec2 TexCoord;\n"
        "void main()\n"
        "{\n"
        "   // note that we read the multiplication from right to left\n"
        "   gl_Position = projection * view * model * vec4(aPos, 1.0); // gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "   ourColor = inColor; // set ourColor to the input color we got from the vertex data\n"
        "   TexCoord = aTexCoord;\n"
        "}\0";

    unsigned int vertexShaderRGBColor;
    vertexShaderRGBColor = f->glCreateShader(GL_VERTEX_SHADER);
    f->glShaderSource(vertexShaderRGBColor, 1, &vertexShaderSourceRGBColor, NULL);
    f->glCompileShader(vertexShaderRGBColor);

    f->glGetShaderiv(vertexShaderRGBColor, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(vertexShaderRGBColor, 512, NULL, infoLog);
        qDebug() << "[PublishGLBattleRenderer] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
        return;
    }

    const char *fragmentShaderSourceRGBColor = "#version 410 core\n"
        "out vec4 FragColor;\n"
        "in vec4 ourColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D texture1;\n"
        "void main()\n"
        "{\n"
        "    FragColor = ourColor;\n"
        "}\0";


    unsigned int fragmentShaderRGBColor;
    fragmentShaderRGBColor = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShaderRGBColor, 1, &fragmentShaderSourceRGBColor, NULL);
    f->glCompileShader(fragmentShaderRGBColor);

    f->glGetShaderiv(fragmentShaderRGBColor, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShaderRGBColor, 512, NULL, infoLog);
        qDebug() << "[PublishGLBattleRenderer] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    _shaderProgramRGBColor = f->glCreateProgram();
    DMH_DEBUG_OPENGL_glCreateProgram(_shaderProgramRGBColor, "_shaderProgramRGBColor");

    f->glAttachShader(_shaderProgramRGBColor, vertexShaderRGBColor);
    f->glAttachShader(_shaderProgramRGBColor, fragmentShaderRGBColor);
    f->glLinkProgram(_shaderProgramRGBColor);

    f->glGetProgramiv(_shaderProgramRGBColor, GL_LINK_STATUS, &success);
    if(!success)
    {
        f->glGetProgramInfoLog(_shaderProgramRGBColor, 512, NULL, infoLog);
        qDebug() << "[PublishGLBattleRenderer] ERROR::SHADER::PROGRAM::COMPILATION_FAILED: " << infoLog;
        return;
    }

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGBColor);
    f->glUseProgram(_shaderProgramRGBColor);
    f->glDeleteShader(vertexShaderRGBColor);
    f->glDeleteShader(fragmentShaderRGBColor);
    _shaderModelMatrixRGBColor = f->glGetUniformLocation(_shaderProgramRGBColor, "model");
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGBColor, _shaderModelMatrixRGBColor, "model");
    _shaderProjectionMatrixRGBColor = f->glGetUniformLocation(_shaderProgramRGBColor, "projection");
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGBColor, _shaderProjectionMatrixRGBColor, "projection");
    _shaderRGBColor = f->glGetUniformLocation(_shaderProgramRGBColor, "inColor");
    DMH_DEBUG_OPENGL_Singleton::registerUniform(_shaderProgramRGBColor, _shaderRGBColor, "inColor");

#ifdef DEBUG_BATTLE_RENDERER
    qDebug() << "[PublishGLBattleRenderer] _shaderProgramRGB: " << _shaderProgramRGB << ", _shaderModelMatrixRGB: " << _shaderModelMatrixRGB << ", _shaderProjectionMatrixRGB: " << _shaderProjectionMatrixRGB << ", _shaderProgramRGBA: " << _shaderProgramRGBA << ", _shaderModelMatrixRGBA: " << _shaderModelMatrixRGBA << ", _shaderProjectionMatrixRGBA: " << _shaderProjectionMatrixRGBA << ", _shaderAlphaRGBA: " << _shaderAlphaRGBA << ", _shaderProgramRGBColor: " << _shaderProgramRGBColor << ", _shaderModelMatrixRGBColor: " << _shaderModelMatrixRGBColor << ", _shaderProjectionMatrixRGBColor: " << _shaderProjectionMatrixRGBColor << ", _shaderRGBColor: " << _shaderRGBColor;
#endif
}

void PublishGLBattleRenderer::destroyShaders()
{
    if((_targetWidget) && (_targetWidget->context()))
    {
        QOpenGLFunctions *f = _targetWidget->context()->functions();
        if(f)
        {
            if(_shaderProgramRGB > 0)
            {
                DMH_DEBUG_OPENGL_Singleton::removeProgram(_shaderProgramRGB);
                f->glDeleteProgram(_shaderProgramRGB);
            }
            if(_shaderProgramRGBA > 0)
            {
                DMH_DEBUG_OPENGL_Singleton::removeProgram(_shaderProgramRGBA);
                f->glDeleteProgram(_shaderProgramRGBA);
            }
            if(_shaderProgramRGBColor > 0)
            {
                DMH_DEBUG_OPENGL_Singleton::removeProgram(_shaderProgramRGBColor);
                f->glDeleteProgram(_shaderProgramRGBColor);
            }
        }
    }

    _shaderProgramRGB = 0;
    _shaderModelMatrixRGB = 0;
    _shaderProjectionMatrixRGB = 0;
    _shaderProgramRGBA = 0;
    _shaderModelMatrixRGBA = 0;
    _shaderProjectionMatrixRGBA = 0;
    _shaderAlphaRGBA = 0;
    _shaderProgramRGBColor = 0;
    _shaderModelMatrixRGBColor = 0;
    _shaderProjectionMatrixRGBColor = 0;
    _shaderRGBColor = 0;
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
    qreal combatantScale = static_cast<qreal>(_activeCombatant->getLayer() ? _activeCombatant->getLayer()->getScale() : DMHelper::STARTING_GRID_SCALE);
    qreal scaleFactor = (combatantScale - 2.0) * _activeCombatant->getSizeFactor() / qMax(textureSize.width(), textureSize.height());

    _activeToken->setPositionScale(PublishGLBattleObject::sceneToWorld(_scene.getSceneRect(), _activeCombatant->getPosition()), scaleFactor);
    emit updateWidget();
}

void PublishGLBattleRenderer::tokenSelectionChanged(PublishGLBattleToken* token)
{
    if((!token) || (!token->getCombatant()) || (!_selectionToken))
        return;

    if(token->getCombatant()->getSelected())
        token->addHighlight(*_selectionToken);
    else
        token->removeHighlight(*_selectionToken);

    emit updateWidget();
}

void PublishGLBattleRenderer::tokensChanged()
{
    _updateTokens = true;
    emit updateWidget();
}

void PublishGLBattleRenderer::createLineToken()
{
    delete _lineImage; _lineImage = nullptr;
    delete _lineTextImage; _lineTextImage = nullptr;
    _recreateLine = false;

    if(_lineItem)
    {
        QRectF lineRect = _lineItem->boundingRect();
        QImage lineImage(lineRect.size().toSize(), QImage::Format_RGBA8888);
        lineImage.fill(Qt::transparent);
        QPainter linePainter;
        linePainter.begin(&lineImage);
            linePainter.translate(-lineRect.topLeft());
            QStyleOptionGraphicsItem options;
            _lineItem->paint(&linePainter, &options);
        linePainter.end();

        if(_lineImage)
            _lineImage->setImage(lineImage);
        else
            _lineImage = new PublishGLImage(lineImage, false);

        QPointF linePos = PublishGLBattleObject::sceneToWorld(_scene.getSceneRect(), lineRect.topLeft());
        _lineImage->setPosition(linePos.x(), linePos.y() - lineRect.height());
    }

    if(_lineText)
    {
        QRectF textRect = _lineText->boundingRect();
        QImage textImage(textRect.size().toSize(), QImage::Format_RGBA8888);
        textImage.fill(Qt::transparent);
        QPainter textPainter;
        textPainter.begin(&textImage);
            textPainter.translate(-textRect.topLeft());
            QStyleOptionGraphicsItem options;
            _lineText->paint(&textPainter, &options, nullptr);
        textPainter.end();

        if(_lineTextImage)
            _lineTextImage->setImage(textImage);
        else
            _lineTextImage = new PublishGLImage(textImage, false);

        QPointF textPos = PublishGLBattleObject::sceneToWorld(_scene.getSceneRect(), _lineText->pos());
        _lineTextImage->setPosition(textPos.x(), textPos.y() - textRect.height());
    }

    emit updateWidget();
}

void PublishGLBattleRenderer::layerAdded(Layer* layer)
{
    if(!layer)
        return;

    if(layer->getFinalType() == DMHelper::LayerType_Tokens)
    {
        LayerTokens* tokenLayer = dynamic_cast<LayerTokens*>(layer);
        if(tokenLayer)
            connect(tokenLayer, &LayerTokens::postCombatantDrawGL, this, &PublishGLBattleRenderer::handleCombatantDrawnGL);
    }

    layer->playerSetShaders(_shaderProgramRGB, _shaderModelMatrixRGB, _shaderProjectionMatrixRGB, _shaderProgramRGBA, _shaderModelMatrixRGBA, _shaderProjectionMatrixRGBA, _shaderAlphaRGBA);
    emit updateWidget();
}

void PublishGLBattleRenderer::layerRemoved(Layer* layer)
{
    if(!layer)
        return;

    if(layer->getFinalType() == DMHelper::LayerType_Tokens)
    {
        LayerTokens* tokenLayer = dynamic_cast<LayerTokens*>(layer);
        if(tokenLayer)
            disconnect(tokenLayer, &LayerTokens::postCombatantDrawGL, this, &PublishGLBattleRenderer::handleCombatantDrawnGL);
    }
}

void PublishGLBattleRenderer::handleCombatantDrawnGL(QOpenGLFunctions* functions, BattleDialogModelCombatant* combatant, PublishGLBattleToken* combatantToken)
{
    if((!functions) || (!combatant))
        return;

    if((combatant == _movementCombatant) && (_movementVisible) && (_movementCombatant) && (_movementToken) && (_model->getShowMovement()) &&
       ((combatantToken->isPC()) || ((_movementCombatant->getKnown()) &&
                                     (_movementCombatant->getShown()) &&
                                     ((_model->getShowDead()) || (_movementCombatant->getHitPoints() > 0)) &&
                                     ((_model->getShowAlive()) || (_movementCombatant->getHitPoints() <= 0)))))
    {
        if(_movementType == DMHelper::MovementType_Distance)
        {
            DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, _movementToken->getMatrixData(), _movementToken->getMatrix());
            functions->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, _movementToken->getMatrixData());
            _movementToken->paintGL(functions, nullptr);
        }
        else if(_movementType == DMHelper::MovementType_Range)
        {
            if(_movementRanges.count() > 0)
            {
                for(int i = 0; i < _movementRanges.count(); ++i)
                {
                    int rangeSquares = 2 * (_movementRanges.at(i) / 5) + 1;
                    qreal rangeRadius = combatant->getLayer()->getScale() * rangeSquares;
                    _movementToken->setScale(rangeRadius / MOVEMENT_TOKEN_SIZE);
                    DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, _movementToken->getMatrixData(), _movementToken->getMatrix());
                    functions->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, _movementToken->getMatrixData());
                    _movementToken->paintGL(functions, nullptr);
                }
            }
        }
    }

    if(combatant == _activeCombatant)
    {
        if((_activeCombatant) && (_activeToken) &&
           ((combatantToken->isPC()) || ((_activeCombatant->getKnown()) &&
                                         (_activeCombatant->getShown()) &&
                                         ((_model->getShowDead()) || (_activeCombatant->getHitPoints() > 0)) &&
                                         ((_model->getShowAlive()) || (_activeCombatant->getHitPoints() <= 0)))))
        {
            DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, _activeToken->getMatrixData(), _activeToken->getMatrix());
            functions->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, _activeToken->getMatrixData());
            _activeToken->paintGL(functions, nullptr);
        }
    }
}
