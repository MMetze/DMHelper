#ifndef PUBLISHGLBATTLERENDERER_H
#define PUBLISHGLBATTLERENDERER_H

#include "publishglrenderer.h"
#include "battleglscene.h"
#include <QList>
#include <QHash>
#include <QColor>
#include <QMatrix4x4>

class BattleDialogModel;
class BattleGLObject;
class BattleGLToken;
class BattleGLBackground;
class BattleDialogModelCombatant;

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
    int _shaderProjectionMatrix;

    BattleGLBackground* _backgroundObject;
    BattleGLBackground* _fowObject;
    QHash<BattleDialogModelCombatant*, BattleGLToken*> _combatantTokens;
    QHash<BattleDialogModelCombatant*, PublishGLImage*> _combatantNames;
    PublishGLImage* _unknownToken;
    QList<BattleGLObject*> _effectTokens;

    bool _updateFow;
};

#endif // PUBLISHGLBATTLERENDERER_H
