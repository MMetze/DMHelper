#include "battledialoggraphicsview.h"
#include <QKeyEvent>
#include <QPaintEvent>
#include <QElapsedTimer>
#include <QTimer>
#include <QDebug>
#include <QEvent>
#include <QShowEvent>

// File-scope statics for the post-paint idle probe (shared between paintEvent and the
// 0-delay lambda — avoids an extern on a function-local static).
static QElapsedTimer s_postPaintProbeTimer;
static int s_postPaintProbeCount = 0;

BattleDialogGraphicsView::BattleDialogGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
}

BattleDialogGraphicsView::~BattleDialogGraphicsView()
{
}

void BattleDialogGraphicsView::paintEvent(QPaintEvent *event)
{
    static int s_paintCount = 0;
    static QElapsedTimer s_paintInterval;
    static bool s_paintFirst = true;
    qint64 intervalUs = 0;
    if(!s_paintFirst)
        intervalUs = s_paintInterval.nsecsElapsed() / 1000;
    s_paintInterval.restart();
    s_paintFirst = false;

    QElapsedTimer t;
    t.start();
    QGraphicsView::paintEvent(event);
    const qint64 paintUs = t.nsecsElapsed() / 1000;
    ++s_paintCount;
    qDebug() << "[FoW-perf] BattleView paintEvent #" << s_paintCount
             << " interval:" << intervalUs << "us"
             << " paint:" << paintUs << "us"
             << " updateMode:" << viewportUpdateMode()
             << " region:" << event->region().boundingRect();

    // Probe: measure how long the event loop stays blocked after paintEvent returns.
    // A QTimer::singleShot(0) fires on the next event-loop iteration; if the reported
    // delay is >> 1ms, something is running on the main thread between this return and idle.
    s_postPaintProbeTimer.restart();
    s_postPaintProbeCount = s_paintCount;
    QTimer::singleShot(0, this, []() {
        qDebug() << "[FoW-perf] BattleView post-paint idle #" << s_postPaintProbeCount
                 << " delay:" << s_postPaintProbeTimer.nsecsElapsed() / 1000 << "us";
    });
}

void BattleDialogGraphicsView::showEvent(QShowEvent *event)
{
    QGraphicsView::showEvent(event);
    if(viewport())
        viewport()->installEventFilter(this);
}

bool BattleDialogGraphicsView::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == viewport() && event->type() == QEvent::UpdateRequest)
    {
        static int s_updateReqCount = 0;
        static QElapsedTimer s_updateReqInterval;
        static bool s_updateReqFirst = true;
        qint64 intervalUs = 0;
        if(!s_updateReqFirst)
            intervalUs = s_updateReqInterval.nsecsElapsed() / 1000;
        s_updateReqInterval.restart();
        s_updateReqFirst = false;
        ++s_updateReqCount;
        qDebug() << "[FoW-perf] Viewport UpdateRequest #" << s_updateReqCount
                 << " interval:" << intervalUs << "us";
    }
    return QGraphicsView::eventFilter(watched, event);
}

void BattleDialogGraphicsView::keyPressEvent(QKeyEvent *event)
{
    if(event->modifiers() == Qt::AltModifier)
        event->ignore();
    else
        QGraphicsView::keyPressEvent(event);
}

