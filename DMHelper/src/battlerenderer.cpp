#include "battlerenderer.h"
#include "battledialoggraphicsscene.h"
#include "battledialogmodel.h"
#include "unselectedpixmap.h"
#include <QPainter>
#include <QGraphicsPixmapItem>
#include <QDebug>

const int BattleRendererItemChild_Index = 0;
const int BattleRendererItemChild_EffectId = 0;
const qreal SELECTED_PIXMAP_SIZE = 800.0;
const qreal ACTIVE_PIXMAP_SIZE = 800.0;

BattleRenderer::BattleRenderer(EncounterBattle& battle, QPixmap background, QObject *parent) :
    CampaignObjectRenderer(parent),
    _battle(battle),
    _background(nullptr),
    _scene(new BattleDialogGraphicsScene(this)),
    _combatantIcons(),
    _targetSize(),
    _rotation(0),
    _selectedCombatant(nullptr),
    _selectedItem(nullptr),
    _scaledSceneSize(),
    _targetRect(),
    _activePixmap(nullptr)
{
    if(!_battle.getBattleDialogModel())
        return;

    _scene->setModel(_battle.getBattleDialogModel());

    _background = new UnselectedPixmap();
    _background->setPixmap(background);
    _scene->addItem(_background);
    _background->setEnabled(false);
    _background->setZValue(DMHelper::BattleDialog_Z_Background);
    _scene->setSceneRect(_scene->itemsBoundingRect());

    _scene->createBattleContents(_background->boundingRect().toRect());

    /*
    _background = new UnselectedPixmap();
    _fow = new UnselectedPixmap();

    _background->setPixmap((QPixmap::fromImage(_model->getBackgroundImage())));
    _fowImage = QPixmap::fromImage(_model->getMap()->getFoWImage());
    _mapDrawer->setMap(_model->getMap(), &_fowImage);

    // TODO : createprescaledbackground

    _scene->addItem(_background);
    _fow->setParentItem(_background);
    _background->setEnabled(false);
    _background->setZValue(DMHelper::BattleDialog_Z_Background);

    // TODO: if(!_videoPlayer)
    _scene->setSceneRect(_scene->itemsBoundingRect());
    ui->graphicsView->fitInView(_model->getMapRect(), Qt::KeepAspectRatio);

    _scene->createBattleContents(_background->boundingRect().toRect());
    */

    //createActiveIcon();
    QPixmap activePmp;
    //if((_activeFile.isEmpty()) || (!activePmp.load(_activeFile)))
        activePmp.load(QString(":/img/data/active.png"));

    _activePixmap = _scene->addPixmap(activePmp);
    _activePixmap->setTransformationMode(Qt::SmoothTransformation);
    //_activePixmap->setScale(static_cast<qreal>(_battle.getBattleDialogModel()->getGridScale()) * _activeScale / ACTIVE_PIXMAP_SIZE);
    _activePixmap->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
    _activePixmap->setFlag(QGraphicsItem::ItemNegativeZStacksBehindParent);
    //_activePixmap->hide();

    // QRect itemRect = item->boundingRect().toRect();
    // int maxSize = qMax(itemRect.width(), itemRect.height());

    /*
    createCombatantFrame();
    createCountdownFrame();

    QPen movementPen(QColor(23,23,23,200), 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
    _movementPixmap = _scene->addEllipse(0, 0, 100, 100, movementPen, QBrush(QColor(255,255,255,25)));
    _movementPixmap->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
    _movementPixmap->setVisible(false);
    */

    // Add icons for existing combatants
    for(int i = 0; i < _battle.getBattleDialogModel()->getCombatantCount(); ++i)
    {
        BattleDialogModelCombatant* combatant = _battle.getBattleDialogModel()->getCombatant(i);
        if(combatant)
            createCombatantIcon(combatant);
    }

    updateModel();

    /*
    updateHighlights();
    */

    QRectF sceneRect = _scene->sceneRect();
    _scaledSceneSize = sceneRect.size().scaled(_targetSize, Qt::KeepAspectRatio);
    _targetRect = QRectF((_targetSize.width() - _scaledSceneSize.width()) / 2,
                         (_targetSize.height() - _scaledSceneSize.height()) / 2,
                         _scaledSceneSize.width(),
                         _scaledSceneSize.height());
}

BattleRenderer::~BattleRenderer()
{
}

void BattleRenderer::startRendering()
{
    refreshRender();
}

