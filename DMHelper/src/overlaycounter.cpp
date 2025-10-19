#include "overlaycounter.h"
#include "publishglimage.h"
#include "dmconstants.h"
#include <QDomElement>
#include <QPainter>
#include <QPainterPath>

OverlayCounter::OverlayCounter(int counter, const QString& name, QObject *parent) :
    Overlay{name, parent},
    _counterImage(nullptr),
    _counter(counter)
{
}

void OverlayCounter::inputXML(const QDomElement &element, bool isImport)
{
    setCounterValue(element.attribute(QString("counter"), QString::number(0)).toInt());

    Overlay::inputXML(element, isImport);
}

void OverlayCounter::copyValues(const CampaignObjectBase* other)
{
    const OverlayCounter* otherOverlayCounter = dynamic_cast<const OverlayCounter*>(other);
    if(!otherOverlayCounter)
        return;

    _counter = otherOverlayCounter->_counter;

    Overlay::copyValues(other);
}

int OverlayCounter::getOverlayType() const
{
    return DMHelper::OverlayType_Counter;
}

int OverlayCounter::getCounterValue() const
{
    return _counter;
}

void OverlayCounter::setCounterValue(int value)
{
    if(_counter == value)
        return;

    _counter = value;
    emit triggerUpdate();
}

void OverlayCounter::increase()
{
    ++_counter;
    emit triggerUpdate();
}

void OverlayCounter::decrease()
{
    --_counter;
    emit triggerUpdate();
}

void OverlayCounter::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute(QString("counter"), QString::number(_counter));

    Overlay::internalOutputXML(doc, element, targetDirectory, isExport);
}

void OverlayCounter::doPaintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix)
{
    Q_UNUSED(targetSize);

    if((!_counterImage) || (!functions))
        return;

    DMH_DEBUG_OPENGL_glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, _counterImage->getMatrixData(), _counterImage->getMatrix());
    functions->glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, _counterImage->getMatrixData());
    _counterImage->paintGL(functions, nullptr);
}

void OverlayCounter::createContentsGL()
{
    if(_counterImage)
    {
        delete _counterImage;
        _counterImage = nullptr;
    }

    QImage fearCounterImageBorder(QString(":/img/data/hoodeyelessborder.png"));
    QImage fearCounterImageGrey(QString(":/img/data/hoodeyeless.png"));
    QImage fearCounterImage(fearCounterImageGrey.size(), QImage::Format_ARGB32_Premultiplied);
    fearCounterImage.fill(Qt::transparent);
    QPainter p(&fearCounterImage);
    QColor redColor(qBound(0, 255 * _counter / 12, 255), 0, 0, 196);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(fearCounterImage.rect(), redColor);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.drawImage(0, 0, fearCounterImageGrey);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.drawImage(0, 0, fearCounterImageBorder);

    QFont f = p.font();
    f.setPixelSize(256);

    f.setStyleStrategy(QFont::ForceOutline);
    QPainterPath path;
    path.addText(0, 0, f, QString::number(_counter));
    QRectF bounds = path.boundingRect();
    QPointF center(fearCounterImage.width()/2.0 - bounds.width()/2.0 - bounds.left(),
                   fearCounterImage.height()/2.0 + bounds.height());
    QTransform transform;
    transform.translate(center.x(), center.y());
    QPainterPath centeredPath = transform.map(path);

    // Draw outline
    p.setPen(QPen(Qt::white, 5));
    p.setBrush(redColor);
    p.drawPath(centeredPath);
    p.end();

    _counterImage = new PublishGLImage(fearCounterImage, false);}

void OverlayCounter::updateContentsScale(int w, int h)
{
    if(!_counterImage)
        return;

    qreal tokenHeight = static_cast<qreal>(h) / 10.0;
    _counterImage->setScale(tokenHeight / static_cast<qreal>(_counterImage->getImageSize().height()));
    _counterImage->setX(static_cast<qreal>(w) - _counterImage->getSize().width());
    _counterImage->setY(static_cast<qreal>(h) - _counterImage->getSize().height());
}
