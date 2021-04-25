#include "battlerenderer.h"
#include "battledialoggraphicsscene.h"
#include <QPainter>
#include <QDebug>

BattleRenderer::BattleRenderer(EncounterBattle& battle, QPixmap background, QObject *parent) :
    CampaignObjectRenderer(parent),
    _battle(battle),
    _background(background),
    _scene(new BattleDialogGraphicsScene(this)),
    _targetSize(),
    _rotation(0)
{
    _scene->setModel(_battle.getBattleDialogModel());

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

    // Add icons for existing combatants
    for(int i = 0; i < _model->getCombatantCount(); ++i)
    {
        createCombatantIcon(_model->getCombatant(i));
    }

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
    painter.drawPixmap(0, 0, _background);

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

