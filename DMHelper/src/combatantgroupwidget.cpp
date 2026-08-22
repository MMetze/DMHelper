#include "combatantgroupwidget.h"
#include "ui_combatantgroupwidget.h"
#include "battledialogmodelcombatantgroup.h"
#include "battledialogmodelcombatant.h"
#include "combatantwidget.h"
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QStyle>
#include <QStyleOption>
#include <QToolButton>
#include <QCoreApplication>

CombatantGroupWidget::CombatantGroupWidget(BattleDialogModelCombatantGroup* group, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CombatantGroupWidget),
    _group(group),
    _memberWidgets(),
    _updatingCheckboxes(false)
{
    ui->setupUi(this);

    if(_group)
    {
        ui->edtGroupName->setText(_group->getName());
        ui->edtGroupInit->setText(QString::number(_group->getInitiative()));
        setCollapsed(_group->isCollapsed());
    }

    connect(ui->btnCollapse, &QToolButton::toggled, this, &CombatantGroupWidget::handleCollapseToggled);
    connect(ui->edtGroupName, &QLineEdit::textChanged, this, &CombatantGroupWidget::handleNameChanged);
    connect(ui->edtGroupInit, &QLineEdit::textChanged, this, &CombatantGroupWidget::handleInitiativeChanged);
    connect(ui->chkVisible, &QCheckBox::clicked, this, &CombatantGroupWidget::handleVisibleClicked);
    connect(ui->chkKnown, &QCheckBox::clicked, this, &CombatantGroupWidget::handleKnownClicked);

    // Active-group cue is painted by overpainting btnCollapse via eventFilter
    // (see eventFilter / setActive). Stylesheets on a child of the parchment
    // parent flip the chrome to QStyleSheetStyle and bleed defaults; palette
    // tweaks are ignored by styles that draw their own button frames.
    ui->btnCollapse->installEventFilter(this);
}

CombatantGroupWidget::~CombatantGroupWidget()
{
    delete ui;
}

BattleDialogModelCombatantGroup* CombatantGroupWidget::getGroup() const
{
    return _group;
}

QUuid CombatantGroupWidget::getGroupId() const
{
    return _group ? _group->getId() : QUuid();
}

void CombatantGroupWidget::addMemberWidget(CombatantWidget* widget)
{
    if(!widget || _memberWidgets.contains(widget))
        return;

    _memberWidgets.append(widget);
    ui->contentLayout->addWidget(widget);

    connect(widget, &CombatantWidget::isShownChanged, this, &CombatantGroupWidget::handleMemberVisibilityChanged);
    connect(widget, &CombatantWidget::isKnownChanged, this, &CombatantGroupWidget::handleMemberVisibilityChanged);

    updateMasterCheckboxes();
}

void CombatantGroupWidget::removeMemberWidget(CombatantWidget* widget)
{
    if(!widget)
        return;

    disconnect(widget, &CombatantWidget::isShownChanged, this, &CombatantGroupWidget::handleMemberVisibilityChanged);
    disconnect(widget, &CombatantWidget::isKnownChanged, this, &CombatantGroupWidget::handleMemberVisibilityChanged);

    _memberWidgets.removeAll(widget);
    ui->contentLayout->removeWidget(widget);

    updateMasterCheckboxes();
}

QList<CombatantWidget*> CombatantGroupWidget::getMemberWidgets() const
{
    QList<CombatantWidget*> members;
    for(const QPointer<CombatantWidget>& member : _memberWidgets)
    {
        if(member)
            members.append(member.data());
    }
    return members;
}

void CombatantGroupWidget::setCollapsed(bool collapsed)
{
    ui->contentWidget->setVisible(!collapsed);
    ui->btnCollapse->setArrowType(collapsed ? Qt::RightArrow : Qt::DownArrow);

    // Update the button's checked state without triggering the slot
    bool wasBlocked = ui->btnCollapse->blockSignals(true);
    ui->btnCollapse->setChecked(collapsed);
    ui->btnCollapse->blockSignals(wasBlocked);

    if(_group)
        _group->setCollapsed(collapsed);

    update(); // repaint connector line
}

