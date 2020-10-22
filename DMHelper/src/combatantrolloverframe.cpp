#include "combatantrolloverframe.h"
#include "battledialogmodelcombatant.h"
#include "battledialogmodelmonsterbase.h"
#include "monsterclass.h"
#include "monsteraction.h"
#include "ui_combatantrolloverframe.h"

CombatantRolloverFrame::CombatantRolloverFrame(BattleDialogModelCombatant* combatant, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CombatantRolloverFrame)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    ui->frameInfo->setWindowFlags(Qt::FramelessWindowHint);
    ui->frameInfo->setAttribute(Qt::WA_TranslucentBackground);

    ui->lblFrameTopLeft->setWindowFlags(Qt::FramelessWindowHint);
    ui->lblFrameTopLeft->setAttribute(Qt::WA_TranslucentBackground);
    ui->lblFrameLeft->setWindowFlags(Qt::FramelessWindowHint);
    ui->lblFrameLeft->setAttribute(Qt::WA_TranslucentBackground);
    ui->lblFrameBottomLeft->setWindowFlags(Qt::FramelessWindowHint);
    ui->lblFrameBottomLeft->setAttribute(Qt::WA_TranslucentBackground);

    ui->lblFrameTop->setWindowFlags(Qt::FramelessWindowHint);
    ui->lblFrameTop->setAttribute(Qt::WA_TranslucentBackground);

    ui->lblFrameTopRight->setWindowFlags(Qt::FramelessWindowHint);
    ui->lblFrameTopRight->setAttribute(Qt::WA_TranslucentBackground);
    ui->lblFrameRight->setWindowFlags(Qt::FramelessWindowHint);
    ui->lblFrameRight->setAttribute(Qt::WA_TranslucentBackground);
    ui->lblFrameBottomRight->setWindowFlags(Qt::FramelessWindowHint);
    ui->lblFrameBottomRight->setAttribute(Qt::WA_TranslucentBackground);

    ui->lblFrameBottom->setWindowFlags(Qt::FramelessWindowHint);
    ui->lblFrameBottom->setAttribute(Qt::WA_TranslucentBackground);

    readCombatant(combatant);

    if(ui->listActions->count() > 0)
        setFixedHeight((ui->listActions->sizeHintForRow(0) * ui->listActions->count()) + (2 * ui->listActions->frameWidth()) + ui->lblFrameTop->height() + ui->lblFrameBottom->height());
}

CombatantRolloverFrame::~CombatantRolloverFrame()
{
    delete ui;
}

void CombatantRolloverFrame::readCombatant(BattleDialogModelCombatant* combatant)
{
    ui->listActions->clear();

    if(combatant->getCombatantType() != DMHelper::CombatantType_Monster)
        return;

    BattleDialogModelMonsterBase* monsterBase = dynamic_cast<BattleDialogModelMonsterBase*>(combatant);
    if(!monsterBase)
        return;

    MonsterClass* monsterClass = monsterBase->getMonsterClass();
    if(!monsterClass)
        return;

    addActionList(monsterClass->getActions(), QString("Actions"));
    addActionList(monsterClass->getLegendaryActions(), QString("Legendary Actions"));
    addActionList(monsterClass->getSpecialAbilities(), QString("Special Actions"));
    addActionList(monsterClass->getReactions(), QString("Reactions"));
}

void CombatantRolloverFrame::addActionList(const QList<MonsterAction>& actionList, const QString& listTitle)
{
    if(actionList.count() <= 0)
        return;

    QListWidgetItem* actionTitle = new QListWidgetItem(listTitle);
    QFont titleFont = actionTitle->font();
    titleFont.setWeight(QFont::Bold);
    if(titleFont.pointSize() > 0)
        titleFont.setPointSize(titleFont.pointSize() + 2);
    else
        titleFont.setPixelSize(titleFont.pixelSize() + 2);
    actionTitle->setFont(titleFont);
    ui->listActions->addItem(actionTitle);

    for(MonsterAction action : actionList)
    {
        ui->listActions->addItem(action.summaryString());
    }

    QListWidgetItem* separator = new QListWidgetItem();
    separator->setSizeHint(QSize(ui->listActions->width(), 5));
    separator->setFlags(Qt::NoItemFlags);
    ui->listActions->addItem(separator);
    QFrame* separatorFrame = new QFrame();
    separatorFrame->setFrameShape(QFrame::HLine);
    ui->listActions->setItemWidget(separator, separatorFrame);
}
