#include "globalsearchframe.h"
#include "ui_globalsearchframe.h"
#include "bestiary.h"

const int GlobalSearchType_None = QTreeWidgetItem::UserType;
const int GlobalSearchType_Campaign = QTreeWidgetItem::UserType + 1;
const int GlobalSearchType_Bestiary = QTreeWidgetItem::UserType + 2;
const int GlobalSearchType_SpellBook = QTreeWidgetItem::UserType + 3;
const int GlobalSearchType_Items = QTreeWidgetItem::UserType + 4;
const int GlobalSearchType_Tools = QTreeWidgetItem::UserType + 5;

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
        QTreeWidgetItem* campaignRootItem = new QTreeWidgetItem(QStringList() << tr("Campaign"), GlobalSearchType_None);
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
        ui->treeResults->expandItem(bestiaryRootItem);
    }

    // Search the Spellbook
    QTreeWidgetItem* spellBookRootItem = new QTreeWidgetItem(QStringList() << tr("Spellbook"), GlobalSearchType_None);
    ui->treeResults->addTopLevelItem(spellBookRootItem);

    // Search the Items
    QTreeWidgetItem* itemsRootItem = new QTreeWidgetItem(QStringList() << tr("Items"), GlobalSearchType_None);
    ui->treeResults->addTopLevelItem(itemsRootItem);

    // Search the Tools
    QTreeWidgetItem* toolsRootItem = new QTreeWidgetItem(QStringList() << tr("Other Tools"), GlobalSearchType_None);
    ui->treeResults->addTopLevelItem(toolsRootItem);

}

void GlobalSearchFrame::handleItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    if((!item) || (item->type() <= GlobalSearchType_None))
        return;

    switch(item->type())
    {
        case GlobalSearchType_Bestiary:
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
        case GlobalSearchType_Bestiary:
            emit monsterSelected(item->text(0));
            break;
        default:
            break;
    }
}
