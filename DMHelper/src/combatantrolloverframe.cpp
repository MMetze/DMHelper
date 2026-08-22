#include "combatantrolloverframe.h"
#include "battledialogmodelcombatant.h"
#include "battledialogmodelcharacter.h"
#include "battledialogmodelmonsterbase.h"
#include "characterv2.h"
#include "monsterclassv2.h"
#include "monsteraction.h"
#include "monsterfactory.h"
#include "combatantfactory.h"
#include "combatantwidget.h"
#include "ui_combatantrolloverframe.h"
#include <QApplication>
#include <QAbstractScrollArea>
#include <QMouseEvent>
#include <QRegularExpression>
#include <QTextDocument>
#include <QStringList>

const int ROLLOVER_LISTITEM_TITLE = Qt::UserRole + 1;
const int ROLLOVER_LISTITEM_DESCRIPTION = Qt::UserRole + 2;
constexpr int LIST_CONTENT_PADDING = 100;

namespace
{
    static constexpr int TITLE_EXTRA_HEIGHT = 6;
    static constexpr int DESCRIPTION_EXTRA_HEIGHT = 8;
    static constexpr int SECTION_TITLE_EXTRA_HEIGHT = 6;
    static constexpr int MIN_TITLE_HEIGHT = 20;
    static constexpr int MIN_DESCRIPTION_HEIGHT = 32;

    QString extractDescription(const QHash<QString, QVariant>& hashEntry)
    {
        const QString shortDescription = hashEntry.value(QStringLiteral("desc")).toString().trimmed();
        if(!shortDescription.isEmpty())
            return shortDescription;

        const QString longDescription = hashEntry.value(QStringLiteral("description")).toString().trimmed();
        return longDescription;
    }
}

namespace
{
    int textHeightForItem(const QListWidgetItem* item, int textWidth, int minimumHeight, int extraHeight)
    {
        if(!item)
            return 0;

        QTextDocument doc;
        doc.setDefaultFont(item->font());
        doc.setPlainText(item->text());
        doc.setTextWidth(textWidth);

        return qMax(minimumHeight, static_cast<int>(doc.size().height()) + extraHeight);
    }

    int separatorHeightForItem(const QListWidgetItem* item)
    {
        return item ? qMax(0, item->sizeHint().height()) : 0;
    }
}

CombatantRolloverFrame::CombatantRolloverFrame(CombatantWidget* combatantWidget, QWidget *parent, bool isDetailFrame) :
    QFrame(parent),
    ui(new Ui::CombatantRolloverFrame),
    _widget(combatantWidget),
    _closeTimer(0),
    _isDetailFrame(isDetailFrame),
    _detailRollover(nullptr)
{
    ui->setupUi(this);

    setWindowFlags(_isDetailFrame ? (Qt::FramelessWindowHint | Qt::Tool) : Qt::FramelessWindowHint);
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

    if((_widget) && (!_isDetailFrame))
        readCombatant(_widget->getCombatant());

    ui->listActions->setWordWrap(true);
    ui->listActions->setTextElideMode(Qt::ElideNone);

    if(_isDetailFrame)
    {
        //ui->frameInfo->setStyleSheet(QStringLiteral("background-color: transparent;"));
        //ui->frameInfoContents->setStyleSheet(QStringLiteral("background-color: transparent;"));
        ui->listActions->setStyleSheet(QStringLiteral("background-color: transparent;"));
    }
    else
    {
        qApp->installEventFilter(this);
    }

    updateFrameHeight();

    connect(ui->listActions, &QListWidget::itemClicked, this, &CombatantRolloverFrame::handleItemClicked);
}

CombatantRolloverFrame::~CombatantRolloverFrame()
{
    if(!_isDetailFrame)
        qApp->removeEventFilter(this);

    if(_closeTimer > 0)
        killTimer(_closeTimer);

    hideDetailPopup();

    delete ui;
}

bool CombatantRolloverFrame::isEmpty() const
{
    return ui->listActions->count() == 0;
}

