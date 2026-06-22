#include "combatantwidget.h"
#include "dmconstants.h"
#include "battledialogmodel.h"
#include <QLineEdit>
#include <QMouseEvent>
#include <QLabel>
#include <QHBoxLayout>
#include <QStyle>

namespace
{
    // One-shot stylesheet driven by the dynamic "state" property. Set on each
    // CombatantWidget at construction time so subsequent state changes only
    // need to repolish the widget rather than re-parse and re-apply a fresh
    // style string. The empty-state selector keeps the default appearance.
    static const char* COMBATANT_STATE_STYLESHEET =
        "CombatantWidget { background-color: none; border-left: 4px solid transparent; padding-left: 4px; border-bottom: 1px solid rgba(0, 0, 0, 40); }"
        "CombatantWidget[state=\"hover\"] { background-color: rgb(64, 64, 64); border-left: 4px solid rgb(100, 100, 100); }"
        "CombatantWidget[state=\"active\"] { background-color: rgb(115, 18, 0); border-left: 4px solid rgb(200, 40, 0); }"
        "CombatantWidget[state=\"selected\"] { background-image: url(); background-color: rgb(196, 196, 196); border-left: 4px solid rgb(80, 80, 80); }";
}

CombatantWidget::CombatantWidget(QWidget *parent) :
    QFrame(parent),
    _mouseDown(Qt::NoButton),
    _active(false),
    _selected(false),
    _hover(false),
    _lblIcon(nullptr),
    _lblInitName(nullptr),
    _edtInit(nullptr)
{
    setAttribute(Qt::WA_Hover);
    setAutoFillBackground(true);
    // Install the state-aware stylesheet once. State changes flip a dynamic
    // property and re-polish the widget, which is dramatically cheaper than
    // setStyleSheet() (the latter triggers a full unpolish/polish across all
    // descendants every call).
    setStyleSheet(QString::fromLatin1(COMBATANT_STATE_STYLESHEET));
    refreshStateStyle();
}

void CombatantWidget::refreshStateStyle()
{
    QString state;
    if(_selected)
        state = QStringLiteral("selected");
    else if(_active)
        state = QStringLiteral("active");
    else if(_hover)
        state = QStringLiteral("hover");

    const QVariant existing = property("state");
    if(existing.toString() == state)
        return;

    setProperty("state", state);
    if(QStyle* s = style())
    {
        s->unpolish(this);
        s->polish(this);
    }
    update();
}

void CombatantWidget::installEventFilterRecursive(QObject* filterObj)
{
    if(!filterObj)
        return;

    installEventFilter(filterObj);
    const QObjectList& kids = children();
    for(QObject* child : kids)
    {
        if(QWidget* w = qobject_cast<QWidget*>(child))
        {
            w->installEventFilter(filterObj);
            // Also cover grandchildren (e.g. widgets inside sub-layouts)
            const QObjectList& grandkids = w->children();
            for(QObject* gk : grandkids)
            {
                if(QWidget* gkw = qobject_cast<QWidget*>(gk))
                    gkw->installEventFilter(filterObj);
            }
        }
    }
}

int CombatantWidget::getInitiative() const
{
    return _edtInit ? _edtInit->text().toInt() : 0;
}

bool CombatantWidget::isActive()
{
    return _active;
}

bool CombatantWidget::isSelected()
{
    return _selected;
}

bool CombatantWidget::isHover()
{
    return _hover;
}

bool CombatantWidget::isShown()
{
    return true;
}

bool CombatantWidget::isKnown()
{
    return true;
}

void CombatantWidget::updateData()
{
    return;
}

void CombatantWidget::updateMove()
{
    return;
}

void CombatantWidget::setInitiative(int initiative)
{
    if(_edtInit)
    {
        _edtInit->setText(QString::number(initiative));
        update();
    }
}

void CombatantWidget::initiativeChanged()
{
    if((_edtInit)&&(getCombatant()))
        getCombatant()->setInitiative(_edtInit->text().toInt());
}

void CombatantWidget::setActive(bool active)
{
    if(_active != active)
    {
        _active = active;
        refreshStateStyle();
    }
}

void CombatantWidget::setSelected(bool selected)
{
    if(_selected != selected)
    {
        _selected = selected;
        refreshStateStyle();
    }
}

void CombatantWidget::setHover(bool hover)
{
    if(_hover != hover)
    {
        _hover = hover;
        refreshStateStyle();
    }
}

void CombatantWidget::showEvent(QShowEvent* event)
{
    refreshStateStyle();
    QFrame::showEvent(event);
}

void CombatantWidget::enterEvent(QEnterEvent* event)
{
    Q_UNUSED(event);

    _hover = true;
    refreshStateStyle();
}

void CombatantWidget::leaveEvent(QEvent* event)
{
    Q_UNUSED(event);

    _mouseDown = Qt::NoButton;
    _hover = false;
    refreshStateStyle();
}

void CombatantWidget::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);

    _mouseDown = event->button();
}

void CombatantWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if(_mouseDown == event->button())
    {
        _mouseDown = Qt::NoButton;
    }
}

void CombatantWidget::loadImage()
{
    if((_lblIcon)&&(getCombatant()))
    {
        _lblIcon->resize(DMHelper::CHARACTER_ICON_WIDTH, DMHelper::CHARACTER_ICON_HEIGHT);
        _lblIcon->setPixmap(getCombatant()->getIconPixmap(DMHelper::PixmapSize_Thumb));
        emit imageChanged(getCombatant());
    }
}

QHBoxLayout* CombatantWidget::createPairLayout(const QString& pairName, const QString& pairValue)
{
    QHBoxLayout* pairLayout = new QHBoxLayout();

    QLabel* nameLabel = new QLabel(pairName + QString(":"), this);
    nameLabel->resize(nameLabel->fontMetrics().boundingRect(nameLabel->text()).width(), nameLabel->height());
    nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QLabel* valueLabel = new QLabel(pairValue, this);
    valueLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    pairLayout->addWidget(nameLabel);
    pairLayout->addWidget(valueLabel);

    return pairLayout;
}

void CombatantWidget::updatePairData(QHBoxLayout* pair, const QString& pairValue)
{
    if((!pair) || (pair->itemAt(1) == nullptr))
        return;

    QLabel* lbl = dynamic_cast<QLabel*>(pair->itemAt(1)->widget());
    if(lbl)
        lbl->setText(pairValue);
}

QString CombatantWidget::getStyleString()
{
    const QString common = QStringLiteral("padding-left: 4px; border-bottom: 1px solid rgba(0, 0, 0, 40); ");

    if(_selected)
        return QStringLiteral("CombatantWidget{ background-image: url(); background-color: rgb(196, 196, 196); border-left: 4px solid rgb(80, 80, 80); ") + common + QStringLiteral("}");
    else if(_active)
        return QStringLiteral("CombatantWidget{ background-color: rgb(115, 18, 0); border-left: 4px solid rgb(200, 40, 0); ") + common + QStringLiteral("}");
    else if(_hover)
        return QStringLiteral("CombatantWidget{ background-color: rgb(64, 64, 64); border-left: 4px solid rgb(100, 100, 100); ") + common + QStringLiteral("}");
    else
        return QStringLiteral("CombatantWidget{ background-color: none; border-left: 4px solid transparent; ") + common + QStringLiteral("}");
}
