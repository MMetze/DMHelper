#ifndef LAYERTOKENS_H
#define LAYERTOKENS_H

#include "layer.h"
#include <QHash>

class BattleDialogModel;
class BattleDialogModelCombatant;
class BattleDialogModelEffect;
class PublishGLBattleToken;
class PublishGLBattleEffect;
class QGraphicsPixmapItem;
class QGraphicsItem;

class LayerTokens : public Layer
{
    Q_OBJECT
public:
    explicit LayerTokens(BattleDialogModel* model = nullptr, const QString& name = QString(), int order = 0, QObject *parent = nullptr);
    virtual ~LayerTokens() override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void postProcessXML(Campaign* campaign, const QDomElement &element, bool isImport) override;

    virtual QRectF boundingRect() const override;
    virtual QImage getLayerIcon() const override;
    virtual DMHelper::LayerType getType() const override;
    virtual Layer* clone() const override;

    // Local Layer Interface (generally should call set*() versions below)
    virtual void applyOrder(int order) override;
    virtual void applyLayerVisible(bool layerVisible) override;
    virtual void applyOpacity(qreal opacity) override;
    virtual void applyPosition(const QPoint& position) override;
    virtual void applySize(const QSize& size) override;

    // Local Interface
    const QList<BattleDialogModelCombatant*> getCombatants() const;
    QList<BattleDialogModelCombatant*> getCombatants();

public slots:
    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene* scene) override;
    virtual void dmUninitialize() override;
    virtual void dmUpdate() override;

    // Player Window Generic Interface
    virtual void playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene) override;
    virtual void playerGLUninitialize() override;
    virtual void playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix) override;
    virtual void playerGLResize(int w, int h) override;

    // Layer Specific Interface
    virtual void initialize(const QSize& layerSize) override;
    virtual void uninitialize() override;
    virtual void setScale(int scale) override;

    // Local Interface
    void setModel(BattleDialogModel* model); // Note: only works if model not yet set!
    void addCombatant(BattleDialogModelCombatant* combatant);
    void removeCombatant(BattleDialogModelCombatant* combatant);
    bool containsCombatant(BattleDialogModelCombatant* combatant);

    void addEffect(BattleDialogModelEffect* effect);
    void removeEffect(BattleDialogModelEffect* effect);
    bool containsEffect(BattleDialogModelEffect* effect);

signals:

protected slots:
    // Local Interface
    void effectChanged(BattleDialogModelEffect* effect);

protected:
    // Layer Specific Interface
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    // DM Window Methods
    void cleanupDM();
    void createCombatantIcon(QGraphicsScene* scene, BattleDialogModelCombatant* combatant);
    void createEffectIcon(QGraphicsScene* scene, BattleDialogModelEffect* effect);
    QGraphicsItem* addEffectShape(QGraphicsScene* scene, BattleDialogModelEffect* effect);
    QGraphicsItem* addSpellEffect(QGraphicsScene* scene, BattleDialogModelEffect* effect);

    // Player Window Methods
    void cleanupPlayer();

    // DM Window Members

    // Player Window Members
    PublishGLScene* _glScene;

    // Core contents
    BattleDialogModel* _model;
    QList<BattleDialogModelCombatant*> _combatants;
    QHash<QString, PublishGLBattleToken*> _combatantTokens;
    QHash<BattleDialogModelCombatant*, QGraphicsPixmapItem*> _combatantIconHash;
    QHash<BattleDialogModelCombatant*, PublishGLBattleToken*> _combatantTokenHash;

    QList<BattleDialogModelEffect*> _effects;
    QHash<QString, PublishGLBattleEffect*> _effectTokens;
    QHash<BattleDialogModelEffect*, QGraphicsItem*> _effectIconHash;
    QHash<BattleDialogModelEffect*, PublishGLBattleEffect*> _effectTokenHash;

    int _scale;
};

#endif // LAYERTOKENS_H
