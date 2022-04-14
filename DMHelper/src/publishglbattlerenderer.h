#ifndef PUBLISHGLBATTLERENDERER_H
#define PUBLISHGLBATTLERENDERER_H

#include "publishglrenderer.h"
#include "publishglbattlescene.h"
#include <QList>
#include <QHash>
#include <QImage>
#include <QColor>
#include <QMatrix4x4>

class BattleDialogModel;
class PublishGLBattleObject;
class PublishGLBattleToken;
class PublishGLBattleEffect;
class PublishGLBattleBackground;
class BattleDialogModelCombatant;
class QGraphicsItem;
class QGraphicsSimpleTextItem;

class PublishGLBattleRenderer : public PublishGLRenderer
{
    Q_OBJECT
public:
    PublishGLBattleRenderer(BattleDialogModel* model, QObject *parent);
    virtual ~PublishGLBattleRenderer() override;

    virtual CampaignObjectBase* getObject() override;
    virtual QColor getBackgroundColor() override;

    // DMH OpenGL renderer calls
    virtual void cleanup() override;
    virtual bool deleteOnDeactivation() override;
    virtual QRect getScissorRect() override;

    virtual void setBackgroundColor(const QColor& color) override;
    virtual QSizeF getBackgroundSize() = 0;

    // Standard OpenGL calls
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

public slots:
    void fowChanged();
    void setCameraRect(const QRectF& cameraRect);
    void setGrid(QImage gridImage);
    void setInitiativeType(int initiativeType);

    void distanceChanged(const QString& distance);
    void distanceItemChanged(QGraphicsItem* shapeItem, QGraphicsSimpleTextItem* textItem);

    void movementChanged(bool visible, BattleDialogModelCombatant* combatant, qreal remaining);
    void activeCombatantChanged(BattleDialogModelCombatant* activeCombatant);

protected:
    // DMH OpenGL renderer calls
    virtual void updateProjectionMatrix() override;

    // Background overrides
    virtual void initializeBackground() = 0;
    virtual bool isBackgroundReady() = 0;
    virtual void resizeBackground(int w, int h) = 0;
    virtual void paintBackground(QOpenGLFunctions* functions) = 0;
    virtual void paintTokens(QOpenGLFunctions* functions, bool drawPCs);
    virtual void updateBackground();

    virtual void updateGrid();
    virtual void updateFoW();
    virtual void createContents();
    void cleanupContents();

    virtual void updateInitiative();
    virtual void paintInitiative(QOpenGLFunctions* functions);

    virtual void createShaders();
    void destroyShaders();

protected slots:
    void recreateContents();
    void activeCombatantMoved();
    void tokenSelectionChanged(PublishGLBattleToken* token);
    void createLineToken();

protected:
    bool _initialized;
    BattleDialogModel* _model;
    PublishGLBattleScene _scene;

    QMatrix4x4 _projectionMatrix;
    QRectF _cameraRect;
    QRect _scissorRect;
    unsigned int _shaderProgramRGB;
    int _shaderModelMatrixRGB;
    int _shaderProjectionMatrixRGB;
    unsigned int _shaderProgramRGBA;
    int _shaderModelMatrixRGBA;
    int _shaderProjectionMatrixRGBA;
    int _shaderAlphaRGBA;

    QImage _gridImage;
    PublishGLImage* _gridObject;

    PublishGLBattleBackground* _fowObject;
    QHash<BattleDialogModelCombatant*, PublishGLBattleToken*> _combatantTokens;
    QHash<BattleDialogModelCombatant*, PublishGLImage*> _combatantNames;
    PublishGLImage* _unknownToken;
    PublishGLImage* _initiativeBackground;
    QList<PublishGLBattleEffect*> _effectTokens;

    int _initiativeType;
    qreal _initiativeTokenHeight;
    bool _movementVisible;
    BattleDialogModelCombatant* _movementCombatant;
    bool _movementPC;
    PublishGLImage* _movementToken;

    BattleDialogModelCombatant* _activeCombatant;
    bool _activePC;
    PublishGLImage* _activeToken;

    PublishGLImage* _selectionToken;

    bool _recreateLine;
    QGraphicsItem* _lineItem;
    QGraphicsSimpleTextItem* _lineText;
    PublishGLImage* _lineImage;
    PublishGLImage* _lineTextImage;

    bool _updateFow;
    bool _recreateContent;
};

#endif // PUBLISHGLBATTLERENDERER_H
