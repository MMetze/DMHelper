#include "battlerenderer.h"
#include "battledialoggraphicsscene.h"
#include "battledialogmodel.h"
#include "unselectedpixmap.h"
#include <QPainter>
#include <QGraphicsPixmapItem>
#include <QDebug>

const int BattleRendererItemChild_Index = 0;
const int BattleRendererItemChild_EffectId = 0;

BattleRenderer::BattleRenderer(EncounterBattle& battle, QPixmap background, QObject *parent) :
    CampaignObjectRenderer(parent),
    _battle(battle),
    _background(nullptr),
    _scene(new BattleDialogGraphicsScene(this)),
    _targetSize(),
    _rotation(0)
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

    createActiveIcon();
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
        createCombatantIcon(_battle.getBattleDialogModel()->getCombatant(i));
    }

    /*
    updateHighlights();
    */
}

BattleRenderer::~BattleRenderer()
{
}

void BattleRenderer::startRendering()
{
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
}

void BattleRenderer::setRotation(int rotation)
{
    _rotation = rotation;
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
    _scene->render(&painter);

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

void BattleRenderer::createCombatantIcon(BattleDialogModelCombatant* combatant)
{
    if((!_battle.getBattleDialogModel()) || (!combatant))
        return;

    QPixmap pix = combatant->getIconPixmap(DMHelper::PixmapSize_Battle);
    if(combatant->hasCondition(Combatant::Condition_Unconscious))
    {
        QImage originalImage = pix.toImage();
        QImage grayscaleImage = originalImage.convertToFormat(QImage::Format_Grayscale8);
        pix = QPixmap::fromImage(grayscaleImage);
    }
    Combatant::drawConditions(&pix, combatant->getConditions());

    QGraphicsPixmapItem* pixmapItem = new UnselectedPixmap(pix, combatant);
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

    //_combatantIcons.insert(combatant, pixmapItem);

    //connect(combatant, SIGNAL(combatantMoved(BattleDialogModelCombatant*)), this, SLOT(handleCombatantMoved(BattleDialogModelCombatant*)), static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection));
    //connect(combatant, SIGNAL(combatantMoved(BattleDialogModelCombatant*)), this, SLOT(updateHighlights()), static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection));
    //connect(combatant, SIGNAL(combatantSelected(BattleDialogModelCombatant*)), this, SLOT(handleCombatantSelected(BattleDialogModelCombatant*)));
}
