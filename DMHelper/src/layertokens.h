#ifndef LAYERTOKENS_H
#define LAYERTOKENS_H

#include "layer.h"
#include <QHash>

class BattleDialogModel;
class BattleDialogModelCombatant;
class PublishGLBattleToken;
class QGraphicsPixmapItem;

class LayerTokens : public Layer
{
    Q_OBJECT
public:
    explicit LayerTokens(BattleDialogModel* model, const QString& name = QString(), int order = 0, QObject *parent = nullptr);
    virtual ~LayerTokens() override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void postProcessXML(Campaign* campaign, const QDomElement &element, bool isImport) override;

    virtual QRectF boundingRect() const override;
    virtual QImage getLayerIcon() const override;
    virtual DMHelper::LayerType getType() const override;
    virtual Layer* clone() const override;

    // Local Layer Interface (generally should call set*() versions below
    virtual void applyOrder(int order) override;
    virtual void applyLayerVisible(bool layerVisible) override;
    virtual void applyOpacity(qreal opacity) override;

public slots:
    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene& scene) override;
    virtual void dmUninitialize() override;
    virtual void dmUpdate() override;

    // Player Window Generic Interface
    virtual void playerGLInitialize(PublishGLScene* scene) override;
    virtual void playerGLUninitialize() override;
    virtual void playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix) override;
    virtual void playerGLResize(int w, int h) override;

    // Layer Specific Interface
    virtual void initialize(const QSize& layerSize) override;
    virtual void uninitialize() override;
    virtual void setScale(int scale) override;

    // Local Interface
    void addCombatant(BattleDialogModelCombatant* combatant);

signals:

protected slots:
    // Local Interface

protected:
    // Layer Specific Interface
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    // DM Window Methods
    void cleanupDM();
    void createCombatantIcon(QGraphicsScene& scene, BattleDialogModelCombatant* combatant);

    // Player Window Methods
    void cleanupPlayer();

    // DM Window Members

    // Player Window Members
    PublishGLScene* _glScene;

    // Core contents
    BattleDialogModel* _model;
    QList<BattleDialogModelCombatant*> _combatants;
    QHash<QString, PublishGLBattleToken*> _tokens;
    QHash<BattleDialogModelCombatant*, QGraphicsPixmapItem*> _combatantIcons;
    QHash<BattleDialogModelCombatant*, PublishGLBattleToken*> _combatantTokens;
    int _scale;

};

#endif // LAYERTOKENS_H