void CombatantRolloverFrame::triggerClose()
{
    // Closure is handled only by explicit interaction outside the rollover.
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
    if(_isDetailFrame)
        return;

    if((!item) || (item->data(ROLLOVER_LISTITEM_DESCRIPTION).isNull()))
        return;

    const QString itemTitle = item->data(ROLLOVER_LISTITEM_TITLE).toString();
    QString description = item->data(ROLLOVER_LISTITEM_DESCRIPTION).toString();

    // Heuristic: only treat as HTML if string starts with '<' and ends with
    // '>', or contains well-known tags.
    static const QRegularExpression htmlTagRegex("<\\s*\\w+.*?>");
    if(htmlTagRegex.match(description).hasMatch())
    {
        QTextDocument doc;
        doc.setHtml(description);
        description = doc.toPlainText();
    }

    if(description.isEmpty())
        return;

    showDetailPopup(item, itemTitle, description);
}

void CombatantRolloverFrame::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
}

void CombatantRolloverFrame::timerEvent(QTimerEvent *event)
{
    if((_closeTimer != 0) && (_closeTimer == event->timerId()))
        cancelClose();
}

bool CombatantRolloverFrame::eventFilter(QObject* watched, QEvent* event)
{
    if((_isDetailFrame) || (!isVisible()) || (!event))
        return QFrame::eventFilter(watched, event);

    if(event->type() != QEvent::MouseButtonPress)
        return QFrame::eventFilter(watched, event);

    QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
    if(!mouseEvent)
        return QFrame::eventFilter(watched, event);

    QWidget* watchedWidget = dynamic_cast<QWidget*>(watched);
    const QPoint globalPos = mouseEvent->globalPosition().toPoint();
    auto isPointInsideFrame = [globalPos](QWidget* frame) -> bool
    {
        if((!frame) || (!frame->isVisible()))
            return false;

        const QPoint localPos = frame->mapFromGlobal(globalPos);
        return frame->rect().contains(localPos);
    };

    const bool fromMainFrame = (watchedWidget) && ((watchedWidget == this) || isAncestorOf(watchedWidget));
    const bool fromDetailFrame = (watchedWidget) && (_detailRollover) && (_detailRollover->isVisible())
        && ((watchedWidget == _detailRollover) || (_detailRollover->isAncestorOf(watchedWidget)));
    const bool inMainFrame = fromMainFrame || isPointInsideFrame(this);
    const bool inDetailFrame = fromDetailFrame || isPointInsideFrame(_detailRollover);

    if((!inMainFrame) && (!inDetailFrame))
    {
        hideDetailPopup();
        emit hoverEnded();
    }

    return QFrame::eventFilter(watched, event);
}

void CombatantRolloverFrame::setDetailContent(const QString& title, const QString& description)
{
    _detailTitle = title;
    _detailDescription = description;

    ui->listActions->clear();

    if(!title.isEmpty())
        addSectionTitle(title);

    if(!description.isEmpty())
    {
        QListWidgetItem* descriptionItem = new QListWidgetItem(description);
        descriptionItem->setFlags(Qt::ItemIsEnabled);
        ui->listActions->addItem(descriptionItem);
    }

    updateFrameHeight();
}

void CombatantRolloverFrame::updateFrameHeight()
{
    if(ui->listActions->count() <= 0)
        return;

    ui->listActions->doItemsLayout();

    const int listWidth = qMax(ui->listActions->sizeHintForColumn(0), ui->listActions->width());
    const int contentWidth = qMax(100, listWidth - LIST_CONTENT_PADDING);

    int rowsHeight = 0;
    for(int row = 0; row < ui->listActions->count(); ++row)
    {
        QListWidgetItem* item = ui->listActions->item(row);
        if(!item)
            continue;

        if(item->flags() == Qt::NoItemFlags)
            rowsHeight += separatorHeightForItem(item);
        else if(item->data(ROLLOVER_LISTITEM_DESCRIPTION).isValid())
            rowsHeight += textHeightForItem(item, contentWidth, MIN_DESCRIPTION_HEIGHT, DESCRIPTION_EXTRA_HEIGHT);
        else if(item->data(ROLLOVER_LISTITEM_TITLE).isValid())
            rowsHeight += textHeightForItem(item, contentWidth, MIN_TITLE_HEIGHT, TITLE_EXTRA_HEIGHT);
        else
            rowsHeight += textHeightForItem(item, contentWidth, MIN_TITLE_HEIGHT, SECTION_TITLE_EXTRA_HEIGHT);
    }

    const int listHeight = rowsHeight + (2 * ui->listActions->frameWidth());
    ui->listActions->setMinimumHeight(listHeight);
    ui->listActions->setMaximumHeight(listHeight);

    setFixedHeight(listHeight
                   + ui->lblFrameTop->height()
                   + ui->lblFrameBottom->height());
}