void BattleRenderer::refreshRender()
{
    if(!_battle.getBattleDialogModel())
        return;

    int i;
    for(i = 0; i < _battle.getBattleDialogModel()->getCombatantCount(); ++i)
    {
        BattleDialogModelCombatant* combatant = _battle.getBattleDialogModel()->getCombatant(i);
        if(combatant)
        {
            QGraphicsPixmapItem* pixmapItem = _combatantIcons.value(combatant);
            if(pixmapItem)
                pixmapItem->setPos(combatant->getPosition());
        }
    }

    QList<QGraphicsItem*> effectItems = _scene->getEffectItems();
    for(QGraphicsItem* item : effectItems)
    {
        if(item)
        {
            BattleDialogModelEffect* effect = BattleDialogModelEffect::getEffectFromItem(item);
            if(effect)
                effect->applyEffectValues(*item, _battle.getBattleDialogModel()->getGridScale());
        }
    }

    updateModel();

    QImage imagePublish;
    getImageForPublishing(imagePublish);
    emit publishImage(imagePublish);
}

void BattleRenderer::stopRendering()
{
}

void BattleRenderer::targetResized(const QSize& newSize)
{
    _targetSize = newSize;
    QRectF sceneRect = _scene->sceneRect();
    _scaledSceneSize = sceneRect.size().scaled(_targetSize, Qt::KeepAspectRatio);
    _targetRect = QRectF((_targetSize.width() - _scaledSceneSize.width()) / 2,
                         (_targetSize.height() - _scaledSceneSize.height()) / 2,
                         _scaledSceneSize.width(),
                         _scaledSceneSize.height());
    refreshRender();
}

void BattleRenderer::setRotation(int rotation)
{
    _rotation = rotation;
    refreshRender();
}

void BattleRenderer::publishWindowMouseDown(const QPointF& position)
{
    _selectedCombatant = nullptr;
    _selectedItem = nullptr;

    if((!_scene) || (_targetRect.width() == 0.0) || (_targetRect.height() == 0.0))
        return;

    if((position.x() < _targetRect.left()) || (position.x() > _targetRect.right()) ||
       (position.y() < _targetRect.top()) || (position.y() > _targetRect.bottom()))
        return;

    QPointF scenePos(_scene->sceneRect().width() * (position.x() - _targetRect.left()) / _targetRect.width(),
                     _scene->sceneRect().height() * (position.y() - _targetRect.top()) / _targetRect.height());

    QList<QGraphicsItem *> itemList = _scene->items(scenePos, Qt::IntersectsItemShape, Qt::DescendingOrder, QTransform());
    if(itemList.count() <= 0)
        return;

    // Search for the first selectable item
    for(QGraphicsItem* item : qAsConst(itemList))
    {
        if((item) && ((item->flags() & QGraphicsItem::ItemIsSelectable) == QGraphicsItem::ItemIsSelectable))
        {
            QGraphicsPixmapItem* pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(item);
            if((pixmapItem) && (_selectedCombatant = _combatantIcons.key(pixmapItem)))
            {
                _selectedItem = pixmapItem;
                _selectedCombatant->setPosition(scenePos);
                refreshRender();
                return;
            }

            BattleDialogModelEffect* effect = BattleDialogModelEffect::getEffectFromItem(item);
            if(effect)
            {
                _selectedItem = item;
                _selectedItem->setPos(scenePos);
                effect->setPosition(scenePos);
            }
        }
    }
}

void BattleRenderer::publishWindowMouseMove(const QPointF& position)
{
    if((!_scene) || (_targetRect.width() == 0.0) || (_targetRect.height() == 0.0))
        return;

    if((position.x() < _targetRect.left()) || (position.x() > _targetRect.right()) ||
       (position.y() < _targetRect.top()) || (position.y() > _targetRect.bottom()))
        return;

    QPointF scenePos(_scene->sceneRect().width() * (position.x() - _targetRect.left()) / _targetRect.width(),
                     _scene->sceneRect().height() * (position.y() - _targetRect.top()) / _targetRect.height());

    if(_selectedCombatant)
    {
        _selectedCombatant->setPosition(scenePos);
        refreshRender();
    }
    else if(_selectedItem)
    {
        BattleDialogModelEffect* effect = BattleDialogModelEffect::getEffectFromItem(_selectedItem);
        if(effect)
        {
            _selectedItem->setPos(scenePos);
            effect->setPosition(scenePos);
            refreshRender();
        }
    }
}

