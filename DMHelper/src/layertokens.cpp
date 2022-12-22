#include "layertokens.h"

LayerTokens::LayerTokens(const QString& name, int order, QObject *parent) :
    Layer{name, order, parent}
{
}

~LayerTokens()

void inputXML(const QDomElement &element, bool isImport)
QRectF boundingRect() const
QImage getLayerIcon() const
DMHelper::LayerType getType() const
Layer* clone() const
void dmInitialize(QGraphicsScene& scene)
void dmUninitialize()
void dmUpdate()
void playerGLInitialize()
void playerGLUninitialize()
void playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
void playerGLResize(int w, int h)
void initialize(const QSize& layerSize)
void uninitialize()
void setOrder(int order)
void setLayerVisible(bool layerVisible)
void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
void cleanupDM();
void cleanupPlayer();
