#include "chaserselectiondialog.h"
#include "character.h"
#include "combatant.h"
#include "monster.h"
#include "ui_chaserselectiondialog.h"
#include <QKeyEvent>

// Todo: add external monsters

ChaserSelectionDialog::ChaserSelectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChaserSelectionDialog)
{
    ui->setupUi(this);

    connect(ui->btnAddPursuer,SIGNAL(clicked()),this,SLOT(addPursuer()));
    connect(ui->btnAddQuarry,SIGNAL(clicked()),this,SLOT(addQuarry()));
    connect(ui->btnRemovePursuer,SIGNAL(clicked()),this,SLOT(removePursuer()));
    connect(ui->btnRemoveQuarry,SIGNAL(clicked()),this,SLOT(removeQuarry()));

    ui->lstBattle->installEventFilter(this);
    ui->lstCharacters->installEventFilter(this);
    ui->lstEncounter->installEventFilter(this);
    ui->lstPursuers->installEventFilter(this);
    ui->lstQuarry->installEventFilter(this);

    ui->lstCharacters->setFocus();
}

ChaserSelectionDialog::~ChaserSelectionDialog()
{
    delete ui;
}

void ChaserSelectionDialog::addCharacters(QList<Character*> characters)
{
    for(int i = 0; i < characters.count(); ++i)
    {
        QListWidgetItem* newItem = new QListWidgetItem(characters.at(i)->getName());
        newItem->setSelected(characters.at(i)->getActive());
        newItem->setData(WidgetData_Pointer, QVariant::fromValue(dynamic_cast<Combatant*>(characters.at(i))));
        newItem->setData(WidgetData_Type, WidgetType_Character);
        ui->lstCharacters->addItem(newItem);
    }

    ui->lstCharacters->selectAll();
}

void ChaserSelectionDialog::clearCharacters()
{
    ui->lstCharacters->clear();
}

void ChaserSelectionDialog::addEncounterCombatants(CombatantGroupList combatants)
{
    for(int i = 0; i < combatants.count(); ++i)
    {
        if(combatants.at(i).second != NULL)
        {

            QString name = combatants.at(i).second->getName();
            if(name.isEmpty())
            {
                Monster* monster = dynamic_cast<Monster*>(combatants.at(i).second);
                if(monster)
                {
                    name = monster->getMonsterClassName();
                }
            }

            QString combatantName = QString::number(combatants.at(i).first) + QString("x ") + name;
            QListWidgetItem* newItem = new QListWidgetItem(combatantName);
            newItem->setData(WidgetData_Pointer, QVariant::fromValue(combatants.at(i).second));
            newItem->setData(WidgetData_Type, WidgetType_Encounter);
            newItem->setData(WidgetData_CombatantCount, combatants.at(i).first);
            ui->lstEncounter->addItem(newItem);
        }
    }
}

void ChaserSelectionDialog::clearEncounterCombatants()
{
    ui->lstEncounter->clear();
}

void ChaserSelectionDialog::addBattleCombatants(QList<Combatant*> combatants)
{
    for(int i = 0; i < combatants.count(); ++i)
    {
        QListWidgetItem* newItem = new QListWidgetItem(combatants.at(i)->getName());
        newItem->setData(WidgetData_Pointer, QVariant::fromValue(combatants.at(i)));
        newItem->setData(WidgetData_Type, WidgetType_Battle);
        ui->lstBattle->addItem(newItem);
    }
}

void ChaserSelectionDialog::clearBattleCombatants()
{
    ui->lstBattle->clear();
}

QList<Combatant*> ChaserSelectionDialog::getPursuers() const
{
    return getListCombatants(ui->lstPursuers);
}

CombatantGroupList ChaserSelectionDialog::getPursuerGroups() const
{
    return getListGroups(ui->lstPursuers);
}

QList<Combatant*> ChaserSelectionDialog::getQuarry() const
{
    return getListCombatants(ui->lstQuarry);
}

CombatantGroupList ChaserSelectionDialog::getQuarryGroups() const
{
    return getListGroups(ui->lstQuarry);
}

void ChaserSelectionDialog::addPursuer()
{
    moveSelectedToList(ui->lstPursuers);
    ui->lstPursuers->setFocus();
}

void ChaserSelectionDialog::addQuarry()
{
    moveSelectedToList(ui->lstQuarry);
    ui->lstQuarry->setFocus();
}

void ChaserSelectionDialog::removePursuer()
{
    removeSelectedFromList(ui->lstPursuers);
}

void ChaserSelectionDialog::removeQuarry()
{
    removeSelectedFromList(ui->lstQuarry);
}

