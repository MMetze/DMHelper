#include "battlerenderer.h"
#include "battledialoggraphicsscene.h"

BattleRenderer::BattleRenderer(EncounterBattle& battle, QObject *parent) :
    CampaignObjectRenderer(parent),
    _battle(battle),
    _scene(new BattleDialogGraphicsScene(this))
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
}

void BattleRenderer::stopRendering()
{
}

void BattleRenderer::targetResized(const QSize& newSize)
{
}

void BattleRenderer::setRotation(int rotation)
{
}
