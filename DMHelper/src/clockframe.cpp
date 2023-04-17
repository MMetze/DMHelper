#include "clockframe.h"
#include "ui_clockframe.h"
#include <QResizeEvent>
#include <QVector3D>
#include <QtMath>

ClockFrame::ClockFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::ClockFrame),
    _astrolobe(),
    _smallHand(),
    _largeHand(),
    _scaleFactor(-1),
    _smallHandScaled(),
    _largeHandScaled(),
    _smallHandAngle(-30.f),
    _largeHandAngle(30.f),
    _largeHandAngleTarget(30.f),
    _smallHandActive(true),
    _timerId(0),
    _mouseDown(MouseDownValue_up),
    _lastMousePos(0, 0)
{
    _astrolobe.load(":/img/data/astrolobe.png");        // size is 370x370
    _smallHand.load(":/img/data/clockhands-small.png"); // 50, 50 is center, size is 242x100
    _smallHandScaled = _smallHand;
    _largeHand.load(":/img/data/clockhands-large.png"); // 50, 50 is center, size is 311x100
    _largeHandScaled = _largeHand;
    ui->setupUi(this);
    ui->lblHandLarge->setAttribute(Qt::WA_TranslucentBackground);
    ui->lblHandSmall->setAttribute(Qt::WA_TranslucentBackground);
}

ClockFrame::~ClockFrame()
{
    if(_timerId > 0)
        killTimer(_timerId);

    delete ui;
}

float ClockFrame::getSmallHandAngle() const
{
    return _smallHandAngle;
}

float ClockFrame::getLargeHandAngle() const
{
    return _largeHandAngle;
}

bool ClockFrame::isSmallHandActive() const
{
    return _smallHandActive;
}

void ClockFrame::setSmallHandAngle(float angle)
{
    if(_smallHandAngle != angle)
    {
        _smallHandAngle = angle;
        positionClockHands();
        emit handsChanged();
    }
}

void ClockFrame::setLargeHandAngle(float angle)
{
    if(_largeHandAngle != angle)
    {
        _largeHandAngle = angle;
        positionClockHands();
        emit handsChanged();
    }
}

void ClockFrame::setLargeHandAngleTarget(float target)
{
    if((target < 0.f) || (target > 360.f))
        return;

    if((_mouseDown == MouseDownValue_up)&&(_largeHandAngle != target))
    {
        _largeHandAngleTarget = target;
        if(_timerId == 0)
            _timerId = startTimer(30);
    }
}

void ClockFrame::setSmallHandActive(bool active)
{
    _smallHandActive = active;
    if(_mouseDown == MouseDownValue_small)
    {
        _mouseDown = MouseDownValue_up;
    }
}

