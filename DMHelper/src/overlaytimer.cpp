#include "overlaytimer.h"
#include "publishglimage.h"
#include "dmconstants.h"
#include "overlayframe.h"
#include <QDomElement>
#include <QPainter>
#include <QPainterPath>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>

OverlayTimer::OverlayTimer(int seconds, const QString& name, QObject *parent) :
    Overlay{name, parent},
    _timerPublishImage(nullptr),
    _timerImage(),
    _seconds(seconds),
    _timerId(0)
{
}

void OverlayTimer::inputXML(const QDomElement &element)
{
    setTimerValue(element.attribute(QString("seconds"), QString::number(0)).toInt());

    Overlay::inputXML(element);
}

int OverlayTimer::getOverlayType() const
{
    return DMHelper::OverlayType_Timer;
}

QSize OverlayTimer::getSize() const
{
    return _timerPublishImage ? _timerPublishImage->getSize() : QSize();
}

void OverlayTimer::prepareFrame(QBoxLayout* frameLayout, int insertIndex)
{
    if(!frameLayout)
        return;

    QLineEdit* edtTimerValue = new QLineEdit();
    edtTimerValue->setText(QString::number(_seconds));
    edtTimerValue->setStyleSheet("font-weight: bold; font-size: 14pt;");
    connect(edtTimerValue, &QLineEdit::textEdited, this, &OverlayTimer::setTimerString);
    connect(this, &OverlayTimer::timerValueChanged, edtTimerValue, [edtTimerValue](int seconds){ edtTimerValue->setText(QString::number(seconds)); });

    QPushButton* btnStart = new QPushButton(QIcon(":/img/data/icon_play.png"), QString());
    btnStart->setCheckable(true);
    btnStart->setChecked(_timerId > 0);
    connect(btnStart, &QPushButton::toggled, this, &OverlayTimer::toggle);
    connect(this, &OverlayTimer::timerStatusChanged, btnStart, [btnStart](bool running)
    {
        btnStart->setChecked(running);
        btnStart->setIcon(running ? QIcon(":/img/data/icon_stop.png") : QIcon(":/img/data/icon_play.png"));
    });

    frameLayout->insertWidget(insertIndex, btnStart);
    frameLayout->insertWidget(insertIndex, edtTimerValue);

}

int OverlayTimer::getTimerValue() const
{
    return _seconds;
}

void OverlayTimer::setX(int x)
{
    if(_timerPublishImage)
        _timerPublishImage->setX(static_cast<qreal>(x));
}

void OverlayTimer::setY(int y)
{
    if(_timerPublishImage)
        _timerPublishImage->setY(static_cast<qreal>(y));
}

void OverlayTimer::setTimerValue(int seconds)
{
    if(_seconds == seconds)
        return;

    _seconds = seconds;
    recreateContents();
    emit timerValueChanged(_seconds);
}

void OverlayTimer::setTimerString(const QString& seconds)
{
    if(seconds.isEmpty())
        return;

    setTimerValue(seconds.toInt());
}

void OverlayTimer::toggle(bool play)
{
    if(play)
        start();
    else
        stop();
}

void OverlayTimer::start()
{
    if(_timerId > 0)
        return;

    _timerId = startTimer(1000);
    emit timerStatusChanged(true);
}

void OverlayTimer::stop()
{
    if(_timerId == 0)
        return;

    killTimer(_timerId);
    _timerId = 0;
    emit timerStatusChanged(false);
}

void OverlayTimer::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    if(_timerId == 0)
        return;

    setTimerValue(getTimerValue() - 1);
    updateContents();

    if(getTimerValue() <= 0)
    {
        stop();
        emit timerExpired();
    }
}

void OverlayTimer::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory)
{
    element.setAttribute(QString("seconds"), QString::number(_seconds));

    Overlay::internalOutputXML(doc, element, targetDirectory);
}

void OverlayTimer::doPaintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix)
{
    Q_UNUSED(targetSize);

    if((!_timerPublishImage) || (!functions))
        return;

    DMH_DEBUG_OPENGL_glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, _timerPublishImage->getMatrixData(), _timerPublishImage->getMatrix());
    functions->glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, _timerPublishImage->getMatrixData());
    _timerPublishImage->paintGL(functions, nullptr);
}

void OverlayTimer::doResizeGL(int w, int h)
{
    Q_UNUSED(w);

    if(!_timerPublishImage)
        return;

    _timerPublishImage->setScale(static_cast<qreal>(h) * getScale() / static_cast<qreal>(_timerPublishImage->getImageSize().height()));
//    _timerPublishImage->setX(static_cast<qreal>(w) - _timerPublishImage->getSize().width());
//    _timerPublishImage->setY(static_cast<qreal>(h) - (_timerPublishImage->getSize().height() * 3));
}

void OverlayTimer::createContentsGL()
{
    if(_timerPublishImage)
    {
        delete _timerPublishImage;
        _timerPublishImage = nullptr;
    }

    _timerImage = QImage();
    createTimerImage();
    _timerPublishImage = new PublishGLImage(_timerImage, false);
}

void OverlayTimer::updateContentsGL()
{
    if(!_timerPublishImage)
        return;

    createTimerImage();
    _timerPublishImage->updateImage(_timerImage);
}

void OverlayTimer::createTimerImage()
{
    // Build text path at origin
    QString timerString = QString::number(_seconds / 60) + QString(":");
    int modSeconds = _seconds % 60;
    if(modSeconds < 10)
        timerString += QString("0");
    timerString += QString::number(modSeconds);

    QImage timerImageBorder(QString(":/img/data/icon_overlaytimerbackground.png"));

    // Draw the number for the counter
    QImage timerNumberImage = textToImage(timerString).scaledToWidth(static_cast<int>(static_cast<qreal>(timerImageBorder.width()) * 0.75), Qt::SmoothTransformation);
    QPoint textLocation = QPoint((timerImageBorder.width() - timerNumberImage.width()) / 2,
                                 (timerImageBorder.height() - timerNumberImage.height()) / 2);

    // Create an image just large enough
    if(_timerImage.isNull())
        _timerImage = QImage(timerImageBorder.size()/*.grownBy(QMargins(4, 4, 4, 4))*/, QImage::Format_ARGB32_Premultiplied);

    _timerImage.fill(Qt::transparent);

    // Prepare painter
    QPainter p(&_timerImage);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.drawImage(0, 0, timerImageBorder);

        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(Qt::white, 5));
        p.setBrush(QColor(115, 18, 0));
        p.drawImage(textLocation, timerNumberImage);

    p.end();
}
