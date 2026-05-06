#ifndef PUBLISHGLTOKENHIGHLIGHTHEALTHBAR_H
#define PUBLISHGLTOKENHIGHLIGHTHEALTHBAR_H

#include "publishgltokenhighlight.h"

class BattleDialogModelCombatant;
class PublishGLImage;

class PublishGLTokenHighlightHealthBar : public PublishGLTokenHighlight
{
    Q_OBJECT
public:
    explicit PublishGLTokenHighlightHealthBar(BattleDialogModelCombatant* combatant, QObject* parent = nullptr);
    virtual ~PublishGLTokenHighlightHealthBar() override;

    virtual void paintGL(QOpenGLFunctions* f, int shaderModelMatrix) override;

    virtual int getWidth() const override;
    virtual int getHeight() const override;

    virtual void setPositionScale(const QVector3D& pos, float sizeFactor) override;

private slots:
    void onCombatantChanged();

private:
    void rebuildPixmap();

    BattleDialogModelCombatant* _combatant;
    PublishGLImage* _image;
    bool _dirty;
};

#endif // PUBLISHGLTOKENHIGHLIGHTHEALTHBAR_H
