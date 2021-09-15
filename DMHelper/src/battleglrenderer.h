#ifndef BATTLEGLRENDERER_H
#define BATTLEGLRENDERER_H

#include "publishglrenderer.h"
#include "battleglscene.h"
#include <QList>

class BattleDialogModel;
class BattleGLObject;

class BattleGLRenderer : public PublishGLRenderer
{
    Q_OBJECT
public:
    BattleGLRenderer(BattleDialogModel* model);
    virtual ~BattleGLRenderer() override;

    // DMH OpenGL renderer calls
    virtual void cleanup() override;

    // Standard OpenGL calls
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

protected:
    void setOrthoProjection();

private:
    bool _initialized;
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