bool CombatantGroupWidget::isCollapsed() const
{
    return _group ? _group->isCollapsed() : false;
}

void CombatantGroupWidget::setActive(bool active)
{
    if(_active == active)
        return;

    _active = active;

    if(ui && ui->btnCollapse)
        ui->btnCollapse->update();

    update();
}

void CombatantGroupWidget::updateMasterCheckboxes()
{
    _updatingCheckboxes = true;

    int shownCount = 0;
    int knownCount = 0;
    int total = 0;

    // Drop any deleted members and only aggregate live widgets.
    for(int i = _memberWidgets.count() - 1; i >= 0; --i)
    {
        if(!_memberWidgets[i])
            _memberWidgets.removeAt(i);
    }

    for(const QPointer<CombatantWidget>& member : _memberWidgets)
    {
        CombatantWidget* widget = member.data();
        if(!widget)
            continue;

        ++total;
        if(widget->isShown())
            ++shownCount;
        if(widget->isKnown())
            ++knownCount;
    }

    if(total > 0)
    {
        ui->chkVisible->setChecked(shownCount > total / 2);
        ui->chkKnown->setChecked(knownCount > total / 2);
    }

    _updatingCheckboxes = false;
}

void CombatantGroupWidget::contextMenuEvent(QContextMenuEvent* event)
{
    if(_group)
        emit contextMenu(_group, event->globalPos());
}

bool CombatantGroupWidget::eventFilter(QObject* watched, QEvent* event)
{
    // When this group is the active initiative holder, overpaint btnCollapse
    // with a solid red rect and a white arrow AFTER the style has finished
    // drawing the button. Doing this in the parent's paintEvent doesn't work
    // (children paint after the parent) and stylesheet/palette approaches are
    // either bled by the parchment theme or ignored by the active style.
    if((_active) && (ui) && (watched == ui->btnCollapse) && (event) && (event->type() == QEvent::Paint))
    {
        // Let the button paint itself first so any focus/press feedback is
        // preserved underneath, then overpaint our active cue on top.
        ui->btnCollapse->removeEventFilter(this);
        QCoreApplication::sendEvent(ui->btnCollapse, event);
        ui->btnCollapse->installEventFilter(this);

        static const QColor ACTIVE_BUTTON_COLOR(200, 40, 40);
        QPainter painter(ui->btnCollapse);
        painter.fillRect(ui->btnCollapse->rect(), ACTIVE_BUTTON_COLOR);

        // Re-render the style's own arrow primitive on top, but with a palette
        // forced to white text so the existing arrow shape matches what the
        // user is used to seeing — just tinted for the active state.
        QStyleOption arrowOpt;
        arrowOpt.initFrom(ui->btnCollapse);
        arrowOpt.rect = ui->btnCollapse->rect();
        arrowOpt.palette.setColor(QPalette::ButtonText, Qt::white);
        arrowOpt.palette.setColor(QPalette::WindowText, Qt::white);
        arrowOpt.palette.setColor(QPalette::Text, Qt::white);

        QStyle::PrimitiveElement pe = QStyle::PE_IndicatorArrowRight;
        switch(ui->btnCollapse->arrowType())
        {
            case Qt::DownArrow:  pe = QStyle::PE_IndicatorArrowDown;  break;
            case Qt::UpArrow:    pe = QStyle::PE_IndicatorArrowUp;    break;
            case Qt::LeftArrow:  pe = QStyle::PE_IndicatorArrowLeft;  break;
            case Qt::RightArrow:
            case Qt::NoArrow:
            default:             pe = QStyle::PE_IndicatorArrowRight; break;
        }
        ui->btnCollapse->style()->drawPrimitive(pe, &arrowOpt, &painter, ui->btnCollapse);
        return true;
    }

    return QFrame::eventFilter(watched, event);
}

void CombatantGroupWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if(event && event->button() == Qt::LeftButton)
    {
        emit clicked(this);
        event->accept();
        return;
    }

    QFrame::mouseReleaseEvent(event);
}

