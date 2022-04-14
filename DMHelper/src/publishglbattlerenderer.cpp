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
#include <QGraphicsLineItem>
#include <QStyleOptionGraphicsItem>
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
    _shaderProgramRGB(0),
    _shaderModelMatrixRGB(0),
    _shaderProjectionMatrixRGB(0),
    _shaderProgramRGBA(0),
    _shaderModelMatrixRGBA(0),
    _shaderProjectionMatrixRGBA(0),
    _shaderAlphaRGBA(0),
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
    _recreateLine(false),
    _lineItem(nullptr),
    _lineText(nullptr),
    _lineImage(nullptr),
    _lineTextImage(nullptr),
    _updateFow(false),
    _recreateContent(false)
{
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

void PublishGLBattleRenderer::cleanup()
{
    _initialized = false;

    disconnect(_model, &BattleDialogModel::effectListChanged, this, &PublishGLBattleRenderer::recreateContents);
    disconnect(_model, &BattleDialogModel::activeCombatantChanged, this, &PublishGLBattleRenderer::updateWidget);
    disconnect(_model, &BattleDialogModel::activeCombatantChanged, this, &PublishGLBattleRenderer::activeCombatantChanged);
    disconnect(_model, &BattleDialogModel::combatantListChanged, this, &PublishGLBattleRenderer::recreateContents);
    disconnect(_model, &BattleDialogModel::showAliveChanged, this, &PublishGLBattleRenderer::updateWidget);
    disconnect(_model, &BattleDialogModel::showDeadChanged, this, &PublishGLBattleRenderer::updateWidget);
    disconnect(_model, &BattleDialogModel::showEffectsChanged, this, &PublishGLBattleRenderer::updateWidget);

    cleanupContents();

    _projectionMatrix.setToIdentity();

    destroyShaders();

    PublishGLRenderer::cleanup();
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
    if((_initialized) || (!_model) || (_model->getBackgroundImage().isNull()) || (!_targetWidget) || (!_targetWidget->context()))
        return;

    _scene.setGridScale(_model->getGridScale());

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = _targetWidget->context()->functions();
    if(!f)
        return;

    createShaders();

    //f->glEnable(GL_TEXTURE_2D); // Enable texturing
    //f->glEnable(GL_BLEND);// you enable blending function
    //f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //f->glEnable(GL_DEPTH_TEST);
    //f->glDepthFunc(GL_GREATER);

    // Create the objects
    initializeBackground();

    if(isBackgroundReady())
        createContents();

    QMatrix4x4 modelMatrix;
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(QVector3D(0.f, 0.f, 500.f), QVector3D(0.f, 0.f, 0.f), QVector3D(0.f, 1.f, 0.f));

    f->glUseProgram(_shaderProgramRGBA);
    f->glUniform1i(f->glGetUniformLocation(_shaderProgramRGBA, "texture1"), 0); // set it manually
    f->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, modelMatrix.constData());
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgramRGBA, "view"), 1, GL_FALSE, viewMatrix.constData());

    f->glUseProgram(_shaderProgramRGB);
    f->glUniform1i(f->glGetUniformLocation(_shaderProgramRGB, "texture1"), 0); // set it manually
    f->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, modelMatrix.constData());
    f->glUniformMatrix4fv(f->glGetUniformLocation(_shaderProgramRGB, "view"), 1, GL_FALSE, viewMatrix.constData());

    // Projection - note, this is set later when resizing the window
    updateProjectionMatrix();

    connect(_model, &BattleDialogModel::combatantListChanged, this, &PublishGLBattleRenderer::recreateContents);
    connect(_model, &BattleDialogModel::activeCombatantChanged, this, &PublishGLBattleRenderer::updateWidget);
    connect(_model, &BattleDialogModel::activeCombatantChanged, this, &PublishGLBattleRenderer::activeCombatantChanged);
    connect(_model, &BattleDialogModel::effectListChanged, this, &PublishGLBattleRenderer::recreateContents);
    connect(_model, &BattleDialogModel::showAliveChanged, this, &PublishGLBattleRenderer::updateWidget);
    connect(_model, &BattleDialogModel::showDeadChanged, this, &PublishGLBattleRenderer::updateWidget);
    connect(_model, &BattleDialogModel::showEffectsChanged, this, &PublishGLBattleRenderer::updateWidget);

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

        if(_recreateLine)
            createLineToken();
    }

    evaluatePointer();

    QOpenGLFunctions *f = _targetWidget->context()->functions();
    QOpenGLExtraFunctions *e = _targetWidget->context()->extraFunctions();
    if((!f) || (!e))
        return;

    f->glUseProgram(_shaderProgramRGB);
    f->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
    f->glUniformMatrix4fv(_shaderProjectionMatrixRGB, 1, GL_FALSE, _projectionMatrix.constData());

    if(!_scissorRect.isEmpty())
    {
        f->glEnable(GL_SCISSOR_TEST);
        f->glScissor(_scissorRect.x(), _scissorRect.y(), _scissorRect.width(), _scissorRect.height());
    }

    // Draw the scene:
    f->glClearColor(_model->getBackgroundColor().redF(), _model->getBackgroundColor().greenF(), _model->getBackgroundColor().blueF(), 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    paintBackground(f);

    if(_gridObject)
    {
        f->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, _gridObject->getMatrixData());
        _gridObject->paintGL();
    }

    if(_model->getShowEffects())
    {
        f->glUseProgram(_shaderProgramRGBA);
        f->glUniformMatrix4fv(_shaderProjectionMatrixRGBA, 1, GL_FALSE, _projectionMatrix.constData());
        f->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
        for(PublishGLBattleEffect* effectToken : _effectTokens)
        {
            if((effectToken) && (effectToken->getEffect()) && (effectToken->getEffect()->getEffectVisible()))
            {
                f->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, effectToken->getMatrixData());
                f->glUniform1f(_shaderAlphaRGBA, effectToken->getEffectAlpha());
                effectToken->paintGL();
            }
        }
        f->glUseProgram(_shaderProgramRGB);
    }

    QList<PublishGLBattleToken*> tokens = _combatantTokens.values();
    for(PublishGLBattleToken* enemyToken : tokens)
    {
        if((enemyToken) && (!enemyToken->isPC()) &&
           (enemyToken->getCombatant()) && (enemyToken->getCombatant()->getKnown()) && (enemyToken->getCombatant()->getShown()) &&
           ((_model->getShowDead()) || (enemyToken->getCombatant()->getHitPoints() > 0)) &&
           ((_model->getShowAlive()) || (enemyToken->getCombatant()->getHitPoints() <= 0)))
        {
            f->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, enemyToken->getMatrixData());
            enemyToken->paintGL();
            enemyToken->paintEffects(_shaderModelMatrixRGB);
        }
    }

    paintTokens(f, false);

    if(_fowObject)
    {
        f->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, _fowObject->getMatrixData());
        _fowObject->paintGL();
    }

    for(PublishGLBattleToken* pcToken : tokens)
    {
        if((pcToken) && (pcToken->isPC()))
        {
            f->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, pcToken->getMatrixData());
            pcToken->paintGL();
            pcToken->paintEffects(_shaderModelMatrixRGB);
        }
    }

    paintTokens(f, true);

    if(_lineImage)
    {
        f->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, _lineImage->getMatrixData());
        _lineImage->paintGL();
    }

    if(_lineTextImage)
    {
        f->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, _lineTextImage->getMatrixData());
        _lineTextImage->paintGL();
    }

    if(!_scissorRect.isEmpty())
        f->glDisable(GL_SCISSOR_TEST);

    paintInitiative(f);

    paintPointer(f, getBackgroundSize().toSize(), _shaderModelMatrixRGB);
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
        functions->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, _activeToken->getMatrixData());
        _activeToken->paintGL();
    }

    if((_movementPC == drawPCs) && (_movementVisible) && (_movementToken) && (_model->getShowMovement()))
    {
        functions->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, _movementToken->getMatrixData());
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
    createLineToken();

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
            PublishGLBattleEffect* effectToken = new PublishGLBattleEffect(&_scene, effect);
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
    delete _lineImage; _lineImage = nullptr;
    delete _lineTextImage; _lineTextImage = nullptr;

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
    functions->glUniformMatrix4fv(_shaderProjectionMatrixRGB, 1, GL_FALSE, screenCoords.constData());
    QMatrix4x4 tokenScreenCoords;
    qreal tokenSize = static_cast<qreal>(_scene.getTargetSize().height()) / 24.0;
    qreal tokenY = _scene.getTargetSize().height() - tokenSize / 2.0 - 5.0;

    if(_initiativeBackground)
    {
        functions->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, _initiativeBackground->getMatrixData());
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
                functions->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, tokenScreenCoords.constData());
                tokenObject->paintGL();
            }

            if((_initiativeType == DMHelper::InitiativeType_ImageName) && (combatant->getShown()))
            {
                PublishGLImage* combatantName = _combatantNames.value(combatant);
                if(combatantName)
                {
                    tokenScreenCoords.setToIdentity();
                    tokenScreenCoords.translate(tokenSize * 1.2, tokenY - (static_cast<qreal>(combatantName->getImageSize().height()) / 2.0));
                    functions->glUniformMatrix4fv(_shaderModelMatrixRGB, 1, GL_FALSE, tokenScreenCoords.constData());
                    combatantName->paintGL();
                }
            }

            tokenY -= (_initiativeTokenHeight * 1.1);
        }

        if(++currentCombatant >= _model->getCombatantCount())
            currentCombatant = 0;

    } while(currentCombatant != activeCombatant);

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

    const char *vertexShaderSourceRGB = "#version 330 core\n"
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
        qDebug() << "[BattleGLRenderer] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
        return;
    }

    const char *fragmentShaderSourceRGB = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec3 ourColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D texture1;\n"
        "void main()\n"
        "{\n"
        "    FragColor = texture(texture1, TexCoord); // FragColor = vec4(ourColor, 1.0f);\n"
        "}\0";

    unsigned int fragmentShaderRGB;
    fragmentShaderRGB = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShaderRGB, 1, &fragmentShaderSourceRGB, NULL);
    f->glCompileShader(fragmentShaderRGB);

    f->glGetShaderiv(fragmentShaderRGB, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShaderRGB, 512, NULL, infoLog);
        qDebug() << "[BattleGLRenderer] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    _shaderProgramRGB = f->glCreateProgram();

    f->glAttachShader(_shaderProgramRGB, vertexShaderRGB);
    f->glAttachShader(_shaderProgramRGB, fragmentShaderRGB);
    f->glLinkProgram(_shaderProgramRGB);

    f->glGetProgramiv(_shaderProgramRGB, GL_LINK_STATUS, &success);
    if(!success) {
        f->glGetProgramInfoLog(_shaderProgramRGB, 512, NULL, infoLog);
        qDebug() << "[BattleGLRenderer] ERROR::SHADER::PROGRAM::COMPILATION_FAILED: " << infoLog;
        return;
    }

    f->glUseProgram(_shaderProgramRGB);
    f->glDeleteShader(vertexShaderRGB);
    f->glDeleteShader(fragmentShaderRGB);
    _shaderModelMatrixRGB = f->glGetUniformLocation(_shaderProgramRGB, "model");
    _shaderProjectionMatrixRGB = f->glGetUniformLocation(_shaderProgramRGB, "projection");

    const char *vertexShaderSourceRGBA = "#version 330 core\n"
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
        qDebug() << "[BattleGLRenderer] ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << infoLog;
        return;
    }

    const char *fragmentShaderSourceRGBA = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec4 ourColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D texture1;\n"
        "void main()\n"
        "{\n"
        "    FragColor = texture(texture1, TexCoord) * ourColor; // FragColor = vec4(ourColor, 1.0f);\n"
        "}\0";

    unsigned int fragmentShaderRGBA;
    fragmentShaderRGBA = f->glCreateShader(GL_FRAGMENT_SHADER);
    f->glShaderSource(fragmentShaderRGBA, 1, &fragmentShaderSourceRGBA, NULL);
    f->glCompileShader(fragmentShaderRGBA);

    f->glGetShaderiv(fragmentShaderRGBA, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        f->glGetShaderInfoLog(fragmentShaderRGBA, 512, NULL, infoLog);
        qDebug() << "[BattleGLRenderer] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: " << infoLog;
        return;
    }

    _shaderProgramRGBA = f->glCreateProgram();

    f->glAttachShader(_shaderProgramRGBA, vertexShaderRGBA);
    f->glAttachShader(_shaderProgramRGBA, fragmentShaderRGBA);
    f->glLinkProgram(_shaderProgramRGBA);

    f->glGetProgramiv(_shaderProgramRGBA, GL_LINK_STATUS, &success);
    if(!success) {
        f->glGetProgramInfoLog(_shaderProgramRGBA, 512, NULL, infoLog);
        qDebug() << "[BattleGLRenderer] ERROR::SHADER::PROGRAM::COMPILATION_FAILED: " << infoLog;
        return;
    }

    f->glUseProgram(_shaderProgramRGBA);
    f->glDeleteShader(vertexShaderRGBA);
    f->glDeleteShader(fragmentShaderRGBA);
    _shaderModelMatrixRGBA = f->glGetUniformLocation(_shaderProgramRGBA, "model");
    _shaderProjectionMatrixRGBA = f->glGetUniformLocation(_shaderProgramRGBA, "projection");
    _shaderAlphaRGBA = f->glGetUniformLocation(_shaderProgramRGBA, "alpha");
}

void PublishGLBattleRenderer::destroyShaders()
{
    if((_targetWidget) && (_targetWidget->context()))
    {
        QOpenGLFunctions *f = _targetWidget->context()->functions();
        if(f)
        {
            if(_shaderProgramRGB > 0)
                f->glDeleteProgram(_shaderProgramRGB);
            if(_shaderProgramRGBA > 0)
                f->glDeleteProgram(_shaderProgramRGBA);
        }
    }

    _shaderProgramRGB = 0;
    _shaderModelMatrixRGB = 0;
    _shaderProjectionMatrixRGB = 0;
    _shaderProgramRGBA = 0;
    _shaderModelMatrixRGBA = 0;
    _shaderProjectionMatrixRGBA = 0;
    _shaderAlphaRGBA = 0;
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
