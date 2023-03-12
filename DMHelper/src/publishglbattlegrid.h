#ifndef PUBLISHGLBATTLEGRID_H
#define PUBLISHGLBATTLEGRID_H

#include "publishglbattleobject.h"
#include "gridconfig.h"
#include "grid.h"
#include <QOpenGLFunctions>

class PublishGLBattleGrid : public PublishGLBattleObject, public Grid_LineInterface
{
    Q_OBJECT
public:
    PublishGLBattleGrid(const GridConfig& config, qreal opacity, const QSizeF& gridSize);
    virtual ~PublishGLBattleGrid() override;

    virtual void cleanup() override;
    virtual void paintGL() override;
    QSize getSize() const;

    void setPosition(const QPoint& position);

    void setProjectionMatrix(const GLfloat* projectionMatrix);
    void setConfig(const GridConfig& config);
    void setOpacity(qreal opacity);

    virtual void addLine(int x0, int y0, int x1, int y1, int zOrder) override;

protected:
    void createGridObjects();
    void rebuildGrid();
    void cleanupGrid();
    void updateModelMatrix();

    unsigned int _VAO;
    unsigned int _VBO;
    unsigned int _EBO;
    unsigned int _shaderProgram;
    int _shaderModelMatrix;

    GridConfig _config;
    QPoint _position;
    qreal _opacity;
    QSizeF _gridSize;

    QVector<float> _vertices;
    QVector<unsigned int> _indices;
};

#endif // PUBLISHGLBATTLEGRID_H