void CombatantGroupWidget::paintEvent(QPaintEvent* event)
{
    static const QColor ACTIVE_LINE_COLOR(200, 40, 40);         // bold red for connectors
    static const QColor INACTIVE_LINE_COLOR(140, 140, 140);     // existing grey
    static const int    ACTIVE_LINE_WIDTH = 2;
    static const int    INACTIVE_LINE_WIDTH = 1;

    QFrame::paintEvent(event);

    // Only draw the connector when expanded and there are members
    if(isCollapsed() || _memberWidgets.isEmpty() || !ui->contentWidget->isVisible())
        return;

    QPainter painter(this);
    QPen pen(_active ? ACTIVE_LINE_COLOR : INACTIVE_LINE_COLOR);
    pen.setWidth(_active ? ACTIVE_LINE_WIDTH : INACTIVE_LINE_WIDTH);
    pen.setStyle(Qt::SolidLine);
    painter.setPen(pen);

    // Vertical line x: centered under the collapse button
    int lineX = ui->btnCollapse->x() + ui->btnCollapse->width() / 2;

    // Start just below the header row (bottom of the collapse button)
    int lineTop = ui->btnCollapse->mapTo(this, QPoint(0, ui->btnCollapse->height())).y() + 2;

    // Find the last visible member widget for the line endpoint
    CombatantWidget* lastVisible = nullptr;
    for(int i = _memberWidgets.count() - 1; i >= 0; --i)
    {
        CombatantWidget* member = _memberWidgets[i].data();
        if((member) && (member->isVisible()))
        {
            lastVisible = member;
            break;
        }
    }

    if(!lastVisible)
        return;

    // End at the vertical center of the last visible member
    int lineBottom = lastVisible->mapTo(this, QPoint(0, lastVisible->height() / 2)).y();

    // Draw the vertical trunk line
    painter.drawLine(lineX, lineTop, lineX, lineBottom);

    // Draw horizontal ticks to each visible member
    int tickRight = ui->contentWidget->mapTo(this, QPoint(0, 0)).x() + ui->contentLayout->contentsMargins().left() - 2;
    for(const QPointer<CombatantWidget>& memberPtr : _memberWidgets)
    {
        CombatantWidget* member = memberPtr.data();
        if(!member)
            continue;

        if(!member->isVisible())
            continue;

        int memberY = member->mapTo(this, QPoint(0, member->height() / 2)).y();
        painter.drawLine(lineX, memberY, tickRight, memberY);
    }
}

void CombatantGroupWidget::handleCollapseToggled(bool checked)
{
    setCollapsed(checked);
}

void CombatantGroupWidget::handleNameChanged(const QString& name)
{
    if(_group)
        _group->setName(name);
}

void CombatantGroupWidget::handleInitiativeChanged(const QString& text)
{
    if(!_group)
        return;

    bool ok;
    int initiative = text.toInt(&ok);
    if(ok)
        _group->setInitiative(initiative);
}

void CombatantGroupWidget::handleVisibleClicked(bool checked)
{
    if(_updatingCheckboxes)
        return;

    for(const QPointer<CombatantWidget>& member : _memberWidgets)
    {
        CombatantWidget* widget = member.data();
        if(!widget)
            continue;

        BattleDialogModelCombatant* combatant = widget->getCombatant();
        if(combatant)
            combatant->setShown(checked);
    }
}

void CombatantGroupWidget::handleKnownClicked(bool checked)
{
    if(_updatingCheckboxes)
        return;

    for(const QPointer<CombatantWidget>& member : _memberWidgets)
    {
        CombatantWidget* widget = member.data();
        if(!widget)
            continue;

        BattleDialogModelCombatant* combatant = widget->getCombatant();
        if(combatant)
            combatant->setKnown(checked);
    }
}

void CombatantGroupWidget::handleMemberVisibilityChanged()
{
    if(!_updatingCheckboxes)
        updateMasterCheckboxes();
}
