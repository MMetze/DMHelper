#include "layertokens.h"
#include <QImage>

LayerTokens::LayerTokens(const QString& name, int order, QObject *parent) :
    Layer{name, order, parent}
{
}

LayerTokens::~LayerTokens()
{
    cleanupDM();
    cleanupPlayer();
}

void LayerTokens::inputXML(const QDomElement &element, bool isImport)
{
    Layer::inputXML(element, isImport);
}

QRectF LayerTokens::boundingRect() const
{
    return QRectF();
}

QImage LayerTokens::getLayerIcon() const
{
    return QImage(":/img/data/icon_contentcharacter.png");
}

DMHelper::LayerType LayerTokens::getType() const
{
    return DMHelper::LayerType_Tokens;
}

Layer* LayerTokens::clone() const
{
    LayerTokens* newLayer = new LayerTokens(_name, _order);

    copyBaseValues(newLayer);

    return newLayer;
}

void LayerTokens::applyOrder(int order)
{
}

void LayerTokens::applyLayerVisible(bool layerVisible)
{
}

void LayerTokens::applyOpacity(qreal opacity)
{
}

void LayerTokens::dmInitialize(QGraphicsScene& scene)
{
    Layer::dmInitialize(scene);
}

void LayerTokens::dmUninitialize()
{
    cleanupDM();
}

void LayerTokens::dmUpdate()
{
}

void LayerTokens::playerGLInitialize()
{
}

void LayerTokens::playerGLUninitialize()
{
    cleanupPlayer();
}

void LayerTokens::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
}

void LayerTokens::playerGLResize(int w, int h)
{
}

void LayerTokens::initialize(const QSize& layerSize)
{
}

void LayerTokens::uninitialize()
{
}

void LayerTokens::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
}

void LayerTokens::cleanupDM()
{
}

void LayerTokens::cleanupPlayer()
{
}

