#ifndef PUBLISHGLBATTLERENDERER_H
#define PUBLISHGLBATTLERENDERER_H

#include "publishglrenderer.h"
#include "battleglscene.h"
#include <QList>
#include <QColor>
#include <QMatrix4x4>

class BattleDialogModel;
class BattleGLObject;
class BattleGLBackground;

class PublishGLBattleRenderer : public PublishGLRenderer
{
    Q_OBJECT
public:
    PublishGLBattleRenderer(BattleDialogModel* model);
    virtual ~PublishGLBattleRenderer() override;

    virtual CampaignObjectBase* getObject() override;
    virtual QColor getBackgroundColor() override;

    // DMH OpenGL renderer calls
    virtual void cleanup() override;
    virtual bool deleteOnDeactivation() override;
    virtual void setBackgroundColor(const QColor& color) override;

    // Standard OpenGL calls
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

public slots:
    void fowChanged();
    void setCameraRect(const QRectF& cameraRect);

protected:
    virtual void updateProjectionMatrix() override;

private:
    bool _initialized;
    BattleDialogModel* _model;
    BattleGLScene _scene;

    QMatrix4x4 _projectionMatrix;
    QRectF _cameraRect;
    QRect _scissorRect;
    unsigned int _shaderProgram;
    int _shaderModelMatrix;

    BattleGLBackground* _backgroundObject;
    BattleGLBackground* _fowObject;
    QList<BattleGLObject*> _pcTokens;
    QList<BattleGLObject*> _enemyTokens;
    QList<BattleGLObject*> _effectTokens;

    bool _updateFow;
};

#endif // PUBLISHGLBATTLERENDERER_H