void BattleRenderer::publishWindowMouseRelease(const QPointF& position)
{
    Q_UNUSED(position);
    if(_selectedCombatant)
    {
        QString message = _selectedCombatant->getID().toString() + QString("#") +
                          QString::number(_selectedCombatant->getPosition().x()) + QString("#") +
                          QString::number(_selectedCombatant->getPosition().y());
        qDebug() << "[BattleRenderer] Sending message to server: " << message;
        emit sendServerMessage(message);
    }
    else if(_selectedItem)
    {
        BattleDialogModelEffect* effect = BattleDialogModelEffect::getEffectFromItem(_selectedItem);
        if(effect)
        {
            QString message = effect->getID().toString() + QString("#") +
                              QString::number(effect->getPosition().x()) + QString("#") +
                              QString::number(effect->getPosition().y());
            qDebug() << "[BattleRenderer] Sending message to server: " << message;
            emit sendServerMessage(message);
        }
    }
    _selectedCombatant = nullptr;
    _selectedItem = nullptr;
}

void BattleRenderer::updateModel()
{
    bool activePixmapShown = false;

    QList<BattleDialogModelCombatant*> combatants = _combatantIcons.keys();
    for(BattleDialogModelCombatant* combatant : combatants)
    {
        if(combatant)
        {
            UnselectedPixmap* combatantPixmap = dynamic_cast<UnselectedPixmap*>(_combatantIcons.value(combatant));
            if(combatantPixmap)
            {
                combatantPixmap->setSelected(combatant == _battle.getBattleDialogModel()->getSelectedCombatant());
                if((combatant == _battle.getBattleDialogModel()->getActiveCombatant()) && (combatantPixmap->scale() > 0.0))
                {
                    _activePixmap->show();
                    activePixmapShown = true;
                    _activePixmap->setScale(static_cast<qreal>(_battle.getBattleDialogModel()->getGridScale()) * combatant->getSizeFactor() / (ACTIVE_PIXMAP_SIZE * combatantPixmap->scale()));
                    QSizeF pixmapSize = combatantPixmap->boundingRect().size();
                    _activePixmap->setPos(-pixmapSize.width() / 2.0, -pixmapSize.height() / 2.0);
                    _activePixmap->setParentItem(combatantPixmap);
                }
            }
        }
    }

    if(!activePixmapShown)
        _activePixmap->hide();
}