bool ChaserSelectionDialog::eventFilter(QObject *obj, QEvent *event)
{
    QListWidget* focusWidget = dynamic_cast<QListWidget*>(obj);
    if(!focusWidget)
        return QObject::eventFilter(obj, event);

    if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Left)
        {
            if(focusWidget == ui->lstQuarry)
            {
                removeQuarry();
                return true;
            }
            else if((focusWidget == ui->lstCharacters) || (focusWidget == ui->lstEncounter) || (focusWidget == ui->lstBattle))
            {
                addPursuer();
                return true;
            }
        }
        else if(keyEvent->key() == Qt::Key_Right)
        {
            if(focusWidget == ui->lstPursuers)
            {
                removePursuer();
                return true;
            }
            else if((focusWidget == ui->lstCharacters) || (focusWidget == ui->lstEncounter) || (focusWidget == ui->lstBattle))
            {
                addQuarry();
                return true;
            }
        }
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}

void ChaserSelectionDialog::moveSelectedToList(QListWidget* list)
{
    if(!list)
        return;

    list->clearSelection();

    if( (ui->lstCharacters->selectedItems().count() > 0) ||
        (ui->lstEncounter->selectedItems().count() > 0) ||
        (ui->lstBattle->selectedItems().count() > 0) )
    {
        moveListToList(ui->lstCharacters, list);
        moveListToList(ui->lstEncounter, list);
        moveListToList(ui->lstBattle, list);
    }
    else
    {
        QListWidget* fromList = dynamic_cast<QListWidget*>(focusWidget());
        if(fromList)
        {
            if((fromList == ui->lstCharacters) ||
               (fromList == ui->lstEncounter) ||
               (fromList == ui->lstBattle))
            {
                QListWidgetItem* widgetItem = fromList->takeItem(fromList->currentRow());
                if(widgetItem)
                {
                    list->addItem(widgetItem);
                    widgetItem->setSelected(true);
                }
            }
        }
    }
}

void ChaserSelectionDialog::moveListToList(QListWidget* from, QListWidget* to)
{
    if(!from || !to)
        return;

    QList<QListWidgetItem *> widgetList = from->selectedItems();

    for(int i = 0; i < widgetList.count(); ++i)
    {
        QListWidgetItem* widgetItem = from->takeItem(from->row(widgetList.at(i)));
        to->addItem(widgetItem);
        widgetItem->setSelected(true);
    }
}

void ChaserSelectionDialog::removeSelectedFromList(QListWidget* list)
{
    if(!list)
        return;

    QList<QListWidgetItem *> widgetList = list->selectedItems();

    ui->lstCharacters->clearSelection();
    ui->lstEncounter->clearSelection();
    ui->lstBattle->clearSelection();

    for(int i = 0; i < widgetList.count(); ++i)
    {
        QListWidgetItem* widgetItem = list->takeItem(list->row(widgetList.at(i)));
        switch(widgetItem->data(WidgetData_Type).toInt())
        {
            case WidgetType_Character:
                ui->lstCharacters->addItem(widgetItem);
                widgetItem->setSelected(true);
                break;
            case WidgetType_Encounter:
                ui->lstEncounter->addItem(widgetItem);
                widgetItem->setSelected(true);
                break;
            case WidgetType_Battle:
                ui->lstBattle->addItem(widgetItem);
                widgetItem->setSelected(true);
                break;
            default:
                delete widgetItem;
                break;
        }
    }

    if(ui->lstCharacters->selectedItems().count() > 0)
    {
        ui->lstCharacters->setFocus();
    }
    else if(ui->lstEncounter->selectedItems().count() > 0)
    {
        ui->lstEncounter->setFocus();
    }
    else
    {
        ui->lstBattle->setFocus();
    }
}

QList<Combatant*> ChaserSelectionDialog::getListCombatants(QListWidget* list) const
{
    QList<Combatant*> result;

    if(list)
    {
        for(int i = 0; i < list->count(); ++i)
        {
            QListWidgetItem* widget = list->item(i);
            int widgetType = widget->data(WidgetData_Type).toInt();
            if((widgetType == WidgetType_Character) || (widgetType == WidgetType_Battle))
            {
                result.append(widget->data(WidgetData_Pointer).value<Combatant*>());
            }
        }
    }

    return result;
}

CombatantGroupList ChaserSelectionDialog::getListGroups(QListWidget* list) const
{
    CombatantGroupList result;

    if(list)
    {
        for(int i = 0; i < list->count(); ++i)
        {
            QListWidgetItem* widget = list->item(i);
            int widgetType = widget->data(WidgetData_Type).toInt();
            if((widgetType == WidgetType_Encounter) || (widgetType == WidgetType_Local))
            {
                result.append(CombatantGroup(widget->data(WidgetData_CombatantCount).toInt(), widget->data(WidgetData_Pointer).value<Combatant*>()));
            }
        }
    }

    return result;
}

