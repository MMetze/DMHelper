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
#include <QPainterPath>

// Pixmap dimensions of the baked health-bar texture, including margin for the
// shadow + border. The bar's painted region sits inside the margins.
static constexpr int HEALTHBAR_TEX_WIDTH = 96;
static constexpr int HEALTHBAR_TEX_HEIGHT = 24;
static constexpr int HEALTHBAR_MARGIN = 2;          // pixels around the bar inside the texture for shadow/border
static constexpr int HEALTHBAR_SHADOW_OFFSET = 3;   // pixels
static constexpr qreal HEALTHBAR_CORNER_FRACTION = 0.45; // of bar height
static constexpr int HEALTHBAR_BORDER_WIDTH = 2;    // pixels

// Visual sizing of the bar relative to the token's world-space size.
static constexpr float HEALTHBAR_WIDTH_FACTOR = 1.20f;  // bar width = token width * 1.20 (10% overhang each side)
static constexpr float HEALTHBAR_HEIGHT_FACTOR = 0.16f; // bar height = token size * 0.16
static constexpr float HEALTHBAR_GAP_FACTOR = 0.04f;    // gap below token, fraction of token size

static const QColor HEALTHBAR_BACKGROUND_COLOR(180, 30, 30);
static const QColor HEALTHBAR_FOREGROUND_COLOR(40, 200, 60);
static const QColor HEALTHBAR_BORDER_COLOR(20, 20, 20);
static const QColor HEALTHBAR_SHADOW_COLOR(0, 0, 0, 120);

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
    return HEALTHBAR_TEX_WIDTH;
}

int PublishGLTokenHighlightHealthBar::getHeight() const
{
    return HEALTHBAR_TEX_HEIGHT;
}

void PublishGLTokenHighlightHealthBar::setPositionScale(const QVector3D& pos, float sizeFactor)
{
    if((HEALTHBAR_TEX_WIDTH <= 0) || (HEALTHBAR_TEX_HEIGHT <= 0))
        return;

    const float visualWidth = sizeFactor * HEALTHBAR_WIDTH_FACTOR;
    const float visualHeight = sizeFactor * HEALTHBAR_HEIGHT_FACTOR;
    const float scaleX = visualWidth / static_cast<float>(HEALTHBAR_TEX_WIDTH);
    const float scaleY = visualHeight / static_cast<float>(HEALTHBAR_TEX_HEIGHT);

    // Token center is at pos; bar center sits below by half-token + half-bar + gap.
    const float yOffset = (sizeFactor * 0.5f) + (visualHeight * 0.5f) + (sizeFactor * HEALTHBAR_GAP_FACTOR);

    _modelMatrix.setToIdentity();
    _modelMatrix.translate(pos.x(), pos.y() - yOffset, pos.z());
    _modelMatrix.scale(scaleX, scaleY);
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

    QImage img(HEALTHBAR_TEX_WIDTH, HEALTHBAR_TEX_HEIGHT, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);

    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRectF barRect(HEALTHBAR_MARGIN,
                         HEALTHBAR_MARGIN,
                         HEALTHBAR_TEX_WIDTH - (2 * HEALTHBAR_MARGIN) - HEALTHBAR_SHADOW_OFFSET,
                         HEALTHBAR_TEX_HEIGHT - (2 * HEALTHBAR_MARGIN) - HEALTHBAR_SHADOW_OFFSET);
    const qreal corner = barRect.height() * HEALTHBAR_CORNER_FRACTION;

    // Shadow
    p.setPen(Qt::NoPen);
    p.setBrush(HEALTHBAR_SHADOW_COLOR);
    p.drawRoundedRect(barRect.translated(HEALTHBAR_SHADOW_OFFSET, HEALTHBAR_SHADOW_OFFSET), corner, corner);

    // Background fill (red)
    p.setBrush(HEALTHBAR_BACKGROUND_COLOR);
    p.drawRoundedRect(barRect, corner, corner);

    // Foreground fill (green) clipped to bar shape
    if(fraction > 0.0)
    {
        QPainterPath clipPath;
        clipPath.addRoundedRect(barRect, corner, corner);
        p.save();
        p.setClipPath(clipPath);
        QRectF greenRect = barRect;
        greenRect.setWidth(barRect.width() * fraction);
        p.setBrush(HEALTHBAR_FOREGROUND_COLOR);
        p.drawRect(greenRect);
        p.restore();
    }

    // Border
    QPen borderPen(HEALTHBAR_BORDER_COLOR, HEALTHBAR_BORDER_WIDTH);
    borderPen.setJoinStyle(Qt::RoundJoin);
    p.setPen(borderPen);
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(barRect, corner, corner);

    p.end();

    delete _image;
    _image = new PublishGLImage(img, true);
    _dirty = false;
}