void BattleRenderer::getImageForPublishing(QImage& imageForPublishing)
{
    /*
    QSize backgroundImageSize = (_videoPlayer && _videoPlayer->getImage()) ? _sourceRect.size() : _prescaledBackground.size();
    QSize unrotatedBackgroundImageSize = backgroundImageSize;
    if((!_videoPlayer || !_videoPlayer->getImage()) && ((_rotation == 90) || (_rotation == 270)))
        unrotatedBackgroundImageSize.transpose();

    //TODO: premultiplied format and draw directly to the return value image???
    QSize tempRotFrameSize;
    if(_videoPlayer && _videoPlayer->getImage())
    {
        tempRotFrameSize = sizeBackgroundToFrame(backgroundImageSize);
        if((_rotation == 90) || (_rotation == 270))
            tempRotFrameSize.transpose();
    }
    else
    {
        tempRotFrameSize = getRotatedTargetFrameSize(backgroundImageSize);
    }
    */

    //QImage drawingImageForPublishing = QImage(tempRotFrameSize, QImage::Format_ARGB32_Premultiplied);
    QImage drawingImageForPublishing = QImage(_targetSize, QImage::Format_ARGB32_Premultiplied);
    //drawingImageForPublishing.fill(_model->getBackgroundColor());
    drawingImageForPublishing.fill(Qt::black);

    QPainter painter;
    painter.begin(&drawingImageForPublishing);

    // For a static image, the pre-rendered background image is pre-rotated, so we should render it now before setting a rotation in the painter
    //if(!_videoPlayer)
    //    renderPrescaledBackground(painter, tempRotFrameSize);
//painter.drawPixmap(0, 0, _background);

    /*
    if(_rotation != 0)
    {
        int rotatePoint = qMax(tempRotFrameSize.width(),tempRotFrameSize.height()) / 2;

        painter.translate(rotatePoint, rotatePoint);
        painter.rotate(_rotation);
        painter.translate(-rotatePoint, -rotatePoint);

        if(_rotation == 90)
            painter.translate(0,tempRotFrameSize.height() - tempRotFrameSize.width());
        if(_rotation == 180)
            painter.translate(0,tempRotFrameSize.width() - tempRotFrameSize.height());
    }*/

    // For a video image, the image is not pre-rotated, so we should render it now after setting the rotation in the painter
    //if(_videoPlayer)
    //    renderVideoBackground(painter);

    // Draw the contents of the battle frame in publish mode
    /*
    QRect viewportRect = _publishRectValue.isValid() ? ui->graphicsView->mapFromScene(getCameraRect()).boundingRect() : ui->graphicsView->viewport()->rect();
    QRect sceneViewportRect = ui->graphicsView->mapFromScene(ui->graphicsView->sceneRect()).boundingRect();
    QRect sourceRect = viewportRect.intersected(sceneViewportRect);
    setPublishVisibility(true);
    ui->graphicsView->render(&painter,
                             QRectF(QPointF(0,0),unrotatedBackgroundImageSize),
                             sourceRect);
    setPublishVisibility(false);
    */

    _scene->render(&painter, _targetRect);

    /*
    // Draw the active combatant image on top
    if(_model->getActiveCombatant())
    {
        BattleDialogModelCombatant* nextCombatant = nullptr;

        if(_showOnDeck)
        {
            QPixmap pmp;
            if(_model->getActiveCombatant()->getShown())
                pmp = _model->getActiveCombatant()->getIconPixmap(DMHelper::PixmapSize_Animate);
            else
                pmp = ScaledPixmap::defaultPixmap()->getPixmap(DMHelper::PixmapSize_Animate);

            painter.drawImage(unrotatedBackgroundImageSize.width(),
                              0,
                              _combatantFrame);
            int dx = qMax(5, (_combatantFrame.width()-pmp.width())/2);
            int dy = qMax(5, (_combatantFrame.height()-pmp.height())/2);
            painter.drawPixmap(unrotatedBackgroundImageSize.width() + dx,
                               dy,
                               pmp);
            int conditions = _model->getActiveCombatant()->getConditions();
            if(conditions > 0)
            {
                int cx = 5;
                int cy = 5;
                for(int i = 0; i < Combatant::getConditionCount(); ++i)
                {
                    int condition = Combatant::getConditionByIndex(i);
                    if((conditions & condition) && (cy <= _combatantFrame.height() - 30))
                    {
                        QPixmap conditionPixmap(QString(":/img/data/img/") + Combatant::getConditionIcon(condition) + QString(".png"));
                        painter.drawPixmap(unrotatedBackgroundImageSize.width() + cx,
                                           cy,
                                           conditionPixmap.scaled(30,30));
                        cx += 30;
                        if(cx > _combatantFrame.width() - 30)
                        {
                            cx = 5;
                            cy += 30;
                        }
                    }
                }
            }

            nextCombatant = getNextCombatant(_model->getActiveCombatant());
        }

        if(_showCountdown)
        {
            int xPos = unrotatedBackgroundImageSize.width();
            if(_showOnDeck)
                xPos += _combatantFrame.width();
            if(_countdown > 0.0)
            {
                int countdownInt = static_cast<int>(_countdown);
                painter.setBrush(QBrush(_countdownColor));
                painter.drawRect(xPos + 5,
                                 _countdownFrame.height() - countdownInt - 5,
                                 10,
                                 countdownInt);
            }
            painter.drawImage(xPos,
                              0,
                              _countdownFrame);
        }

        if(_showOnDeck)
        {
            int yPos = DMHelper::PixmapSizes[DMHelper::PixmapSize_Animate][1] + 10;
            while((nextCombatant) &&
                  (nextCombatant != _model->getActiveCombatant()) &&
                  (yPos + _combatantFrame.height() < tempRotFrameSize.height()))
            {
                QPixmap nextPmp;
                if(nextCombatant->getShown())
                    nextPmp = nextCombatant->getIconPixmap(DMHelper::PixmapSize_Animate);
                else
                    nextPmp = ScaledPixmap::defaultPixmap()->getPixmap(DMHelper::PixmapSize_Animate);
                painter.drawImage(unrotatedBackgroundImageSize.width(),
                                  yPos,
                                  _combatantFrame);
                int dx = qMax(5, (_combatantFrame.width()-nextPmp.width())/2);
                int dy = qMax(5, (_combatantFrame.height()-nextPmp.height())/2);
                painter.drawPixmap(unrotatedBackgroundImageSize.width() + dx,
                                   yPos + dy,
                                   nextPmp);
                int conditions = nextCombatant->getConditions();
                if(conditions > 0)
                {
                    int cx = 5;
                    int cy = 5;
                    for(int i = 0; i < Combatant::getConditionCount(); ++i)
                    {
                        int condition = Combatant::getConditionByIndex(i);
                        if((conditions & condition) && (cy <= _combatantFrame.height() - 30))
                        {
                            QPixmap conditionPixmap(QString(":/img/data/img/") + Combatant::getConditionIcon(condition) + QString(".png"));
                            painter.drawPixmap(unrotatedBackgroundImageSize.width() + cx,
                                               yPos + cy,
                                               conditionPixmap.scaled(30,30));
                            cx += 30;
                            if(cx > _combatantFrame.width() - 30)
                            {
                                cx = 5;
                                cy += 30;
                            }
                        }
                    }
                }

                yPos += DMHelper::PixmapSizes[DMHelper::PixmapSize_Animate][1] + 10;
                nextCombatant = getNextCombatant(nextCombatant);
            }
        }
    }
    */

    painter.end();

    imageForPublishing = drawingImageForPublishing;
}

