#ifndef PUBLISHGLBATTLETOKEN_H
#define PUBLISHGLBATTLETOKEN_H

#include "publishglbattleobject.h"
#include <QList>

class BattleDialogModelCombatant;
class PublishGLImage;
class PublishGLEffect;

class PublishGLBattleToken : public PublishGLBattleObject
{
    Q_OBJECT

public:
    PublishGLBattleToken(PublishGLScene* scene, BattleDialogModelCombatant* combatant, bool isPC = false);
    virtual ~PublishGLBattleToken() override;

    virtual void cleanup() override;
    virtual void paintGL() override;
    virtual void paintEffects(int shaderModelMatrix);

    BattleDialogModelCombatant* getCombatant() const;
    QSizeF getTextureSize() const;
    bool isPC() const;

    void addEffect(PublishGLImage& effectImage);
    void removeEffect(const PublishGLImage& effectImage);

signals:
    void selectionChanged(PublishGLBattleToken* token);

public slots:
    void combatantMoved();
    void combatantSelected();
    void setPC(bool isPC);
    void recreateToken();

protected:
    void createTokenObjects();

    BattleDialogModelCombatant* _combatant;
    unsigned int _VAO;
    unsigned int _VBO;
    unsigned int _EBO;

    QSizeF _textureSize;
    bool _isPC;

    QList<PublishGLEffect*> _effectList;

    bool _recreateToken;
};

#endif // PUBLISHGLBATTLETOKEN_H
