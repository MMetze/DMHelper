#include "layertokens.h"
#include "battledialogmodel.h"
#include "battledialogmodelcombatant.h"
#include "unselectedpixmap.h"
#include "publishglbattletoken.h"
#include "campaign.h"
#include "character.h"
#include "bestiary.h"
#include "monster.h"
#include "monsterclass.h"
#include "battledialogmodelcharacter.h"
#include "battledialogmodelmonsterclass.h"
#include "battledialogmodelmonstercombatant.h"
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QDebug>

LayerTokens::LayerTokens(BattleDialogModel* model, const QString& name, int order, QObject *parent) :
    Layer{name, order, parent},
    _glScene(nullptr),
    _model(model),
    _combatants(),
    _tokens(),
    _combatantIcons(),
    _combatantTokens(),
    _scale(DMHelper::STARTING_GRID_SCALE)
{
}

LayerTokens::~LayerTokens()
{
    cleanupDM();
    cleanupPlayer();
}

void LayerTokens::inputXML(const QDomElement &element, bool isImport)
{
    Layer::inputXML(element, isImport);
}

void LayerTokens::postProcessXML(Campaign* campaign, const QDomElement &element, bool isImport)
{
    if((!_model) || (!campaign))
        return;

    QDomElement combatantsElement = element.firstChildElement("combatants");
    if(combatantsElement.isNull())
        return;

    QDomElement combatantElement = combatantsElement.firstChildElement("battlecombatant");
    while(!combatantElement.isNull())
    {
        BattleDialogModelCombatant* combatant = nullptr;
        int combatantType = combatantElement.attribute("type",QString::number(DMHelper::CombatantType_Base)).toInt();
        if(combatantType == DMHelper::CombatantType_Character)
        {
            QUuid combatantId = QUuid(combatantElement.attribute("combatantId"));
            Character* character = campaign->getCharacterById(combatantId);
            if(!character)
                character = campaign->getNPCById(combatantId);

            if(character)
                combatant = new BattleDialogModelCharacter(character);
            else
                qDebug() << "[LayerTokens] Unknown character ID type found: " << combatantId;
        }
        else if(combatantType == DMHelper::CombatantType_Monster)
        {
            int monsterType = combatantElement.attribute("monsterType",QString::number(BattleDialogModelMonsterBase::BattleMonsterType_Base)).toInt();
            if(monsterType == BattleDialogModelMonsterBase::BattleMonsterType_Combatant)
            {
                QUuid combatantId = QUuid(combatantElement.attribute("combatantId"));
                Monster* monster = dynamic_cast<Monster*>(_model->getCombatantById(combatantId));
                if(monster)
                    combatant = new BattleDialogModelMonsterCombatant(monster);
                else
                    qDebug() << "[LayerTokens] Unknown monster ID type found: " << combatantId << " for battle";// " << battleId;
            }
            else if(monsterType == BattleDialogModelMonsterBase::BattleMonsterType_Class)
            {
                QString monsterClassName = combatantElement.attribute("monsterClass");
                MonsterClass* monsterClass = Bestiary::Instance()->getMonsterClass(monsterClassName);
                if(monsterClass)
                    combatant = new BattleDialogModelMonsterClass(monsterClass);
                else
                    qDebug() << "[LayerTokens] Unknown monster class type found: " << monsterClassName;
            }
            else
            {
                qDebug() << "[LayerTokens] Invalid monster type found: " << monsterType;
            }
        }
        else
        {
            qDebug() << "[LayerTokens] Invalid combatant type found: " << combatantType;
        }

        if(combatant)
        {
            combatant->inputXML(combatantElement, isImport);
            _model->appendCombatant(combatant);
            // TODO: Layers - add this back
            /*
            if( ((!activeId.isNull()) && (combatant->getID() == activeId)) ||
                (( activeId.isNull()) && (combatant->getIntID() == activeIdInt)) )
            {
                _battleModel->setActiveCombatant(combatant);
            }
            */
        }

        combatantElement = combatantElement.nextSiblingElement("battlecombatant");
    }
}

