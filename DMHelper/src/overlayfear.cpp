#include "overlayfear.h"
#include "publishglimage.h"
#include "campaign.h"
#include "dmconstants.h"
#include "overlayframe.h"
#include "fearframe.h"
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QHBoxLayout>

OverlayFear::OverlayFear(const QString& name, QObject *parent) :
    Overlay{name, parent},
    _fearCounter(nullptr)
{
}

int OverlayFear::getOverlayType() const
{
    return DMHelper::OverlayType_Fear;
}

void OverlayFear::prepareFrame(OverlayFrame* frame)
{
    if(!_campaign)
        return;

    FearFrame* fearFrame = new FearFrame();
    fearFrame->setCampaign(_campaign);

    frame->getLayout()->insertWidget(OverlayFrame::OVERLAY_FRAME_INSERT_POINT, fearFrame);
}

QSize OverlayFear::getSize() const
{
    return _fearCounter ? _fearCounter->getSize() : QSize();
}

void OverlayFear::setX(int x)
{
    if(_fearCounter)
        _fearCounter->setX(static_cast<qreal>(x));
}

void OverlayFear::setY(int y)
{
    if(_fearCounter)
        _fearCounter->setY(static_cast<qreal>(y));
}

void OverlayFear::doSetCampaign(Campaign* campaign)
{
    if(_campaign)
        disconnect(_campaign, &Campaign::fearChanged, this, &OverlayFear::recreateContents);

    if(campaign)
        connect(campaign, &Campaign::fearChanged, this, &OverlayFear::recreateContents);
}

void OverlayFear::doPaintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix)
{
    Q_UNUSED(targetSize);

    if((!_fearCounter) || (!functions))
        return;

    DMH_DEBUG_OPENGL_glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, _fearCounter->getMatrixData(), _fearCounter->getMatrix());
    functions->glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, _fearCounter->getMatrixData());
    _fearCounter->paintGL(functions, nullptr);
}

void OverlayFear::doResizeGL(int w, int h)
{
    Q_UNUSED(w);

    if(!_fearCounter)
        return;

    _fearCounter->setScale(static_cast<qreal>(h) * getScale() / static_cast<qreal>(_fearCounter->getImageSize().height()));
//    _fearCounter->setX(static_cast<qreal>(w) - _fearCounter->getSize().width());
//    _fearCounter->setY(static_cast<qreal>(h) - _fearCounter->getSize().height());
}

void OverlayFear::createContentsGL()
{
    if(_fearCounter)
    {
        delete _fearCounter;
        _fearCounter = nullptr;
    }

    if(!_campaign)
        return;

    QImage fearCounterImageBorder(QString(":/img/data/hoodeyelessborder.png"));
    QImage fearCounterImageGrey(QString(":/img/data/hoodeyeless.png"));
    QImage fearCounterImage(fearCounterImageGrey.size(), QImage::Format_ARGB32_Premultiplied);
    fearCounterImage.fill(Qt::transparent);
    QPainter p(&fearCounterImage);
        QColor redColor(qBound(0, 255 * _campaign->getFearCount() / 12, 255), 0, 0, 196);
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
        path.addText(0, 0, f, QString::number(_campaign->getFearCount()));
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

    _fearCounter = new PublishGLImage(fearCounterImage, false);
}
