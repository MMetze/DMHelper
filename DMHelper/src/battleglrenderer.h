#ifndef BATTLEGLRENDERER_H
#define BATTLEGLRENDERER_H

#include <QOpenGLWidget>
#include <QList>
#include "battleglscene.h"

class BattleDialogModel;
class BattleGLObject;

class BattleGLRenderer : public QOpenGLWidget
{
    Q_OBJECT
public:
    BattleGLRenderer(BattleDialogModel* model);
    ~BattleGLRenderer();

public slots:
    void cleanup();
    void updateWidget();

protected:
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

    void setOrthoProjection();

private:
    BattleDialogModel* _model;
    BattleGLScene _scene;
    unsigned int _shaderProgram;

    BattleGLObject* _backgroundObject;
    BattleGLObject* _fowObject;
    QList<BattleGLObject*> _pcTokens;
    QList<BattleGLObject*> _enemyTokens;
    QList<BattleGLObject*> _effectTokens;
};

#endif // BATTLEGLRENDERER_H
