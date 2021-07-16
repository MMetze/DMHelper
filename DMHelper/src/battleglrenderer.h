#ifndef BATTLEGLRENDERER_H
#define BATTLEGLRENDERER_H

#include <QOpenGLWidget>

class BattleDialogModel;

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

private:
    BattleDialogModel* _model;
    QSize _targetSize;
    unsigned int _backgroundTexture;
    unsigned int _fowTexture;
    unsigned int _shaderProgram;
    unsigned int _VBO;
    unsigned int _EBO;

};

#endif // BATTLEGLRENDERER_H
