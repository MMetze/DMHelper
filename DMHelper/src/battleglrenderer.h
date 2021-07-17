#ifndef BATTLEGLRENDERER_H
#define BATTLEGLRENDERER_H

#include <QOpenGLWidget>

class BattleDialogModel;
class BattleGLObject;

class BattleGLRenderer : public QOpenGLWidget
{
    Q_OBJECT
public:
    BattleGLRenderer(BattleDialogModel* model);

public slots:
    void cleanup();

protected:
     virtual void initializeGL() override;
     virtual void resizeGL(int w, int h) override;
     virtual void paintGL() override;

    void setOrthoProjection();

private:
    BattleDialogModel* _model;
    QSize _targetSize;
    unsigned int _backgroundTexture;
    unsigned int _fowTexture;
    unsigned int _shaderProgram;

    BattleGLObject* _backgroundObject;
    BattleGLObject* _fowObject;

    unsigned int _VAO;
    unsigned int _VBO;
    unsigned int _EBO;
    unsigned int _textureID;
};

#endif // BATTLEGLRENDERER_H
