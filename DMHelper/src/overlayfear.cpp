#include "overlayfear.h"
#include "publishglimage.h"
#include "campaign.h"
#include "dmconstants.h"
#include <QImage>
#include <QPainter>
#include <QPainterPath>

OverlayFear::OverlayFear(QObject *parent) :
    Overlay{parent},
    _fearCounter(nullptr)
{
}

int OverlayFear::getOverlayType() const
{
    return DMHelper::OverlayType_Fear;
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

void OverlayFear::createContentsGL()
{
    if(_fearCounter)
    {
        delete _fearCounter;
        _fearCounter = nullptr;
    }

    if((!_campaign) || (!_campaign->getShowFear()) || (!_campaign->getRuleset().objectName().contains(QString("daggerheart"), Qt::CaseInsensitive)))
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

void OverlayFear::updateContentsScale(int w, int h)
{
    if(!_fearCounter)
        return;

    qreal tokenHeight = static_cast<qreal>(h) / 10.0;
    _fearCounter->setScale(tokenHeight / static_cast<qreal>(_fearCounter->getImageSize().height()));
    _fearCounter->setX(static_cast<qreal>(w) - _fearCounter->getSize().width());
    _fearCounter->setY(static_cast<qreal>(h) - _fearCounter->getSize().height());
}