QRectF LayerTokens::boundingRect() const
{
    return QRectF();
}

QImage LayerTokens::getLayerIcon() const
{
    return QImage(":/img/data/icon_contentcharacter.png");
}

DMHelper::LayerType LayerTokens::getType() const
{
    return DMHelper::LayerType_Tokens;
}

Layer* LayerTokens::clone() const
{
    LayerTokens* newLayer = new LayerTokens(_model, _name, _order);

    copyBaseValues(newLayer);

    return newLayer;
}

void LayerTokens::applyOrder(int order)
{
    foreach(QGraphicsPixmapItem* pixmapItem, _combatantIcons)
    {
        if(pixmapItem)
            pixmapItem->setZValue(order);
    }
}

void LayerTokens::applyLayerVisible(bool layerVisible)
{
    foreach(QGraphicsPixmapItem* pixmapItem, _combatantIcons)
    {
        if(pixmapItem)
            pixmapItem->setVisible(layerVisible);
    }
}

void LayerTokens::applyOpacity(qreal opacity)
{
    foreach(QGraphicsPixmapItem* pixmapItem, _combatantIcons)
    {
        if(pixmapItem)
            pixmapItem->setOpacity(opacity);
    }
}

void LayerTokens::dmInitialize(QGraphicsScene& scene)
{
    for(int i = 0; i < _combatants.count(); ++i)
    {
        createCombatantIcon(scene, _combatants.at(i));
    }

    Layer::dmInitialize(scene);
}

void LayerTokens::dmUninitialize()
{
    cleanupDM();
}

void LayerTokens::dmUpdate()
{
}

void LayerTokens::playerGLInitialize(PublishGLScene* scene)
{
    if(!scene)
        return;

    _glScene = scene;

    for(int i = 0; i < _combatants.count(); ++i)
    {
        BattleDialogModelCombatant* combatant = _combatants.at(i);
        if(combatant)
        {
            PublishGLBattleToken* combatantToken = new PublishGLBattleToken(_glScene, combatant);
            // TODO: Layers - is this needed?
            /*
            BattleDialogModelCharacter* characterCombatant = dynamic_cast<BattleDialogModelCharacter*>(combatant);
            if((characterCombatant) && (characterCombatant->getCharacter()) && (characterCombatant->getCharacter()->isInParty()))
                combatantToken->setPC(true);
            */
            // TODO: Layers - how do selections work?
            /*
            if(combatant->getSelected())
                combatantToken->addEffect(*_selectionToken);
            */

            _combatantTokens.insert(combatant, combatantToken);
            // TODO: Layers
            //connect(combatantToken, &PublishGLBattleObject::changed, this, &PublishGLBattleRenderer::updateWidget);
            //connect(combatantToken, &PublishGLBattleToken::selectionChanged, this, &PublishGLBattleRenderer::tokenSelectionChanged);
        }
    }
}

void LayerTokens::playerGLUninitialize()
{
    cleanupPlayer();
}

void LayerTokens::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    if(!_model)
        return;

    for(BattleDialogModelCombatant* combatant : _combatants)
    {
        if((combatant) && (combatant->getKnown()) && (combatant->getShown()) &&
           ((_model->getShowDead()) || (combatant->getHitPoints() > 0)) &&
           ((_model->getShowAlive()) || (combatant->getHitPoints() <= 0)))
        {
            PublishGLBattleToken* token = _combatantTokens.value(combatant);
            if(token)
            {
                functions->glUniformMatrix4fv(defaultModelMatrix, 1, GL_FALSE, token->getMatrixData());
                token->paintGL();
                token->paintEffects(defaultModelMatrix);
            }
        }
    }
}

void LayerTokens::playerGLResize(int w, int h)
{
}

void LayerTokens::initialize(const QSize& layerSize)
{
}

void LayerTokens::uninitialize()
{
}

void LayerTokens::setScale(int scale)
{
    if(_scale != scale)
    {
        // TODO: if the scale is changed, update the token scales
        _scale = scale;
    }
}

