#include "combatantrolloverframe.h"
#include "battledialogmodelcombatant.h"
#include "battledialogmodelcharacter.h"
#include "battledialogmodelmonsterbase.h"
#include "character.h"
#include "monsterclass.h"
#include "monsteraction.h"
#include "combatantwidget.h"
#include "ui_combatantrolloverframe.h"
#include <QStringList>

CombatantRolloverFrame::CombatantRolloverFrame(CombatantWidget* combatantWidget, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CombatantRolloverFrame),
    _widget(combatantWidget),
    _closeTimer(0)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_Hover, true);

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

    if(_widget)
        readCombatant(_widget->getCombatant());

    if(ui->listActions->count() > 0)
        setFixedHeight((ui->listActions->sizeHintForRow(0) * ui->listActions->count()) + (2 * ui->listActions->frameWidth()) + ui->lblFrameTop->height() + ui->lblFrameBottom->height());
}

CombatantRolloverFrame::~CombatantRolloverFrame()
{
    if(_closeTimer > 0)
        killTimer(_closeTimer);

    delete ui;
}

void CombatantRolloverFrame::triggerClose()
{
    if(_closeTimer > 0)
        return;

    // This is needed to ensure all mouse events are handled
    _closeTimer = startTimer(0);
}

void CombatantRolloverFrame::cancelClose()
{
    if(_closeTimer == 0)
        return;

    killTimer(_closeTimer);
    _closeTimer = 0;
}

void CombatantRolloverFrame::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);

    if(!_widget)
        return;

    triggerClose();
}

void CombatantRolloverFrame::timerEvent(QTimerEvent *event)
{
    if(_closeTimer != event->timerId())
        return;

    cancelClose();

    if((_widget) && (!_widget->underMouse()) && (!underMouse()))
        emit hoverEnded();
}

void CombatantRolloverFrame::readCombatant(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return;

    ui->listActions->clear();

    switch(combatant->getCombatantType())
    {
        case DMHelper::CombatantType_Character:
            readCharacter(dynamic_cast<BattleDialogModelCharacter*>(combatant));
            break;
        case DMHelper::CombatantType_Monster:
            readMonster(dynamic_cast<BattleDialogModelMonsterBase*>(combatant));
            break;
        default:
            return;
    }
}

void CombatantRolloverFrame::readCharacter(BattleDialogModelCharacter* character)
{
    if(!character)
        return;

    Character* characterBase = character->getCharacter();
    if(!characterBase)
        return;

    addSectionTitle(QString("Actions"));
    const QList<MonsterAction>& actionList = characterBase->getActions();
    for(const MonsterAction& action : qAsConst(actionList))
    {
        QListWidgetItem *item = new QListWidgetItem(action.getName());
        item->setToolTip(action.getDescription());
        ui->listActions->addItem(item);
    }

    /*
    addActionList(characterBase->getActions(), QString("Actions"));

    QString proficiencyString = characterBase->getStringValue(Character::StringValue_proficiencies);
    if(!proficiencyString.isEmpty())
    {
        addSectionTitle(QString("Proficiencies"));
        QStringList proficiencyList = proficiencyString.split(QChar::LineFeed);
        for(const QString& oneItem : qAsConst(proficiencyList))
        {
            ui->listActions->addItem(oneItem);
        }
    }
    */
}

void CombatantRolloverFrame::readMonster(BattleDialogModelMonsterBase* monster)
{
    if(!monster)
        return;

    MonsterClass* monsterClass = monster->getMonsterClass();
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

    addSectionTitle(listTitle);

    for(const MonsterAction& action : qAsConst(actionList))
    {
        QListWidgetItem *item = new QListWidgetItem(action.summaryString());
        item->setToolTip(action.getDescription());
        ui->listActions->addItem(item);
//        ui->listActions->addItem(action.summaryString());
    }

    QListWidgetItem* separator = new QListWidgetItem();
    separator->setSizeHint(QSize(ui->listActions->width(), 5));
    separator->setFlags(Qt::NoItemFlags);
    ui->listActions->addItem(separator);
    QFrame* separatorFrame = new QFrame();
    separatorFrame->setFrameShape(QFrame::HLine);
    ui->listActions->setItemWidget(separator, separatorFrame);
}

void CombatantRolloverFrame::addSectionTitle(const QString& sectionTitle)
{
    if(sectionTitle.isEmpty())
        return;

    QListWidgetItem* actionTitle = new QListWidgetItem(sectionTitle);
    QFont titleFont = actionTitle->font();
    titleFont.setWeight(QFont::Bold);
    if(titleFont.pointSize() > 0)
        titleFont.setPointSize(titleFont.pointSize() + 2);
    else
        titleFont.setPixelSize(titleFont.pixelSize() + 2);
    actionTitle->setFont(titleFont);
    ui->listActions->addItem(actionTitle);

}
