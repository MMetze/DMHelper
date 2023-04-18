#include "combatantwidget.h"
#include "dmconstants.h"
#include "battledialogmodel.h"
#include <QLineEdit>
#include <QMouseEvent>
#include <QLabel>
#include <QHBoxLayout>

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
    {
        getCombatant()->setInitiative(_edtInit->text().toInt());
    }
}

void CombatantWidget::setActive(bool active)
{
    if(_active != active)
    {
        _active = active;
        setStyleSheet(getStyleString());
    }
}

void CombatantWidget::setSelected(bool selected)
{
    if(_selected != selected)
    {
        _selected = selected;
        setStyleSheet(getStyleString());
    }
}

void CombatantWidget::setHover(bool hover)
{
    if(_hover != hover)
    {
        _hover = hover;
        setStyleSheet(getStyleString());
    }
}

void CombatantWidget::showEvent(QShowEvent *event)
{
    setStyleSheet(getStyleString());
    QFrame::showEvent(event);
}

void CombatantWidget::enterEvent(QEvent * event)
{
    Q_UNUSED(event);

    _hover = true;
    setStyleSheet(getStyleString());
}

void CombatantWidget::leaveEvent(QEvent * event)
{
    Q_UNUSED(event);

    setFrameStyle(QFrame::Panel | QFrame::Raised);
    _mouseDown = Qt::NoButton;
    _hover = false;
    setStyleSheet(getStyleString());
}

void CombatantWidget::mousePressEvent(QMouseEvent * event)
{
    Q_UNUSED(event);

    if(event->button() == Qt::LeftButton)
        setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _mouseDown = event->button();
}

void CombatantWidget::mouseReleaseEvent(QMouseEvent * event)
{
    if(_mouseDown == event->button())
    {
        if(event->button() == Qt::LeftButton)
        {
            setFrameStyle(QFrame::Panel | QFrame::Raised);
        }
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
    setLineWidth(5);

    if(_selected)
        return QString("CombatantWidget{ background-image: url(); background-color: rgb(196, 196, 196); }");
    else if(_active)
        return QString("CombatantWidget{ background-color: rgb(115, 18, 0); }");
    else if(_hover)
        return QString("CombatantWidget{ background-color: rgb(64, 64, 64); }");
    else
        return QString("CombatantWidget{ background-color: none; }");
}