void LayerTokens::addCombatant(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return;

    if(!_combatants.contains(combatant))
        _combatants.append(combatant);
}

void LayerTokens::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    QDomElement combatantsElement = doc.createElement("combatants");
    for(BattleDialogModelCombatant* combatant : _combatants)
    {
        if(combatant)
            combatant->outputXML(doc, combatantsElement, targetDirectory, isExport);
    }
    element.appendChild(combatantsElement);
}

void LayerTokens::cleanupDM()
{
    _combatantIcons.clear();
}

void LayerTokens::createCombatantIcon(QGraphicsScene& scene, BattleDialogModelCombatant* combatant)
{
    if((!combatant) || (_combatantIcons.contains(combatant)))
        return;

    if(combatant)
    {
        QPixmap pix = combatant->getIconPixmap(DMHelper::PixmapSize_Battle);
        if(combatant->hasCondition(Combatant::Condition_Unconscious))
        {
            QImage originalImage = pix.toImage();
            QImage grayscaleImage = originalImage.convertToFormat(QImage::Format_Grayscale8);
            pix = QPixmap::fromImage(grayscaleImage);
        }
        Combatant::drawConditions(&pix, combatant->getConditions());

        QGraphicsPixmapItem* pixmapItem = new UnselectedPixmap(pix, combatant);
        scene.addItem(pixmapItem);
        pixmapItem->setFlag(QGraphicsItem::ItemIsMovable, true);
        pixmapItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
        pixmapItem->setPos(combatant->getPosition());
        pixmapItem->setOffset(-static_cast<qreal>(pix.width())/2.0, -static_cast<qreal>(pix.height())/2.0);
        qreal sizeFactor = combatant->getSizeFactor();
        qreal scaleFactor = (static_cast<qreal>(_scale-2)) * sizeFactor / static_cast<qreal>(qMax(pix.width(),pix.height()));
        pixmapItem->setScale(scaleFactor);

        QString itemTooltip = QString("<b>") + combatant->getName() + QString("</b>");
        QStringList conditionString = Combatant::getConditionString(combatant->getConditions());
        if(conditionString.count() > 0)
            itemTooltip += QString("<p>") + conditionString.join(QString("<br/>"));
        pixmapItem->setToolTip(itemTooltip);

        qDebug() << "[LayerTokens] combatant icon added " << combatant->getName() << ", scale " << scaleFactor;

        qreal gridSize = (static_cast<qreal>(_scale)) / scaleFactor;
        qreal gridOffset = gridSize * static_cast<qreal>(sizeFactor) / 2.0;
        QGraphicsRectItem* rect = new QGraphicsRectItem(0, 0, gridSize * sizeFactor, gridSize * static_cast<qreal>(sizeFactor));
        rect->setPos(-gridOffset, -gridOffset);
        // TODO: Layers
        //rect->setData(BattleDialogItemChild_Index, BattleDialogItemChild_Area);
        rect->setParentItem(pixmapItem);
        rect->setVisible(false);
        qDebug() << "[LayerTokens] created " << pixmapItem << " with area child " << rect;

        // TODO: Layers
        // applyPersonalEffectToItem(pixmapItem);

        _combatantIcons.insert(combatant, pixmapItem);

        //connect(combatant, SIGNAL(combatantMoved(BattleDialogModelCombatant*)), this, SLOT(handleCombatantMoved(BattleDialogModelCombatant*)), static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection));
        //connect(combatant, SIGNAL(combatantMoved(BattleDialogModelCombatant*)), this, SLOT(updateHighlights()), static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection));
        //connect(combatant, SIGNAL(combatantSelected(BattleDialogModelCombatant*)), this, SLOT(handleCombatantSelected(BattleDialogModelCombatant*)));
    }
}

void LayerTokens::cleanupPlayer()
{
    qDeleteAll(_tokens);
    _tokens.clear();
    _combatantTokens.clear();

    _glScene = nullptr;
}