UnselectedPixmap* BattleRenderer::createCombatantIcon(BattleDialogModelCombatant* combatant)
{
    if((!_battle.getBattleDialogModel()) || (!combatant))
        return nullptr;

    QPixmap pix = combatant->getIconPixmap(DMHelper::PixmapSize_Battle);
    if(combatant->hasCondition(Combatant::Condition_Unconscious))
    {
        QImage originalImage = pix.toImage();
        QImage grayscaleImage = originalImage.convertToFormat(QImage::Format_Grayscale8);
        pix = QPixmap::fromImage(grayscaleImage);
    }
    Combatant::drawConditions(&pix, combatant->getConditions());

    UnselectedPixmap* pixmapItem = new UnselectedPixmap(pix, combatant);
    _scene->addItem(pixmapItem);
    pixmapItem->setFlag(QGraphicsItem::ItemIsMovable, true);
    pixmapItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
    pixmapItem->setZValue(DMHelper::BattleDialog_Z_Combatant);
    pixmapItem->setPos(combatant->getPosition());
    pixmapItem->setOffset(-static_cast<qreal>(pix.width())/2.0, -static_cast<qreal>(pix.height())/2.0);
    qreal sizeFactor = combatant->getSizeFactor();
    qreal scaleFactor = (static_cast<qreal>(_battle.getBattleDialogModel()->getGridScale()-2)) * sizeFactor / static_cast<qreal>(qMax(pix.width(),pix.height()));
    pixmapItem->setScale(scaleFactor);

    QString itemTooltip = QString("<b>") + combatant->getName() + QString("</b>");
    QStringList conditionString = Combatant::getConditionString(combatant->getConditions());
    if(conditionString.count() > 0)
        itemTooltip += QString("<p>") + conditionString.join(QString("<br/>"));
    pixmapItem->setToolTip(itemTooltip);

    qDebug() << "[Battle Renderer] combatant icon added " << combatant->getName() << ", scale " << scaleFactor;

    qreal gridSize = (static_cast<qreal>(_battle.getBattleDialogModel()->getGridScale())) / scaleFactor;
    qreal gridOffset = gridSize * static_cast<qreal>(sizeFactor) / 2.0;
    QGraphicsRectItem* rect = new QGraphicsRectItem(0, 0, gridSize * sizeFactor, gridSize * static_cast<qreal>(sizeFactor));
    rect->setPos(-gridOffset, -gridOffset);
    rect->setData(BattleRendererItemChild_Index, BattleRendererItemChild_Area);
    rect->setParentItem(pixmapItem);
    rect->setVisible(false);
    qDebug() << "[Battle Renderer] created " << pixmapItem << " with area child " << rect;

    //applyPersonalEffectToItem(pixmapItem);

    _combatantIcons.insert(combatant, pixmapItem);

    //connect(combatant, SIGNAL(combatantMoved(BattleDialogModelCombatant*)), this, SLOT(handleCombatantMoved(BattleDialogModelCombatant*)), static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection));
    //connect(combatant, SIGNAL(combatantMoved(BattleDialogModelCombatant*)), this, SLOT(updateHighlights()), static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection));
    //connect(combatant, SIGNAL(combatantSelected(BattleDialogModelCombatant*)), this, SLOT(handleCombatantSelected(BattleDialogModelCombatant*)));

    return pixmapItem;
}
