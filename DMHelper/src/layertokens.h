#ifndef LAYERTOKENS_H
#define LAYERTOKENS_H

#include "layer.h"
#include <QHash>

class BattleDialogModel;
class BattleDialogModelCombatant;
class BattleDialogModelEffect;
class BattleDialogModelObject;
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
    virtual void applyLayerVisibleDM(bool layerVisible) override;
    virtual void applyLayerVisiblePlayer(bool layerVisible) override;
    virtual void applyOpacity(qreal opacity) override;
    virtual void applyPosition(const QPoint& position) override;
    virtual void applySize(const QSize& size) override;

    // Local Interface
    const QList<BattleDialogModelCombatant*> getCombatants() const;
    QList<BattleDialogModelCombatant*> getCombatants();
    QList<QGraphicsPixmapItem*> getCombatantItems();
    const QList<BattleDialogModelEffect*> getEffects() const;
    QList<BattleDialogModelEffect*> getEffects();
    QList<QGraphicsItem*> getEffectItems();
    BattleDialogModelObject* getObjectById(QUuid id);

    int getScale() const;

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
    virtual bool playerIsInitialized() override;

    // Layer Specific Interface
    virtual void initialize(const QSize& sceneSize) override;
    virtual void uninitialize() override;
    virtual void setScale(int scale) override;

    // Local Interface
    void setModel(BattleDialogModel* model); // Note: only works if model not yet set!
    void addCombatant(BattleDialogModelCombatant* combatant);
    void removeCombatant(BattleDialogModelCombatant* combatant);
    bool containsCombatant(BattleDialogModelCombatant* combatant);
    QGraphicsItem* getCombatantItem(BattleDialogModelCombatant* combatant);
    PublishGLBattleToken* getCombatantToken(BattleDialogModelCombatant* combatant);
    BattleDialogModelCombatant* getCombatantFromItem(QGraphicsPixmapItem* item);

    void addEffect(BattleDialogModelEffect* effect);
    void removeEffect(BattleDialogModelEffect* effect);
    bool containsEffect(BattleDialogModelEffect* effect);
    QGraphicsItem* getEffectItem(BattleDialogModelEffect* effect);
    BattleDialogModelEffect* getEffectFromItem(QGraphicsItem* item);
    void refreshEffects();

signals:
    void objectRemoved(BattleDialogModelObject* object);
    void postCombatantDrawGL(QOpenGLFunctions* functions, BattleDialogModelCombatant* combatant, PublishGLBattleToken* combatantToken);

protected slots:
    // Local Interface
    void handleCombatantSelected(BattleDialogModelCombatant* combatant);
    void combatantMoved(BattleDialogModelObject* object);
    void combatantConditionChanged(BattleDialogModelCombatant* combatant);
    void aliveVisibilityChanged(bool showAlive);
    void deadVisibilityChanged(bool showDead);
    void effectChanged(BattleDialogModelEffect* effect);
    void effectMoved(BattleDialogModelObject* object);
    void effectVisibilityChanged(bool showEffects);
    void linkedObjectChanged(BattleDialogModelObject* object, BattleDialogModelObject* previousLink);
    void linkedObjectMoved(BattleDialogModelObject* object);

protected:
    // Layer Specific Interface
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    // DM Window Methods
    void cleanupDM();
    QGraphicsPixmapItem* createCombatantIcon(QGraphicsScene* scene, BattleDialogModelCombatant* combatant);
    QGraphicsItem* createEffectIcon(QGraphicsScene* scene, BattleDialogModelEffect* effect);
    QGraphicsItem* addEffectShape(QGraphicsScene* scene, BattleDialogModelEffect* effect);
    QGraphicsItem* addSpellEffect(QGraphicsScene* scene, BattleDialogModelEffect* effect);
    QGraphicsItem* findCombatantItem(BattleDialogModelCombatant* combatant);
    BattleDialogModelEffect* findEffectKey(BattleDialogModelEffect* effect);
    QGraphicsItem* findEffectItem(BattleDialogModelEffect* effect);
    QGraphicsItem* findGraphicsItem(BattleDialogModelObject* object);
    qreal getTotalScale(QGraphicsItem& item);
    bool isItemInEffect(QGraphicsPixmapItem* item, QGraphicsItem* effect);
    bool isItemInEffectArea(QGraphicsPixmapItem* item, QGraphicsItem* collisionEffect);
    void removeAllEffectsFromItem(QGraphicsPixmapItem* item);
    void removeSpecificEffectFromItem(QGraphicsPixmapItem* item, BattleDialogModelEffect* effect);
    void applyEffectToItem(QGraphicsPixmapItem* item, BattleDialogModelEffect* effect);
    void applyEffectToToken(PublishGLBattleToken* token, BattleDialogModelEffect* effect);
    void removeEffectFromToken(PublishGLBattleToken* token, BattleDialogModelEffect* effect);
    QPixmap generateCombatantPixmap(BattleDialogModelCombatant* combatant);
    void applyCombatantTooltip(QGraphicsItem* item, BattleDialogModelCombatant* combatant);
    void applyCombatantVisibility(bool layerVisible, bool aliveVisible, bool deadVisible);
    void applySingleCombatantVisibility(BattleDialogModelCombatant* combatant, bool layerVisible, bool aliveVisible, bool deadVisible);
    void applyEffectVisibility(bool visible);

    // Player Window Methods
    void cleanupPlayer();

    // DM Window Members
    qreal getIconOrder(int iconType, qreal order);

    // Player Window Members
    PublishGLScene* _glScene;
    bool _playerInitialized;

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
