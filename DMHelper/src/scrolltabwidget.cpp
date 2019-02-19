#include "scrolltabwidget.h"
#include "scrolltabtransitionbase.h"
#include "scrolltabtransitionfactory.h"
#include <QFrame>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QResizeEvent>
#include <QShowEvent>
#include <QtGlobal>
#include <QPixmap>

// TODO: handle pages of all shapes and sizes with buttons in different positions!!

const int SCROLL_BORDER_HEIGHT = 5;
const int SHADOW_WIDTH = 6;
const int FRAME_WIDTH = 20;
const int FRAME_HEIGHT = 30;

// #define SCROLL_PAGE_MULTIPLIED
#ifdef SCROLL_PAGE_MULTIPLIED
    const double SCROLL_X_FACTOR = 0.9;
    const double SCROLL_Y_FACTOR = 0.75;
#else
    const int SCROLL_GAP_SIZE = 10;
    const int SCROLL_GAP_SIZE_DOUBLE = SCROLL_GAP_SIZE * 2;
#endif

ScrollTabWidget::ScrollTabWidget(QWidget* page, QSizeF parentRatio, QWidget *parent) :
    QWidget(parent),
    _page(page),
    _scrollLabel(0),
    _borderTop(0),
    _borderBottom(0),
    _handleFrame(0),
    _offset(0),
    _parentRatio(0,0),
    _transition(0),
    _animated(false)
{
    if(!_page)
        return;

    setFocusPolicy(Qt::ClickFocus);
    setFocusProxy(_page);
    setStyleSheet(QString("ScrollTabWidget{background-color: rgba(0,0,0,0);}"));

    if(validRatio(parentRatio))
        _parentRatio = parentRatio;

    _handleFrame = new QFrame(this);
    _handleFrame->setStyleSheet(QString("background-color: rgb(200,200,200);"));

    _scrollLabel = new QLabel(this);
    _page->setParent(_scrollLabel);
    QGraphicsDropShadowEffect* myShadow = new QGraphicsDropShadowEffect();
    myShadow->setBlurRadius(5.0);
    myShadow->setOffset(SHADOW_WIDTH);
    _scrollLabel->setGraphicsEffect(myShadow);

    //_scrollLabel->setStyleSheet("background-image: url(:/img/data/parchmentdark.jpg);");
    _scrollLabel->setStyleSheet("background-image: url(:/img/data/parchment.jpg);");
    _scrollLabel->setFrameStyle(QFrame::Box);
    _borderTop = new QFrame(this);
    _borderTop ->setStyleSheet(QString("background-color: rgb(200,200,200);"));
    _borderBottom = new QFrame(this);
    _borderBottom ->setStyleSheet(QString("background-color: rgb(200,200,200);"));
}

ScrollTabWidget::~ScrollTabWidget()
{
    if(_transition && _transition->isStarted())
    {
        _transition->stop();
        delete _transition;
    }
}

int ScrollTabWidget::getTabOffset() const
{
    return _offset;
}

QSize ScrollTabWidget::sizeHint() const
{
    if(!_page)
        return QSize(100,100);

    if(validRatio(_parentRatio) && parentWidget())
    {
        return QSize((qreal)(parentWidget()->width()) * _parentRatio.width(), (qreal)(parentWidget()->height()) * _parentRatio.height());
    }
    else
    {
        QSize tabSize = _page->sizeHint();
        /*
#ifdef SCROLL_PAGE_MULTIPLIED
        tabSize.setWidth((qreal)tabSize.width() / SCROLL_X_FACTOR);
        tabSize.setHeight((qreal)tabSize.height() / SCROLL_Y_FACTOR);
#else
        tabSize.setWidth(tabSize.width() - SCROLL_GAP_SIZE_DOUBLE - SHADOW_WIDTH);
        tabSize.setHeight(tabSize.height() - SCROLL_GAP_SIZE_DOUBLE - SHADOW_WIDTH);
#endif
*/
        return tabSize;
    }
}

void ScrollTabWidget::activateTab(bool activated)
{
    if(activated)
    {
        if(_animated)
            _transition = ScrollTabTransitionFactory::createTransition(this);
        else
            _transition = ScrollTabTransitionFactory::createInstantTransition(this);

        connect(_transition, SIGNAL(completed()), this, SLOT(transitionComplete()));
        _transition->start();

        show();
        _page->hide();
        _handleFrame->hide();
        _scrollLabel->hide();
        _borderTop->hide();
        _borderBottom->hide();
    }
    else
    {
        delete _transition;
        _transition = 0;
        hide();
    }
}

