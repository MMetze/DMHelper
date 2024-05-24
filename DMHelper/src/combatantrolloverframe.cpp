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

const int ROLLOVER_LISTITEM_TITLE = Qt::UserRole + 1;
const int ROLLOVER_LISTITEM_DESCRIPTION = Qt::UserRole + 2;

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

    ui->listActions->setWordWrap(true);
    ui->listActions->setTextElideMode(Qt::ElideNone);

    if(ui->listActions->count() > 0)
        setFixedHeight((ui->listActions->sizeHintForRow(0) * ui->listActions->count()) + (2 * ui->listActions->frameWidth()) + ui->lblFrameTop->height() + ui->lblFrameBottom->height());

    connect(ui->listActions, &QListWidget::itemClicked, this, &CombatantRolloverFrame::handleItemClicked);
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

void CombatantRolloverFrame::handleItemClicked(QListWidgetItem *item)
{
    if((!item) || (item->data(ROLLOVER_LISTITEM_DESCRIPTION).isNull()))
        return;

    // Expand the item to show the full description
    if(item->text() != item->data(ROLLOVER_LISTITEM_TITLE).toString())
        item->setText(item->data(ROLLOVER_LISTITEM_TITLE).toString());
    else
        item->setText(item->data(ROLLOVER_LISTITEM_TITLE).toString() + QChar::LineFeed + item->data(ROLLOVER_LISTITEM_DESCRIPTION).toString());
    update();
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

    const QList<MonsterAction>& actionList = characterBase->getActions();

    addSectionTitle(QString("Attacks"));
    for(const MonsterAction& action : std::as_const(actionList))
    {
        if(action.hasDiceSummary())
        {
            QListWidgetItem *item = new QListWidgetItem(action.summaryString());
            item->setData(ROLLOVER_LISTITEM_TITLE, action.summaryString());
            item->setData(ROLLOVER_LISTITEM_DESCRIPTION, action.getDescription());
            ui->listActions->addItem(item);
        }
    }

    addSeparator();

    addSectionTitle(QString("Abilities"));

    for(const MonsterAction& action : std::as_const(actionList))
    {
        if(!action.hasDiceSummary())
        {
            QListWidgetItem *item = new QListWidgetItem(action.summaryString());
            item->setData(ROLLOVER_LISTITEM_TITLE, action.summaryString());
            item->setData(ROLLOVER_LISTITEM_DESCRIPTION, action.getDescription());
            ui->listActions->addItem(item);
        }
    }
}

void CombatantRolloverFrame::readMonster(BattleDialogModelMonsterBase* monster)
{
    if(!monster)
        return;

    MonsterClass* monsterClass = monster->getMonsterClass();
    if(!monsterClass)
        return;

    addActionList(monsterClass->getActions(), QString("Actions"));
    addSeparator();
    addActionList(monsterClass->getLegendaryActions(), QString("Legendary Actions"));
    addSeparator();
    addActionList(monsterClass->getSpecialAbilities(), QString("Special Actions"));
    addSeparator();
    addActionList(monsterClass->getReactions(), QString("Reactions"));
}

void CombatantRolloverFrame::addActionList(const QList<MonsterAction>& actionList, const QString& listTitle)
{
    if(actionList.count() <= 0)
        return;

    addSectionTitle(listTitle);

    for(const MonsterAction& action : std::as_const(actionList))
    {
        QListWidgetItem *item = new QListWidgetItem(action.summaryString());
        item->setData(ROLLOVER_LISTITEM_TITLE, action.summaryString());
        if(!action.getDescription().isEmpty())
            item->setData(ROLLOVER_LISTITEM_DESCRIPTION, action.getDescription());
        else
            item->setFlags(Qt::NoItemFlags);

        ui->listActions->addItem(item);
    }
}

void CombatantRolloverFrame::addSeparator()
{
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
    actionTitle->setFlags(Qt::NoItemFlags);
    QFont titleFont = actionTitle->font();
    titleFont.setWeight(QFont::Bold);
    if(titleFont.pointSize() > 0)
        titleFont.setPointSize(titleFont.pointSize() + 2);
    else
        titleFont.setPixelSize(titleFont.pixelSize() + 2);
    actionTitle->setFont(titleFont);
    ui->listActions->addItem(actionTitle);

}
