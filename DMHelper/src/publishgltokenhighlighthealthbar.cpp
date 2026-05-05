#include "publishgltokenhighlighthealthbar.h"
#include "battledialogmodelcombatant.h"
#include "battledialogmodelcharacter.h"
#include "battledialogmodelmonsterbase.h"
#include "campaignobjectbase.h"
#include "characterv2.h"
#include "publishglimage.h"
#include "rulehealth.h"
#include "dmh_opengl.h"
#include <QImage>
#include <QPainter>

static constexpr int HEALTHBAR_WIDTH = 64;
static constexpr int HEALTHBAR_HEIGHT = 8;
static constexpr float HEALTH_BAR_Y_BIAS = 0.5f;

PublishGLTokenHighlightHealthBar::PublishGLTokenHighlightHealthBar(BattleDialogModelCombatant* combatant, QObject* parent) :
    PublishGLTokenHighlight(parent),
    _combatant(combatant),
    _image(nullptr),
    _dirty(true)
{
    if(!_combatant)
        return;

    BattleDialogModelCharacter* charCombatant = qobject_cast<BattleDialogModelCharacter*>(_combatant);
    if(charCombatant)
    {
        Characterv2* character = charCombatant->getCharacter();
        if(character)
            connect(character, &CampaignObjectBase::dirty, this, &PublishGLTokenHighlightHealthBar::onCombatantChanged, Qt::QueuedConnection);
    }
    else
    {
        BattleDialogModelMonsterBase* monsterBase = qobject_cast<BattleDialogModelMonsterBase*>(_combatant);
        if(monsterBase)
            connect(monsterBase, &BattleDialogModelMonsterBase::dataChanged, this,
                    [this](BattleDialogModelMonsterBase*) { onCombatantChanged(); },
                    Qt::QueuedConnection);
    }
}

PublishGLTokenHighlightHealthBar::~PublishGLTokenHighlightHealthBar()
{
    delete _image;
}

void PublishGLTokenHighlightHealthBar::paintGL(QOpenGLFunctions* f, int shaderModelMatrix)
{
    if(!f)
        return;

    if(_dirty || !_image)
        rebuildPixmap();

    if(!_image)
        return;

    DMH_DEBUG_OPENGL_glUniformMatrix4fv(shaderModelMatrix, 1, GL_FALSE, getMatrixData(), getMatrix());
    f->glUniformMatrix4fv(shaderModelMatrix, 1, GL_FALSE, getMatrixData());
    _image->paintGL(f, nullptr);
}

int PublishGLTokenHighlightHealthBar::getWidth() const
{
    return HEALTHBAR_WIDTH;
}

int PublishGLTokenHighlightHealthBar::getHeight() const
{
    return HEALTHBAR_HEIGHT;
}

void PublishGLTokenHighlightHealthBar::setPositionScale(const QVector3D& pos, float sizeFactor)
{
    int maxDim = qMax(getWidth(), getHeight());
    if(maxDim <= 0)
        return;

    float scaleFactor = sizeFactor / static_cast<float>(maxDim);
    _modelMatrix.setToIdentity();
    _modelMatrix.translate(pos.x(), pos.y() + sizeFactor * HEALTH_BAR_Y_BIAS, pos.z());
    _modelMatrix.scale(scaleFactor, scaleFactor);
}

void PublishGLTokenHighlightHealthBar::onCombatantChanged()
{
    _dirty = true;
}

void PublishGLTokenHighlightHealthBar::rebuildPixmap()
{
    qreal fraction = 0.0;
    RuleHealth* rule = RuleHealth::forCombatant(_combatant);
    if(rule)
        fraction = rule->getHealthFraction(_combatant);

    QImage img(HEALTHBAR_WIDTH, HEALTHBAR_HEIGHT, QImage::Format_ARGB32);
    img.fill(Qt::red);
    QPainter p(&img);
    p.fillRect(0, 0, qRound(static_cast<qreal>(HEALTHBAR_WIDTH) * fraction), HEALTHBAR_HEIGHT, Qt::green);
    p.end();

    delete _image;
    _image = new PublishGLImage(img, true);
    _dirty = false;
}
