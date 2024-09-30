#include "globalsearchframe.h"
#include "ui_globalsearchframe.h"
#include "campaign.h"
#include "bestiary.h"
#include "spellbook.h"
#include "quickref.h"

const int GlobalSearchType_None = QTreeWidgetItem::UserType;
const int GlobalSearchType_Campaign = QTreeWidgetItem::UserType + 1;
const int GlobalSearchType_Bestiary = QTreeWidgetItem::UserType + 2;
const int GlobalSearchType_SpellBook = QTreeWidgetItem::UserType + 3;
const int GlobalSearchType_Tools = QTreeWidgetItem::UserType + 4;

GlobalSearchFrame::GlobalSearchFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::GlobalSearchFrame),
    _campaign(nullptr)
{
    ui->setupUi(this);
    connect(ui->btnSearch, &QPushButton::clicked, this, &GlobalSearchFrame::executeSearch);

    connect(ui->treeResults, &QTreeWidget::itemClicked, this, &GlobalSearchFrame::handleItemClicked);
    connect(ui->treeResults, &QTreeWidget::itemDoubleClicked, this, &GlobalSearchFrame::handleItemDoubleClicked);
}

GlobalSearchFrame::~GlobalSearchFrame()
{
    delete ui;
}

void GlobalSearchFrame::setCampaign(Campaign* campaign)
{
    _campaign = campaign;
}

void GlobalSearchFrame::executeSearch()
{
    ui->treeResults->clear();

    // Search the Campaign
    if(_campaign)
    {
        QTreeWidgetItem* campaignRootItem = nullptr;
        QList<CampaignObjectBase*> campaignObjects = _campaign->getChildObjects();
        for(CampaignObjectBase* object : campaignObjects)
        {
            QTreeWidgetItem* objectItem = searchCampaignObject(object, ui->edtSearch->text());
            if(objectItem)
            {
                if(!campaignRootItem)
                    campaignRootItem = new QTreeWidgetItem(QStringList() << tr("Campaign"), GlobalSearchType_None);
                campaignRootItem->addChild(objectItem);
            }
        }

        if(campaignRootItem)
            ui->treeResults->addTopLevelItem(campaignRootItem);
    }

    // Search the Bestiary
    QStringList bestiaryList = Bestiary::Instance()->search(ui->edtSearch->text());
    if(!bestiaryList.isEmpty())
    {
        QTreeWidgetItem* bestiaryRootItem = new QTreeWidgetItem(QStringList() << tr("Bestiary"), GlobalSearchType_None);
        for(const QString& monsterName : bestiaryList)
        {
            QTreeWidgetItem* monsterItem = new QTreeWidgetItem(QStringList() << monsterName, GlobalSearchType_Bestiary);
            bestiaryRootItem->addChild(monsterItem);
        }
        ui->treeResults->addTopLevelItem(bestiaryRootItem);
    }

    // Search the Spellbook
    QStringList spellbookList = Spellbook::Instance()->search(ui->edtSearch->text());
    if(!spellbookList.isEmpty())
    {
        QTreeWidgetItem* spellbookRootItem = new QTreeWidgetItem(QStringList() << tr("Spellbook"), GlobalSearchType_None);
        for(const QString& spellName : spellbookList)
        {
            QTreeWidgetItem* spellItem = new QTreeWidgetItem(QStringList() << spellName, GlobalSearchType_SpellBook);
            spellbookRootItem->addChild(spellItem);
        }
        ui->treeResults->addTopLevelItem(spellbookRootItem);
    }

    // Search the Tools
    if(QuickRef::Instance())
    {
        QStringList quickRefList = QuickRef::Instance()->search(ui->edtSearch->text());
        if(!quickRefList.isEmpty())
        {
            QTreeWidgetItem* toolsRootItem = new QTreeWidgetItem(QStringList() << tr("Tools"), GlobalSearchType_None);
            for(int i = 0; i < quickRefList.size() / 2; i++)
            {
                QString sectionName = quickRefList.at(i*2);
                QString subsectionTitle = quickRefList.at(i*2 + 1);
                QString widgetText = sectionName + (subsectionTitle.isEmpty() ? QString("") : QString(" - ") + subsectionTitle);

                if(sectionName.isEmpty())
                    continue;

                QTreeWidgetItem* toolItem = new QTreeWidgetItem(QStringList() << widgetText, GlobalSearchType_Tools);
                toolItem->setData(0, Qt::UserRole, QVariant(sectionName));
                toolsRootItem->addChild(toolItem);
            }
            ui->treeResults->addTopLevelItem(toolsRootItem);
        }
    }

    ui->treeResults->expandAll();
}

void GlobalSearchFrame::handleItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    if((!item) || (item->type() <= GlobalSearchType_None))
        return;

    // TODO: Add support to show more details about the selected item when clicked
    switch(item->type())
    {
        case GlobalSearchType_Campaign:
        case GlobalSearchType_Bestiary:
            /* TODO: This sort of thing will work
        {
            QTreeWidgetItem* testItem = new QTreeWidgetItem(QStringList() << tr("Here is a test description under the item!"), GlobalSearchType_None);
            item->insertChild(0, testItem);
            break;
        }
*/
        case GlobalSearchType_SpellBook:
        case GlobalSearchType_Tools:
        default:
            break;
    }
}

void GlobalSearchFrame::handleItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    if((!item) || (item->type() <= GlobalSearchType_None))
        return;

    switch(item->type())
    {
        case GlobalSearchType_Campaign:
            emit campaignObjectSelected(item->data(0, Qt::UserRole).toUuid());
        case GlobalSearchType_Bestiary:
            emit monsterSelected(item->text(0));
            break;
        case GlobalSearchType_SpellBook:
            emit spellSelected(item->text(0));
            break;
        case GlobalSearchType_Tools:
            emit toolSelected(item->data(0, Qt::UserRole).toString());
            break;
        default:
            break;
    }

    emit frameAccept();
}

QTreeWidgetItem* GlobalSearchFrame::searchCampaignObject(CampaignObjectBase* object, const QString& searchString)
{
    QTreeWidgetItem* newItem = nullptr;

    if(!object)
        return newItem;

    if(object->matchSearch(searchString))
    {
        newItem = new QTreeWidgetItem(QStringList() << object->getName(), GlobalSearchType_Campaign);
        newItem->setIcon(0, object->getIcon());
        newItem->setData(0, Qt::UserRole, QVariant(object->getID()));
    }

    QList<CampaignObjectBase*> childObjects = object->getChildObjects();
    for(CampaignObjectBase* childObject : childObjects)
    {
        QTreeWidgetItem* childItem = searchCampaignObject(childObject, searchString);
        if(childItem)
        {
            if(!newItem)
            {
                newItem = new QTreeWidgetItem(QStringList() << object->getName(), GlobalSearchType_Campaign);
                newItem->setIcon(0, object->getIcon());
                newItem->setData(0, Qt::UserRole, QVariant(object->getID()));
                newItem->setForeground(0, QBrush(Qt::gray));
            }
            newItem->addChild(childItem);
        }
    }

    return newItem;
}
