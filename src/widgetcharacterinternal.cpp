#include "widgetcharacterinternal.h"
#include "dmconstants.h"
#include "battledialogmodel.h"
#include "battledialogmodelcharacter.h"
#include "widgetcharacter.h"
#include <QLineEdit>
#include <QMouseEvent>
#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>

WidgetCharacterInternal::WidgetCharacterInternal(BattleDialogModelCharacter* character, WidgetCharacter *parent) :
    WidgetCombatantBase(parent),
    _widgetParent(parent),
    _character(character)
//    _mouseDown(Qt::NoButton),
//    _active(false),
//    _selected(false),
//    _lblIcon(0),
//    _lblInitName(0),
//    _edtInit(0),
//    _angle(0),
//    _timer(0)
{
//    setAutoFillBackground(true);
    if(_widgetParent)
        _widgetParent->setInternals(this);
}

BattleDialogModelCombatant* WidgetCharacterInternal::getCombatant()
{
    return _character;
}

QFrame* WidgetCharacterInternal::getFrame()
{
    return _widgetParent;
}

int WidgetCharacterInternal::getInitiative() const
{
    //return _edtInit ? _edtInit->text().toInt() : 0;
    return _character->getInitiative();
}

/*
bool WidgetCharacterInternal::isActive()
{
    return _active;
}

bool WidgetCharacterInternal::isSelected()
{
    return _selected;
}

bool WidgetCharacterInternal::isShown()
{
    return true;
}

bool WidgetCharacterInternal::isKnown()
{
    return true;
}
*/

void WidgetCharacterInternal::updateData()
{
    if(_widgetParent)
        _widgetParent->updateData();
    // TODO: load data from character
    /*
    updatePairData(pairName, _character->getName());
    updatePairData(pairArmorClass, QString::number(_character->getArmorClass()));

    frame->loadImage();

    update();

     */
}

void WidgetCharacterInternal::setInitiative(int initiative)
{
    // TODO: change widget and character initiative
    Q_UNUSED(initiative);

    if(_widgetParent)
        _widgetParent->updateData();

    /*
    if(_edtInit)
    {
        _edtInit->setText(QString::number(initiative));
        update();
    }
    */
}

void WidgetCharacterInternal::setHitPoints(int hp)
{
    Q_UNUSED(hp);

    if(_widgetParent)
        _widgetParent->updateData();
}

/*
void WidgetCharacterInternal::setActive(bool active)
{
    _active = active;
    setStyleSheet(getStyleString());
}

void WidgetCharacterInternal::setSelected(bool selected)
{
    _selected = selected;
    setHighlighted(_selected);
}
*/

void WidgetCharacterInternal::setHighlighted(bool highlighted)
{
    WidgetCombatantBase::setHighlighted(highlighted);

/*
    setWidgetHighlighted(_lblIcon, highlighted);
    setWidgetHighlighted(_lblInitName, highlighted);
    setWidgetHighlighted(_edtInit, highlighted);

    setPairHighlighted(pairName, highlighted);
    setPairHighlighted(pairArmorClass, highlighted);
*/
}

/*
void WidgetCharacterInternal::timerExpired()
{
    _angle += 20;
    setStyleSheet(getStyleString());
}
*/

/*
void WidgetCharacterInternal::leaveEvent(QEvent * event)
{
    Q_UNUSED(event);
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    _mouseDown = Qt::NoButton;
}

void WidgetCharacterInternal::mousePressEvent(QMouseEvent * event)
{
    Q_UNUSED(event);

    if(event->button() == Qt::LeftButton)
        setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _mouseDown = event->button();
}

void WidgetCharacterInternal::mouseReleaseEvent(QMouseEvent * event)
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

void WidgetCharacterInternal::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        if(rect().contains(event->pos()))
            executeDoubleClick();
    }
}
*/

void WidgetCharacterInternal::executeDoubleClick()
{
    // TODO: get this working
    /*
    if(_character->getCharacter())
        emit clicked(_character->getCharacter()->getID());
    else
        qDebug() << "[Character Widget] no valid character found!";
    */
}

/*
void WidgetCharacterInternal::loadImage()
{
    if((_lblIcon)&&(getCombatant()))
    {
        _lblIcon->resize(DMHelper::CHARACTER_ICON_WIDTH, DMHelper::CHARACTER_ICON_HEIGHT);
        _lblIcon->setPixmap(getCombatant()->getIconPixmap(DMHelper::PixmapSize_Thumb));
    }
}
*/
/*
QHBoxLayout* WidgetCharacterInternal::createPairLayout(const QString& pairName, const QString& pairValue)
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

void WidgetCharacterInternal::updatePairData(QHBoxLayout* pair, const QString& pairValue)
{
    if((!pair) || (pair->itemAt(1) == 0))
        return;

    QLabel* lbl = dynamic_cast<QLabel*>(pair->itemAt(1)->widget());
    if( lbl )
    {
        lbl->setText( pairValue );
    }
}

void WidgetCharacterInternal::setPairHighlighted(QHBoxLayout* pair, bool highlighted)
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

void WidgetCharacterInternal::setWidgetHighlighted(QWidget* widget, bool highlighted)
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
*/

/*
QString WidgetCharacterInternal::getStyleString()
{
    QString styleStr("WidgetCharacterInternal{");

    if(_active)
    {
        setLineWidth(7);
        styleStr.append("color: rgb(242, 236, 226); background-color: rgb(115, 18, 0); ");
    }
    else
    {
        setLineWidth(1);
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
*/
