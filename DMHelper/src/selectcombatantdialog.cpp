#include "selectcombatantdialog.h"
#include "ui_selectcombatantdialog.h"
#include "characterv2.h"
#include "layertokens.h"
#include <QGraphicsItem>
#include <QPainter>

SelectCombatantDialog::SelectCombatantDialog(BattleDialogModel& model, BattleDialogModelObject* thisItem, bool includeParents, bool includeChildren, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectCombatantDialog),
    _model(model),
    _thisItemList(),
    _includeParents(includeParents),
    _includeChildren(includeChildren)
{
    ui->setupUi(this);
    if(thisItem)
        _thisItemList.append(thisItem);

    connect(ui->listWidget, &QListWidget::itemDoubleClicked, this, &QDialog::accept);
}

SelectCombatantDialog::~SelectCombatantDialog()
{
    delete ui;
}

void SelectCombatantDialog::addObject(BattleDialogModelObject* itemObject)
{
    if((!itemObject) || (_thisItemList.contains(itemObject)))
        return;

    _thisItemList.append(itemObject);
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

void SelectCombatantDialog::showEvent(QShowEvent *event)
{
    readModel();
    QDialog::showEvent(event);
}

void SelectCombatantDialog::readModel()
{
    QList<BattleDialogModelCombatant*> combatantList = _model.getCombatantList();

    // Find the PCs and add them
    foreach(BattleDialogModelCombatant* combatant, combatantList)
    {
        if((combatant) && (combatant->getCombatantType() == DMHelper::CombatantType_Character) && (combatant->getCombatant()))
        {
            Characterv2* character = dynamic_cast<Characterv2*>(combatant->getCombatant());
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
                Characterv2* character = dynamic_cast<Characterv2*>(combatant->getCombatant());
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

    if(!_includeParents)
        removeParents();
}

void SelectCombatantDialog::addCombatant(BattleDialogModelCombatant* combatant)
{
    if((!combatant) || (_thisItemList.contains(dynamic_cast<BattleDialogModelObject*>(combatant))))
        return;
    
    if((!_includeChildren) && (isChild(combatant)))
        return;

    QListWidgetItem* newItem = new QListWidgetItem(QIcon(combatant->getIconPixmap(DMHelper::PixmapSize_Showcase)), combatant->getName());
    newItem->setData(Qt::UserRole, QVariant::fromValue(dynamic_cast<BattleDialogModelObject*>(combatant)));
    ui->listWidget->addItem(newItem);
}

void SelectCombatantDialog::addEffect(BattleDialogModelEffect* effect, BattleDialogModelEffect* childEffect, QGraphicsItem* effectItem)
{
    if((!effect) || (!effectItem) || (_thisItemList.contains(dynamic_cast<BattleDialogModelObject*>(effect))))
        return;

    if((!_includeChildren) && (isChild(effect) || isChild(childEffect)))
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

bool SelectCombatantDialog::isChild(BattleDialogModelObject* item)
{
    if((!item) || (_thisItemList.contains(item)))
        return false;

    BattleDialogModelObject* parentItem = item->getLinkedObject();
    while(parentItem)
    {
        if(_thisItemList.contains(parentItem))
            return true;

        parentItem = parentItem->getLinkedObject();
    }

    return false;
}

void SelectCombatantDialog::removeParents()
{
    foreach(BattleDialogModelObject* thisItem, _thisItemList)
    {
        if(thisItem)
        {
            BattleDialogModelObject* parentItem = thisItem->getLinkedObject();
            while(parentItem)
            {
                for(int i = 0; i < ui->listWidget->count(); ++i)
                {
                    QListWidgetItem* item = ui->listWidget->item(i);
                    if((item) && (item->data(Qt::UserRole).value<BattleDialogModelObject*>() == parentItem))
                        delete item;
                }
                parentItem = parentItem->getLinkedObject();
            }
        }
    }
}
