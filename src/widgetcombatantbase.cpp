#include "widgetcombatantbase.h"
#include "dmconstants.h"
#include "battledialogmodel.h"
#include <QLineEdit>
#include <QMouseEvent>
#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>

WidgetCombatantBase::WidgetCombatantBase(QWidget *parent) :
    QObject(parent),
    _mouseDown(Qt::NoButton),
    _active(false),
    _selected(false),
//    _lblIcon(0),
//    _lblInitName(0),
//    _edtInit(0),
    _angle(0),
    _timer(0)
{
    //setAutoFillBackground(true);
}

int WidgetCombatantBase::getInitiative() const
{
    //return _edtInit ? _edtInit->text().toInt() : 0;
    return 0;
}

bool WidgetCombatantBase::isActive()
{
    return _active;
}

bool WidgetCombatantBase::isSelected()
{
    return _selected;
}

bool WidgetCombatantBase::isShown()
{
    return true;
}

bool WidgetCombatantBase::isKnown()
{
    return true;
}

void WidgetCombatantBase::leaveEvent(QEvent * event)
{
    Q_UNUSED(event);
    getFrame()->setFrameStyle(QFrame::Panel | QFrame::Raised);
    _mouseDown = Qt::NoButton;
}

void WidgetCombatantBase::mousePressEvent(QMouseEvent * event)
{
    Q_UNUSED(event);

    if(event->button() == Qt::LeftButton)
        getFrame()->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _mouseDown = event->button();
}

void WidgetCombatantBase::mouseReleaseEvent(QMouseEvent * event)
{
    if(_mouseDown == event->button())
    {
        if(event->button() == Qt::LeftButton)
        {
            getFrame()->setFrameStyle(QFrame::Panel | QFrame::Raised);
        }
        else if(event->button() == Qt::RightButton)
        {
            if(getCombatant())
            {
                emit contextMenu(getCombatant(), event->globalPos());
            }
        }
        _mouseDown = Qt::NoButton;
    }
}

void WidgetCombatantBase::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        if(getFrame()->rect().contains(event->pos()))
            executeDoubleClick();
    }
}

void WidgetCombatantBase::updateData()
{
}

    /*
void WidgetCombatantBase::setInitiative(int initiative)
{
    Q_UNUSED(initiative);
    if(_edtInit)
    {
        _edtInit->setText(QString::number(initiative));
        update();
    }
}
    */

void WidgetCombatantBase::initiativeChanged(int initiative)
{
    if(getCombatant() && (getCombatant()->getInitiative() != initiative))
    {
        getCombatant()->setInitiative(initiative);
        updateData();
    }
}

void WidgetCombatantBase::handleHitPointsChanged(int hp)
{
    if(getCombatant() && (getCombatant()->getHitPoints() != hp))
    {
        getCombatant()->setHitPoints(hp);
        updateData();
        emit hitPointsChanged();
    }
}

void WidgetCombatantBase::setActive(bool active)
{
    _active = active;
    getFrame()->setStyleSheet(getStyleString());
}

void WidgetCombatantBase::setSelected(bool selected)
{
    _selected = selected;
    setHighlighted(_selected);
}

void WidgetCombatantBase::setHighlighted(bool highlighted)
{
    if(highlighted)
    {
        _angle = 0;
        if(!_timer)
        {
            _timer = new QTimer(this);
            connect(_timer,SIGNAL(timeout()),this,SLOT(timerExpired()));
        }
        _timer->start(50);
    }
    else
    {
        if(_timer)
        {
            _timer->stop();
        }
    }

    getFrame()->setStyleSheet(getStyleString());

//    setWidgetHighlighted(_lblIcon, highlighted);
//    setWidgetHighlighted(_lblInitName, highlighted);
//    setWidgetHighlighted(_edtInit, highlighted);
}

void WidgetCombatantBase::timerExpired()
{
    _angle += 20;
    getFrame()->setStyleSheet(getStyleString());
}

void WidgetCombatantBase::executeDoubleClick()
{
}

/*
void WidgetCombatantBase::loadImage()
{
    if((_lblIcon)&&(getCombatant()))
    {
        _lblIcon->resize(DMHelper::CHARACTER_ICON_WIDTH, DMHelper::CHARACTER_ICON_HEIGHT);
        _lblIcon->setPixmap(getCombatant()->getIconPixmap(DMHelper::PixmapSize_Thumb));
    }
}
*/

/*
QHBoxLayout* WidgetCombatantBase::createPairLayout(const QString& pairName, const QString& pairValue)
{
    QHBoxLayout* pairLayout = new QHBoxLayout();

    QLabel* nameLabel = new QLabel(pairName + QString(":"), getFrame());
    nameLabel->resize( nameLabel->fontMetrics().width(nameLabel->text()), nameLabel->height());
    nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QLabel* valueLabel = new QLabel(pairValue, getFrame());
    valueLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    pairLayout->addWidget(nameLabel);
    pairLayout->addWidget(valueLabel);

    return pairLayout;
}

void WidgetCombatantBase::updatePairData(QHBoxLayout* pair, const QString& pairValue)
{
    if((!pair) || (pair->itemAt(1) == 0))
        return;

    QLabel* lbl = dynamic_cast<QLabel*>(pair->itemAt(1)->widget());
    if( lbl )
    {
        lbl->setText( pairValue );
    }
}

void WidgetCombatantBase::setPairHighlighted(QHBoxLayout* pair, bool highlighted)
{
    if(!pair)
        return;

    for (int i = 0; i < pair->count(); ++i)
    {
        if(pair->itemAt(i))
        {
            setWidgetHighlighted(pair->itemAt(i)->widget(), highlighted);
        }
    }
}
*/

void WidgetCombatantBase::setWidgetHighlighted(QWidget* widget, bool highlighted)
{
    if(widget)
    {
        if(highlighted)
        {
            widget->setStyleSheet("background-image: url(); background-color: rgba(0,0,0,0);");
        }
        else
        {
            widget->setStyleSheet("");
        }
    }
}

QString WidgetCombatantBase::getStyleString()
{
    QString styleStr("WidgetCombatantBase{");

    if(_active)
    {
        getFrame()->setLineWidth(2); // 7
        styleStr.append("color: rgb(242, 236, 226); background-color: rgb(115, 18, 0); ");
    }
    else
    {
        getFrame()->setLineWidth(2); // 1
    }

    if(_selected)
    {
        styleStr.append("background: qconicalgradient(cx:0.5, cy:0.5, angle: ");
        styleStr.append(QString::number(_angle));
        styleStr.append(", stop:0   rgba(255, 255, 255, 192), \
                   stop:0.1 rgba(186, 231, 241, 128), \
                   stop:0.2 rgba(255, 255, 255, 0), \
                   stop:0.3 rgba(255, 255, 255, 0), \
                   stop:0.4 rgba(186, 231, 241, 128), \
                   stop:0.5 rgba(255, 255, 255, 192), \
                   stop:0.6 rgba(186, 231, 241, 128), \
                   stop:0.7 rgba(255, 255, 255, 0), \
                   stop:0.8 rgba(255, 255, 255, 0), \
                   stop:0.9 rgba(186, 231, 241, 128), \
                   stop:1   rgba(255, 255, 255, 192));");
    }

    styleStr.append("}");

    return styleStr;
}
