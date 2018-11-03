#include "scrolloverlaywidget.h"
#include "scrolltabwidget.h"
#include <QPushButton>
#include <QVBoxLayout>

ScrollOverlayWidget::ScrollOverlayWidget(QWidget *parent) :
    QWidget(parent),
    _tabList(),
    _mainLayout(0),
    _widgetOffset()
{
    _mainLayout = new QVBoxLayout(this);
    _mainLayout->addStretch();
    setLayout(_mainLayout);
}

ScrollOverlayWidget::~ScrollOverlayWidget()
{
}

void ScrollOverlayWidget::repositionTab(ScrollTabWidget* tab)
{
    if(!tab)
        return;

    QPushButton* i = _tabList.key(tab);
    if(!i)
        return;

    tab->setTabOffset(i->y());

    int xp = _widgetOffset.x() - tab->width() + i->x() + 1;
    int yp = _widgetOffset.y();
    tab->move(xp,yp);
}

void ScrollOverlayWidget::resizeTabs()
{
    for(auto i : _tabList.keys())
    {
        ScrollTabWidget* tab = _tabList.value(i);
        if(tab)
        {
            QWidget* tabParent = tab->parentWidget();
            if(tabParent)
            {
                // TODO: Add resizing to also increase the size of tabs if interesting
                // TODO: possibly make tabs manually resizeable by dragging on the scroll itself
                QSize tabSize = tab->sizeHint();
                if(tabSize.width() > tabParent->width())
                    tabSize.setWidth(tabParent->width());

                if(tabSize.height() > tabParent->height())
                    tabSize.setHeight(tabParent->height());

                if(tabSize != tab->size())
                    tab->resize(tabSize);
            }
        }
    }

    // Find the offset from mutual parents
    QWidget* p = this;
    QWidget* tabParent = _tabList.first() ? _tabList.first()->parentWidget() : 0;
    _widgetOffset = QPoint();
    while((p != 0) && (p != tabParent))
    {
        _widgetOffset += p->pos();
        p = p->parentWidget();
    }

    repositionAllTabs();
}

void ScrollOverlayWidget::setAnimatedTransitions(bool animated)
{
    for(auto i : _tabList.keys())
    {
        ScrollTabWidget* tab = _tabList.value(i);
        if(tab)
        {
            tab->setAnimatedTransitions(animated);
        }
    }
}

int ScrollOverlayWidget::addTab(ScrollTabWidget* tab, const QIcon &icon)
{
    if(!tab)
        return -1;

    QPushButton* button = new QPushButton(icon, QString(), this);
    button->setFlat(true);
    button->setCheckable(true);
    button->setStyleSheet(QString("QPushButton:checked {background-color: rgb(159,109,58);}"));
    button->resize(30,30);
    button->setMinimumSize(30,30);
    button->setMaximumSize(30,30);
    button->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

    connect(button,SIGNAL(toggled(bool)),tab,SLOT(activateTab(bool)));
    connect(tab,SIGNAL(tabResized(ScrollTabWidget*)),this,SLOT(repositionTab(ScrollTabWidget*)));

    _tabList.insert(button, tab);
    _mainLayout->insertWidget(_mainLayout->count() - 1, button);

    tab->hide();

    return _tabList.count();
}

void ScrollOverlayWidget::moveEvent(QMoveEvent *event)
{
    Q_UNUSED(event);

    repositionAllTabs();
}

void ScrollOverlayWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
}

void ScrollOverlayWidget::repositionAllTabs()
{
    if(_tabList.count() <= 0)
        return;

    for(auto i : _tabList.keys())
    {
        repositionTab(_tabList.value(i));
    }
}
