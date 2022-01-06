#ifndef BATTLEGLTOKEN_H
#define BATTLEGLTOKEN_H

#include "battleglobject.h"

class BattleDialogModelCombatant;

class BattleGLToken : public BattleGLObject
{
    Q_OBJECT

public:
    BattleGLToken(BattleGLScene* scene, BattleDialogModelCombatant* combatant, bool isPC = false);
    virtual ~BattleGLToken() override;

    virtual void cleanup() override;
    virtual void paintGL() override;

    QSizeF getTextureSize() const;
    bool isPC() const;

public slots:
    void combatantMoved();
    void setPC(bool isPC);

protected:
    BattleDialogModelCombatant* _combatant;
    unsigned int _VAO;
    unsigned int _VBO;
    unsigned int _EBO;

    QSizeF _textureSize;
    bool _isPC;
};

#endif // BATTLEGLTOKEN_H
