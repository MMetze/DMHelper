#ifndef PUBLISHGLBATTLEGRID_H
#define PUBLISHGLBATTLEGRID_H

#include "publishglbattleobject.h"
#include "gridconfig.h"
#include "grid.h"
#include <QOpenGLFunctions>

//#define DEBUG_BATTLE_GRID

class PublishGLBattleGrid : public PublishGLBattleObject, public Grid_LineInterface
{
    Q_OBJECT
public:
    PublishGLBattleGrid(const GridConfig& config, qreal opacity, const QSizeF& gridSize);
    virtual ~PublishGLBattleGrid() override;

    virtual void cleanup() override;
    virtual void paintGL(QOpenGLFunctions* functions, const GLfloat* projectionMatrix) override;
    QSize getSize() const;

    void setPosition(const QPoint& position);
    void setSize(const QSize& size);

    void setProjectionMatrix(const GLfloat* projectionMatrix);
    void setConfig(const GridConfig& config);
    void setOpacity(qreal opacity);

    virtual void addLine(int x0, int y0, int x1, int y1, int zOrder) override;

protected:
    void createGridObjectsGL();
    void rebuildGridGL();
    void cleanupGridGL();
    void updateModelMatrix();

    void rebuildGrid();

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

    bool _recreateGrid;
};

#endif // PUBLISHGLBATTLEGRID_H
