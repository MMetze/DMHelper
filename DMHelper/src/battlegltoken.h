#ifndef BATTLEGLTOKEN_H
#define BATTLEGLTOKEN_H

#include "battleglobject.h"

class BattleDialogModelCombatant;

class BattleGLToken : public BattleGLObject
{
    Q_OBJECT

public:
    BattleGLToken(BattleGLScene& scene, BattleDialogModelCombatant* combatant);
    virtual ~BattleGLToken() override;

    virtual void paintGL() override;

public slots:
    void combatantMoved();

protected:
    BattleDialogModelCombatant* _combatant;
    unsigned int _VAO;
    unsigned int _VBO;
    unsigned int _EBO;
    QSizeF _textureSize;
};

#endif // BATTLEGLTOKEN_H
