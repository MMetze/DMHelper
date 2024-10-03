#include "globalsearchframe.h"
#include "ui_globalsearchframe.h"
#include "campaign.h"
#include "bestiary.h"
#include "spellbook.h"
#include "quickref.h"
#include "monsterclass.h"
#include "scaledpixmap.h"

const int GlobalSearchType_None = QTreeWidgetItem::UserType;
const int GlobalSearchType_Campaign = QTreeWidgetItem::UserType + 1;
const int GlobalSearchType_Bestiary = QTreeWidgetItem::UserType + 2;
const int GlobalSearchType_SpellBook = QTreeWidgetItem::UserType + 3;
const int GlobalSearchType_Tools = QTreeWidgetItem::UserType + 4;

const int GlobalSearchData_ObjectName = Qt::UserRole;
const int GlobalSearchData_ExpandedText = Qt::UserRole + 1;
const int GlobalSearchData_LinkedData = Qt::UserRole + 2;

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
        for(int i = 0; i < bestiaryList.size() / 2; ++i)
        {
            /*
             * TODO: check whether the performance is relevant here, makes a lot of difference in debug mode...
            QIcon monsterIcon;
            MonsterClass* monsterClass = Bestiary::Instance()->getMonsterClass(bestiaryList.at(i*2));
            if(monsterClass)
                monsterIcon = QIcon(monsterClass->getIconPixmap(DMHelper::PixmapSize_Thumb));
            */
            bestiaryRootItem->addChild(createTreeWidget(bestiaryList.at(i*2), GlobalSearchType_Bestiary, QIcon(), bestiaryList.at(i*2+1)));
        }
        ui->treeResults->addTopLevelItem(bestiaryRootItem);
    }

    // Search the Spellbook
    QStringList spellbookList = Spellbook::Instance()->search(ui->edtSearch->text());
    if(!spellbookList.isEmpty())
    {
        QTreeWidgetItem* spellbookRootItem = new QTreeWidgetItem(QStringList() << tr("Spellbook"), GlobalSearchType_None);
        for(int i = 0; i < spellbookList.size() / 2; ++i)
        {
            spellbookRootItem->addChild(createTreeWidget(spellbookList.at(i*2), GlobalSearchType_SpellBook, QIcon(), spellbookList.at(i*2+1)));
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

                toolsRootItem->addChild(createTreeWidget(widgetText, GlobalSearchType_Tools, QIcon(), subsectionTitle, QVariant(sectionName)));
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
        case GlobalSearchType_SpellBook:
            {
                if(!item->data(0, GlobalSearchData_ExpandedText).isNull())
                {
                    if(item->text(0) == item->data(0, GlobalSearchData_ObjectName).toString())
                        item->setText(0, item->data(0, GlobalSearchData_ObjectName).toString() + QChar::LineFeed + item->data(0, GlobalSearchData_ExpandedText).toString());
                    else
                        item->setText(0, item->data(0, GlobalSearchData_ObjectName).toString());
                }
            }
            break;
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
            emit campaignObjectSelected(item->data(0, GlobalSearchData_LinkedData).toUuid());
        case GlobalSearchType_Bestiary:
            emit monsterSelected(item->data(0, GlobalSearchData_ObjectName).toString());
            break;
        case GlobalSearchType_SpellBook:
            emit spellSelected(item->data(0, GlobalSearchData_ObjectName).toString());
            break;
        case GlobalSearchType_Tools:
            emit toolSelected(item->data(0, GlobalSearchData_LinkedData).toString());
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

    QString searchResult;
    if(object->matchSearch(searchString, searchResult))
        newItem = createTreeWidget(object->getName(), GlobalSearchType_Campaign, object->getIcon(), searchResult, QVariant(object->getID()));

    QList<CampaignObjectBase*> childObjects = object->getChildObjects();
    for(CampaignObjectBase* childObject : childObjects)
    {
        QTreeWidgetItem* childItem = searchCampaignObject(childObject, searchString);
        if(childItem)
        {
            if(!newItem)
            {
                newItem = createTreeWidget(object->getName(), GlobalSearchType_Campaign, object->getIcon(), QString(), QVariant(object->getID()));
                newItem->setForeground(0, QBrush(Qt::gray));
            }
            newItem->addChild(childItem);
        }
    }

    return newItem;
}

QTreeWidgetItem* GlobalSearchFrame::createTreeWidget(const QString& widgetName, int widgetType, const QIcon &widgetIcon, const QString& expandedText, const QVariant& data)
{
    QTreeWidgetItem* newWidget = new QTreeWidgetItem(QStringList() << widgetName, widgetType);
    newWidget->setData(0, GlobalSearchData_ObjectName, widgetName);
    if(!expandedText.isEmpty())
        newWidget->setData(0, GlobalSearchData_ExpandedText, expandedText);
    if(!data.isNull())
        newWidget->setData(0, GlobalSearchData_LinkedData, data);
    if(!widgetIcon.isNull())
        newWidget->setIcon(0, widgetIcon);

    return newWidget;
}
