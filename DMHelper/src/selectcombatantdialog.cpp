#include "selectcombatantdialog.h"
#include "ui_selectcombatantdialog.h"
#include "character.h"
#include "layertokens.h"
#include <QGraphicsItem>
#include <QPainter>

SelectCombatantDialog::SelectCombatantDialog(BattleDialogModel& model, BattleDialogModelObject* thisItem, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectCombatantDialog),
    _model(model),
    _thisItem(thisItem)
{
    ui->setupUi(this);
    readModel();

    connect(ui->listWidget, &QListWidget::itemDoubleClicked, this, &QDialog::accept);
}

SelectCombatantDialog::~SelectCombatantDialog()
{
    delete ui;
}

BattleDialogModelObject* SelectCombatantDialog::getSelectedObject() const
{
    if(!ui->listWidget->currentItem())
        return nullptr;

    return ui->listWidget->currentItem()->data(Qt::UserRole).value<BattleDialogModelObject*>();
}

bool SelectCombatantDialog::isCentered() const
{
    return ui->chkCentered->isChecked();
}

void SelectCombatantDialog::readModel()
{
    QList<BattleDialogModelCombatant*> combatantList = _model.getCombatantList();

    // Find the PCs and add them
    foreach(BattleDialogModelCombatant* combatant, combatantList)
    {
        if((combatant) && (combatant->getCombatantType() == DMHelper::CombatantType_Character) && (combatant->getCombatant()))
        {
            Character* character = dynamic_cast<Character*>(combatant->getCombatant());
            if((character) && (character->isInParty()))
                addCombatant(combatant);
        }
    }

    // Add all non-PC combatants
    foreach(BattleDialogModelCombatant* combatant, combatantList)
    {
        if(combatant)
        {
            if(combatant->getCombatantType() != DMHelper::CombatantType_Character)
            {
                addCombatant(combatant);
            }
            else
            {
                Character* character = dynamic_cast<Character*>(combatant->getCombatant());
                if((!character) || (!character->isInParty()))
                    addCombatant(combatant);
            }
        }
    }
    
    // Add all effects
    QList<Layer*> tokenLayers = _model.getLayerScene().getLayers(DMHelper::LayerType_Tokens);
    foreach(Layer* layer, tokenLayers)
    {
        LayerTokens* tokenLayer = dynamic_cast<LayerTokens*>(layer);
        if(tokenLayer)
        {
            QList<BattleDialogModelEffect*> effectList = tokenLayer->getEffects();
            foreach(BattleDialogModelEffect* effect, effectList)
            {
                QList<CampaignObjectBase*> effectChildren = effect->getChildObjects();
                if(effectChildren.isEmpty())
                {
                    addEffect(effect, nullptr, tokenLayer->getEffectItem(effect));
                }
                else
                {
                    BattleDialogModelEffect* childEffect = dynamic_cast<BattleDialogModelEffect*>(effectChildren.first());
                    if(childEffect)
                        addEffect(effect, childEffect, tokenLayer->getEffectItem(childEffect));
                }
            }
        }
    }
}

void SelectCombatantDialog::addCombatant(BattleDialogModelCombatant* combatant)
{
    if((!combatant) || (dynamic_cast<BattleDialogModelObject*>(combatant) == _thisItem))
        return;
    
    QListWidgetItem* newItem = new QListWidgetItem(QIcon(combatant->getIconPixmap(DMHelper::PixmapSize_Showcase)), combatant->getName());
    newItem->setData(Qt::UserRole, QVariant::fromValue(dynamic_cast<BattleDialogModelObject*>(combatant)));
    ui->listWidget->addItem(newItem);
}

void SelectCombatantDialog::addEffect(BattleDialogModelEffect* effect, BattleDialogModelEffect* childEffect, QGraphicsItem* effectItem)
{
    if((!effect) || (!effectItem) || (dynamic_cast<BattleDialogModelObject*>(effect) == _thisItem))
        return;

    QPixmap effectPixmap(256, 256);
    effectPixmap.fill(Qt::transparent);
    QPainter painter;
    QStyleOptionGraphicsItem options;
    painter.begin(&effectPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QRectF localRect = effectItem->boundingRect();
    qreal itemScale = qMin(256.0 / localRect.width(), 256.0 / localRect.height());
    painter.scale(itemScale, itemScale);
    painter.translate(-localRect.x(), -localRect.y());
    effectItem->paint(&painter, &options);
    painter.end();

    QString effectName;
    if((childEffect) && (!childEffect->getName().isEmpty()))
        effectName = effect->getName();
    if(effectName.isEmpty())
    {
        effectName = effect->getName();
        if(effectName.isEmpty())
            effectName = QString("Effect");
    }

    QListWidgetItem* newItem = new QListWidgetItem(QIcon(effectPixmap), effectName);
    newItem->setData(Qt::UserRole, QVariant::fromValue(dynamic_cast<BattleDialogModelObject*>(effect)));
    ui->listWidget->addItem(newItem);
}
