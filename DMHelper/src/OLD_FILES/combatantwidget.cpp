#include "combatantwidget.h"
#include "dmconstants.h"
#include "battledialogmodel.h"
#include <QLineEdit>
#include <QMouseEvent>
#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>

CombatantWidget::CombatantWidget(QWidget *parent) :
    QFrame(parent),
    _mouseDown(Qt::NoButton),
    _active(false),
    _selected(false),
    _lblIcon(nullptr),
    _lblInitName(nullptr),
    _edtInit(nullptr),
    _angle(0),
    _timer(nullptr)
{
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
    _active = active;
    setStyleSheet(getStyleString());
}

void CombatantWidget::setSelected(bool selected)
{
    _selected = selected;
    setHighlighted(_selected);
}

void CombatantWidget::setHighlighted(bool highlighted)
{
    /*
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
    */

    setStyleSheet(getStyleString());

    setWidgetHighlighted(_lblIcon, highlighted);
    setWidgetHighlighted(_lblInitName, highlighted);
    setWidgetHighlighted(_edtInit, highlighted);
}

void CombatantWidget::timerExpired()
{
    _angle += 20;
    setStyleSheet(getStyleString());
}

void CombatantWidget::leaveEvent(QEvent * event)
{
    Q_UNUSED(event);
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    _mouseDown = Qt::NoButton;
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

void CombatantWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        if(rect().contains(event->pos()))
            executeDoubleClick();
    }
}

void CombatantWidget::executeDoubleClick()
{
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
    nameLabel->resize( nameLabel->fontMetrics().width(nameLabel->text()), nameLabel->height());
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
    if( lbl )
    {
        lbl->setText( pairValue );
    }
}

void CombatantWidget::setPairHighlighted(QHBoxLayout* pair, bool highlighted)
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

void CombatantWidget::setWidgetHighlighted(QWidget* widget, bool highlighted)
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

QString CombatantWidget::getStyleString()
{
    QString styleStr("CombatantWidget{");

    if(_active)
    {
        setLineWidth(2); // 7
        styleStr.append("color: rgb(242, 236, 226); background-color: rgb(115, 18, 0); ");
    }
    else
    {
        setLineWidth(2); // 1
    }

    if(_selected)
    {
        styleStr.append("background-color: rgb(115, 18, 0); background: qconicalgradient(cx:0.5, cy:0.5, angle: ");
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
