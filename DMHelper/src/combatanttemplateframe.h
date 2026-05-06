#ifndef COMBATANTTEMPLATEFRAME_H
#define COMBATANTTEMPLATEFRAME_H

#include "combatantwidget.h"
#include "templateframe.h"
#include <QPointer>
#include <QPixmap>
#include <QSize>
#include <QUuid>

class BattleDialogModelCombatant;
class BattleDialogModelMonsterBase;
class BattleDialogModelCharacter;
class CombatantTemplateAdapter;
class CombatantWidgetBase;
class QWidget;
class QScrollArea;
class QFrame;

// Template-driven replacement for CombatantWidgetMonster /
// CombatantWidgetCharacter. Loads a combatant.ui template through
// TemplateFactory and binds it to a BattleDialogModelCombatant via a
// CombatantTemplateAdapter. Hosts the conditions strip, per-round resource
// strip, and forwards the legacy CombatantWidget contract to BattleFrame.
class CombatantTemplateFrame : public CombatantWidget, public TemplateFrame
{
    Q_OBJECT
public:
    CombatantTemplateFrame(BattleDialogModelCombatant* combatant, bool showDone, const QString& templateFile, QWidget* parent = nullptr);
    virtual ~CombatantTemplateFrame() override;

    // From CombatantWidget
    virtual BattleDialogModelCombatant* getCombatant() override;
    virtual int getInitiative() const override;
    virtual bool isShown() override;
    virtual bool isKnown() override;
    virtual void setShowDone(bool showDone) override;
    virtual void disconnectInternals() override;

signals:
    void clicked(const QString& monsterClass);
    void clickedCharacter(const QUuid& characterId);
    void hitPointsChanged(BattleDialogModelCombatant* combatant, int change);

public slots:
    virtual void updateData() override;
    virtual void updateMove() override;
    virtual void selectCombatant() override;

protected:
    // From QWidget
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
    virtual void contextMenuEvent(QContextMenuEvent* event) override;

    // From QObject
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

    // From TemplateFrame
    virtual QObject* getFrameObject() override;
    virtual bool localEventFilter(QObject* object, QEvent* event) override;

private slots:
    void handleResourceCountChanged(BattleDialogModelMonsterBase* monster, const QString& name, int value);
    void handleConditionsChanged(BattleDialogModelCombatant* combatant);
    void handleModelImageChanged(BattleDialogModelCombatant* combatant);
    void handleMonsterImageChanged(BattleDialogModelMonsterBase* monster);
    void handleCharacterImageChanged(BattleDialogModelCharacter* character);

private:
    bool loadTemplate(const QString& templateFile);
    void rebuildBindings();
    void applyIcon();
    void applyConditionDecorations();
    void applyResourceDecorations();
    void wireResourceButtons(QWidget* resourceWidget, int index);
    QScrollArea* findScrollArea(const QString& dmhValueKey) const;
    void connectModelSignals();
    void emitDoubleClickSignal();
    void showConditionContextMenu(const QString& conditionId, const QPoint& globalPos);
    void showAddConditionMenu(const QPoint& globalPos);

    QPointer<QScrollArea> _conditionStrip;
    QPointer<QScrollArea> _resourceStrip;

    BattleDialogModelCombatant* _combatant;
    CombatantTemplateAdapter* _adapter;
    CombatantWidgetBase* _base;
    QPointer<QWidget> _uiWidget;
    bool _showDone;
    int _previousHitPoints;

    // Cache for scaled icon pixmap to avoid expensive Qt::SmoothTransformation rescaling on every rebuild.
    QPixmap _cachedScaledIcon;
    qint64 _cachedIconSourceKey;
    QSize _cachedIconTargetSize;
    qreal _cachedIconDpr;
};

#endif // COMBATANTTEMPLATEFRAME_H