void ClockFrame::resizeEvent(QResizeEvent *event)
{
    int smallerDimension = qMin(event->size().width(), event->size().height());

    ui->lblAstrolobe->resize(smallerDimension, smallerDimension);
    ui->lblAstrolobe->setPixmap(QPixmap::fromImage(_astrolobe.scaled(smallerDimension, smallerDimension, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    ui->lblAstrolobe->move((width()/2) - (smallerDimension/2), (height()/2) - (smallerDimension/2));

    scaleClockFrame(smallerDimension);

    QFrame::resizeEvent(event);
}

void ClockFrame::mouseMoveEvent(QMouseEvent *event)
{
    if(_mouseDown != MouseDownValue_up)
    {
        QPoint midPoint(ui->lblAstrolobe->x() + (ui->lblAstrolobe->width()/2), ui->lblAstrolobe->y() + (ui->lblAstrolobe->height()/2));
        QVector2D newPos = QVector2D(event->pos() - midPoint).normalized();
        float dp = QVector2D::dotProduct(newPos, _lastMousePos);
        QVector3D cpv = QVector3D::normal(newPos.toVector3D(), _lastMousePos.toVector3D());
        float cp = cpv.z();
        float angle = -cp * 180.f * acos(dp) / 3.1415;
        if(_mouseDown == MouseDownValue_small)
        {
            _smallHandAngle += angle;
            if(_smallHandAngle > 360.f)
            {
                _smallHandAngle -= 360.f;
            }
            if(_smallHandAngle < 0.f)
            {
                _smallHandAngle += 360.f;
            }
        }
        else
        {
            _largeHandAngle += angle;
            if(_largeHandAngle > 360.f)
            {
                _largeHandAngle -= 360.f;
            }
            if(_largeHandAngle < 0.f)
            {
                _largeHandAngle += 360.f;
            }
        }
        positionClockHands();
        _lastMousePos = newPos;
        emit handsChanged();
    }
    QFrame::mouseMoveEvent(event);
}

void ClockFrame::mousePressEvent(QMouseEvent *event)
{
    QPoint midPoint(ui->lblAstrolobe->x() + (ui->lblAstrolobe->width()/2), ui->lblAstrolobe->y() + (ui->lblAstrolobe->height()/2));
    QVector2D vectorFromCenter = QVector2D(event->pos() - midPoint);
    _lastMousePos = vectorFromCenter.normalized();
    if((_smallHandActive) && (vectorFromCenter.lengthSquared() < qPow((float)qMin(ui->lblAstrolobe->width(), ui->lblAstrolobe->height())/6.f, 2.f) * 2.f ))
    {
        _mouseDown = MouseDownValue_small;
    }
    else
    {
        _mouseDown = MouseDownValue_large;
    }
    QFrame::mousePressEvent(event);
}

void ClockFrame::mouseReleaseEvent(QMouseEvent *event)
{
    _mouseDown = MouseDownValue_up;
    QFrame::mousePressEvent(event);
}

void ClockFrame::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    if(_mouseDown != MouseDownValue_up)
    {
        killTimer(_timerId);
        _timerId = 0;
    }

    float delta = _largeHandAngleTarget - _largeHandAngle;
    if( ((delta <= 20.f) && (delta >= 0.f)) || (delta <= -340.f))
    {
        _largeHandAngle = _largeHandAngleTarget;
        killTimer(_timerId);
        _timerId = 0;
    }
    else
    {
        _largeHandAngle += 20.f;
        if(_largeHandAngle >= 360.f)
            _largeHandAngle -= 360.f;
    }

    positionClockHands();
    emit handsChanged();
}

void ClockFrame::scaleClockFrame(int scaleFactor)
{
    if( (scaleFactor > 0 ) && (scaleFactor != _scaleFactor) )
    {
        _scaleFactor = scaleFactor;

        _largeHandScaled = _largeHand.scaled(_scaleFactor, _scaleFactor, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        ui->lblHandLarge->resize(_scaleFactor, _scaleFactor);

        _smallHandScaled = _smallHand.scaled(_scaleFactor, _scaleFactor, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        ui->lblHandSmall->resize(_scaleFactor, _scaleFactor);
    }

    ui->lblHandLarge->move((width()/2) - (_scaleFactor/2), (height()/2) - (_scaleFactor/2));
    ui->lblHandSmall->move((width()/2) - (_scaleFactor/2), (height()/2) - (_scaleFactor/2));

    positionClockHands();
}

void ClockFrame::positionClockHands()
{
    QTransform largeTransform;
    largeTransform.rotate(static_cast<qreal>(_largeHandAngle));
    QImage transformedImg = _largeHandScaled.transformed(largeTransform, Qt::SmoothTransformation);
    QRect rect = _largeHandScaled.rect().translated((transformedImg.width() - _largeHandScaled.width()) / 2,
                                                    (transformedImg.height() - _largeHandScaled.height()) / 2);
    ui->lblHandLarge->setPixmap(QPixmap::fromImage(transformedImg.copy(rect)));

    QTransform smallTransform;
    smallTransform.rotate(static_cast<qreal>(_smallHandAngle));
    transformedImg = _smallHandScaled.transformed(smallTransform, Qt::SmoothTransformation);
    rect = _smallHandScaled.rect().translated((transformedImg.width() - _largeHandScaled.width()) / 2,
                                              (transformedImg.height() - _largeHandScaled.height()) / 2);
    ui->lblHandSmall->setPixmap(QPixmap::fromImage(transformedImg.copy(rect)));
}