void CombatantRolloverFrame::showDetailPopup(QListWidgetItem* item, const QString& title, const QString& description)
{
    if((!item) || (title.isEmpty()) || (description.isEmpty()))
        return;

    if(!_detailRollover)
    {
        _detailRollover = new CombatantRolloverFrame(_widget, this, true);
        _detailRollover->setPalette(palette());
        _detailRollover->setFont(font());
        if(!styleSheet().isEmpty())
            _detailRollover->setStyleSheet(styleSheet());
    }

    _detailRollover->setDetailContent(title, description);

    static constexpr int POPUP_SPACING = 12;
//    static constexpr int POPUP_CONTENT_PADDING = 24;

    const QRect itemRect = ui->listActions->visualItemRect(item);
    const QPoint itemTopRight = ui->listActions->viewport()->mapToGlobal(itemRect.topRight());
    const QPoint itemTopLeft = ui->listActions->viewport()->mapToGlobal(itemRect.topLeft());

    QRect mainWindowRect;
    if((_widget) && (_widget->window()))
        mainWindowRect = _widget->window()->frameGeometry();
    else if(QApplication::activeWindow())
        mainWindowRect = QApplication::activeWindow()->frameGeometry();
    else
        mainWindowRect = QRect(itemTopRight.x() - 600, itemTopRight.y() - 350, 1200, 700);

    const int popupWidth = qMax(_detailRollover->width(), _detailRollover->sizeHint().width());

    const int availableRight = mainWindowRect.right() - (itemTopRight.x() + POPUP_SPACING);
    const int availableLeft = (itemTopLeft.x() - POPUP_SPACING) - mainWindowRect.left();
    const bool canShowOnRight = (availableRight >= popupWidth);
    const bool canShowOnLeft = (availableLeft >= popupWidth);
    const bool showOnRight = canShowOnRight || ((!canShowOnLeft) && (availableRight >= availableLeft));

    QListWidget* detailList = _detailRollover->ui->listActions;
    const int textWidth = qMax(100, popupWidth - LIST_CONTENT_PADDING);
    if(detailList->count() > 0)
    {
        QListWidgetItem* titleItem = detailList->item(0);
        if(titleItem)
        {
            QTextDocument doc;
            doc.setDefaultFont(titleItem->font());
            doc.setPlainText(titleItem->text());
            doc.setTextWidth(textWidth);
            titleItem->setSizeHint(QSize(textWidth, qMax(20, static_cast<int>(doc.size().height()) + 6)));
        }
    }
    if(detailList->count() > 1)
    {
        QListWidgetItem* descriptionItem = detailList->item(1);
        if(descriptionItem)
        {
            QTextDocument doc;
            doc.setDefaultFont(descriptionItem->font());
            doc.setPlainText(descriptionItem->text());
            doc.setTextWidth(textWidth);
            descriptionItem->setSizeHint(QSize(textWidth, qMax(32, static_cast<int>(doc.size().height()) + 8)));
        }
    }

    _detailRollover->updateFrameHeight();

    QPoint popupPos = showOnRight
        ? QPoint(itemTopRight.x() + POPUP_SPACING, itemTopRight.y())
        : QPoint(itemTopLeft.x() - POPUP_SPACING - _detailRollover->width(), itemTopLeft.y());

    if(popupPos.y() + _detailRollover->height() > mainWindowRect.bottom())
        popupPos.setY(mainWindowRect.bottom() - _detailRollover->height());
    if(popupPos.y() < mainWindowRect.top())
        popupPos.setY(mainWindowRect.top());

    _detailRollover->move(popupPos);
    _detailRollover->show();
    _detailRollover->raise();
}