void ScrollTabWidget::setTabOffset(int offset)
{
    if(_offset != offset)
    {
        _offset = offset;
        if(_handleFrame)
        {
            _handleFrame->move(width()-_handleFrame->width(), getTabOffset());
        }
    }
}

void ScrollTabWidget::setParentRatio(const QSizeF& ratio)
{
    if(validRatio(ratio))
        _parentRatio = ratio;
    else
        _parentRatio = QSizeF(0,0);
}

void ScrollTabWidget::setAnimatedTransitions(bool animated)
{
    _animated = animated;
}

void ScrollTabWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if(_page)
    {
        qreal w, h;
        QSize pageSize;
        if(validRatio(_parentRatio) && parentWidget())
        {
            w = (qreal)width() - FRAME_WIDTH;
            h = (qreal)height();
#ifdef SCROLL_PAGE_MULTIPLIED
            pageSize = QSize(w * SCROLL_X_FACTOR, h * SCROLL_Y_FACTOR);
#else
            pageSize = QSize(w - SCROLL_GAP_SIZE_DOUBLE - SHADOW_WIDTH, h - SCROLL_GAP_SIZE_DOUBLE - SHADOW_WIDTH);
#endif
        }
        else
        {
            pageSize = _page->sizeHint();
#ifdef SCROLL_PAGE_MULTIPLIED
            if(pageSize.width() > (width() * SCROLL_X_FACTOR))
                pageSize.setWidth(width() * SCROLL_X_FACTOR);
            if(pageSize.height() > (height() * SCROLL_Y_FACTOR))
                pageSize.setHeight(height() * SCROLL_Y_FACTOR);

            w = (qreal)pageSize.width()/SCROLL_X_FACTOR;
            h = (qreal)pageSize.height()/SCROLL_Y_FACTOR;
#else
            /*
            if(pageSize.width() > (width() - SCROLL_GAP_SIZE_DOUBLE - SHADOW_WIDTH))
                pageSize.setWidth(width() - SCROLL_GAP_SIZE_DOUBLE - SHADOW_WIDTH);
            if(pageSize.height() > (height() - SCROLL_GAP_SIZE_DOUBLE - SHADOW_WIDTH))
                pageSize.setHeight(height() - SCROLL_GAP_SIZE_DOUBLE - SHADOW_WIDTH);
*/
            w = (qreal)pageSize.width() + SCROLL_GAP_SIZE_DOUBLE + SHADOW_WIDTH;
            h = (qreal)pageSize.height() + SCROLL_GAP_SIZE_DOUBLE + SHADOW_WIDTH;
#endif
            resize(w + FRAME_WIDTH, h);
        }

        if(pageSize != _page->size())
            _page->resize(pageSize);

        _scrollLabel->resize(w - SHADOW_WIDTH, h - SHADOW_WIDTH);
        _borderTop->resize(_scrollLabel->width(), SCROLL_BORDER_HEIGHT);
        _borderBottom->resize(_scrollLabel->width(), SCROLL_BORDER_HEIGHT);
        _borderBottom->move(0,_scrollLabel->height() - SCROLL_BORDER_HEIGHT);
#ifdef SCROLL_PAGE_MULTIPLIED
        _page->move((_scrollLabel->width()-_page->width())/2, (_scrollLabel->height()-_page->width())/2);
#else
        _page->move(SCROLL_GAP_SIZE, SCROLL_GAP_SIZE);
#endif

        _handleFrame->resize(FRAME_WIDTH + (w * 0.1), FRAME_HEIGHT);
        _handleFrame->move(width()-_handleFrame->width(), getTabOffset());

        emit tabResized(this);
    }
}

void ScrollTabWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    if(isVisible())
    {
        setFocus(Qt::PopupFocusReason);
    }
    else
    {
        clearFocus();
    }
}

void ScrollTabWidget::paintEvent(QPaintEvent *event)
{
    if((_transition)&&(_transition->isStarted()))
    {
        QPainter painter(this);
        _transition->paintTransition(painter);
    }

    QWidget::paintEvent(event);
}

bool ScrollTabWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(_transition && event && (event->type() == QEvent::Paint))
        return true; // Eat the paint events while the transition is ongoing

    return QWidget::eventFilter(watched, event);
}

void ScrollTabWidget::transitionComplete()
{
    _transition = nullptr;
    _page->show();
    _handleFrame->show();
    _scrollLabel->show();
    _borderTop->show();
    _borderBottom->show();
    setFocus(Qt::PopupFocusReason);
}

bool ScrollTabWidget::validRatio(const QSizeF& ratio) const
{
    return((ratio.width() > 0.0) && (ratio.width() <= 1.0) && (ratio.height() > 0.0) && (ratio.height() <= 1.0));
}
