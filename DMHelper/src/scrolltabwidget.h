#ifndef SCROLLTABWIDGET_H
#define SCROLLTABWIDGET_H

#include <QWidget>

class QLabel;
class QFrame;
class ScrollTabTransitionBase;

class ScrollTabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ScrollTabWidget(QWidget* page, QSizeF parentRatio = QSizeF(0,0), QWidget *parent = nullptr);
    virtual ~ScrollTabWidget();

    virtual int getTabOffset() const;

    virtual QSize sizeHint() const;

signals:
    void tabResized(ScrollTabWidget* tab);

public slots:
    virtual void activateTab(bool activated);
    virtual void setTabOffset(int offset);
    virtual void setParentRatio(const QSizeF& ratio);
    virtual void setAnimatedTransitions(bool animated);

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void showEvent(QShowEvent *event);
    virtual void paintEvent(QPaintEvent *event);
    virtual bool eventFilter(QObject *watched, QEvent *event);

protected slots:
    virtual void transitionComplete();

private:
    bool validRatio(const QSizeF& ratio) const;

    QWidget* _page;
    QLabel* _scrollLabel;
    QFrame* _borderTop;
    QFrame* _borderBottom;
    QFrame* _handleFrame;
    int _offset;
    QSizeF _parentRatio;
    ScrollTabTransitionBase* _transition;
    bool _animated;
};

#endif // SCROLLTABWIDGET_H