void CombatantRolloverFrame::hideDetailPopup()
{
    if(!_detailRollover)
        return;

    _detailRollover->hide();
    delete _detailRollover;
    _detailRollover = nullptr;
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

    if(!CombatantFactory::Instance()->hasElementList("actions"))
        return;

    Characterv2* characterBase = character->getCharacter();
    if(!characterBase)
        return;    

    QList<QVariant> listValue = characterBase->getListValue("actions");
    if(listValue.isEmpty())
        return;

    addSectionTitle(QString("Actions"));

    for(const auto &listEntry : std::as_const(listValue))
    {
        QHash<QString, QVariant> hashEntry = listEntry.toHash();
        if((hashEntry.isEmpty()) || (!hashEntry.contains("name")))
            continue;

        QString summaryString = MonsterAction::createSummaryString(hashEntry);
        QListWidgetItem *item = new QListWidgetItem(summaryString);
        item->setData(ROLLOVER_LISTITEM_TITLE, summaryString);
        const QString description = extractDescription(hashEntry);
        if(!description.isEmpty())
            item->setData(ROLLOVER_LISTITEM_DESCRIPTION, description);
        else
            item->setFlags(Qt::ItemIsEnabled);

        ui->listActions->addItem(item);
    }
}

void CombatantRolloverFrame::readMonster(BattleDialogModelMonsterBase* monster)
{
    if(!monster)
        return;

    MonsterClassv2* monsterClass = monster->getMonsterClass();
    if(!monsterClass)
        return;

    QList<QString> elementNames = MonsterFactory::Instance()->getElementLists().keys();
    for(const QString& elementName : elementNames)
    {
        if(!monsterClass->hasValue(elementName))
            continue;

        QList<QVariant> listValue = monsterClass->getListValue(elementName);
        if(listValue.isEmpty())
            continue;

        if(ui->listActions->count() > 0)
            addSeparator();

        addSectionTitle(elementName);

        for(const auto &listEntry : std::as_const(listValue))
        {
            QHash<QString, QVariant> hashEntry = listEntry.toHash();
            if((hashEntry.isEmpty()) || (!hashEntry.contains("name")))
                continue;

            QString summaryString = MonsterAction::createSummaryString(hashEntry);
            QListWidgetItem *item = new QListWidgetItem(summaryString);
            item->setData(ROLLOVER_LISTITEM_TITLE, summaryString);
            const QString description = extractDescription(hashEntry);
            if(!description.isEmpty())
                item->setData(ROLLOVER_LISTITEM_DESCRIPTION, description);
            else
                item->setFlags(Qt::ItemIsEnabled);

            ui->listActions->addItem(item);
        }
    }
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
            item->setFlags(Qt::ItemIsEnabled);

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

    QStringList words = sectionTitle.simplified().split(' ', Qt::SkipEmptyParts);
    for(QString& word : words)
    {
        if(word.isEmpty())
            continue;

        word[0] = word.at(0).toUpper();
        if(word.length() > 1)
            word = word.left(1) + word.mid(1).toLower();
    }

    const QString displayTitle = words.join(' ');

    QListWidgetItem* actionTitle = new QListWidgetItem(displayTitle);
    actionTitle->setFlags(Qt::ItemIsEnabled);
    QFont titleFont = actionTitle->font();
    titleFont.setWeight(QFont::Bold);
    if(titleFont.pointSize() > 0)
        titleFont.setPointSize(titleFont.pointSize() + 2);
    else
        titleFont.setPixelSize(titleFont.pixelSize() + 2);
    actionTitle->setFont(titleFont);
    ui->listActions->addItem(actionTitle);

}
