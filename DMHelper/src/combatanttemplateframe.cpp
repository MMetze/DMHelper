#include "combatanttemplateframe.h"
#include "combatanttemplateadapter.h"
#include "combatantwidgetbase.h"
#include "templatefactory.h"
#include "battledialogmodelcombatant.h"
#include "battledialogmodelmonsterbase.h"
#include "battledialogmodelcharacter.h"
#include "monsterclassv2.h"
#include "characterv2.h"
#include "perroundresource.h"
#include "conditions.h"
#include "dmconstants.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QPixmap>
#include <QSignalBlocker>
#include <QDebug>

static const int    COMBATANT_ICON_SIZE      = 64;
static const int    CONDITION_ICON_SIZE      = 28;
static const char*  CONDITION_PROPERTY_KEY   = "dmhConditionId";
static const char*  RESOURCE_PROPERTY_KEY    = "dmhResourceName";
static const char*  RESOURCE_DECREMENT_NAME  = "resourceDecrement";
static const char*  RESOURCE_RESET_NAME      = "resourceReset";
static const char*  RESOURCE_NAME_LABEL_NAME = "resourceName";
static const char*  RESOURCE_RECHARGE_LABEL  = "resourceRecharge";
static const char*  CONDITION_ICON_LABEL     = "conditionIcon";
static const char*  COMBATANT_ICON_LABEL     = "combatantIcon";

CombatantTemplateFrame::CombatantTemplateFrame(BattleDialogModelCombatant* combatant, bool showDone, const QString& templateFile, QWidget* parent) :
    CombatantWidget(parent),
    TemplateFrame(),
    _combatant(combatant),
    _adapter(nullptr),
    _base(nullptr),
    _uiWidget(nullptr),
    _showDone(showDone),
    _previousHitPoints(combatant ? combatant->getHitPoints() : 0)
{
    _adapter = new CombatantTemplateAdapter(_combatant, this);

    QVBoxLayout* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    _base = new CombatantWidgetBase(this);
    outerLayout->addWidget(_base);

    if(QFrame* host = _base->contentFrame())
    {
        QVBoxLayout* hostLayout = new QVBoxLayout(host);
        hostLayout->setContentsMargins(0, 0, 0, 0);
        hostLayout->setSpacing(0);
    }

    if(_combatant)
    {
        if(QCheckBox* chk = _base->visibleCheckbox())
        {
            chk->setChecked(_combatant->getShown());
            connect(chk, &QCheckBox::toggled, _combatant, &BattleDialogModelCombatant::setShown);
        }
        if(QCheckBox* chk = _base->knownCheckbox())
        {
            chk->setChecked(_combatant->getKnown());
            connect(chk, &QCheckBox::toggled, _combatant, &BattleDialogModelCombatant::setKnown);
        }
        if(QCheckBox* chk = _base->doneCheckbox())
        {
            chk->setChecked(_combatant->getDone());
            connect(chk, &QCheckBox::toggled, _combatant, &BattleDialogModelCombatant::setDone);
        }
    }

    if(!loadTemplate(templateFile))
        qDebug() << "[CombatantTemplateFrame] WARNING: Could not load template " << templateFile << " for combatant " << (combatant ? combatant->getName() : QStringLiteral("<null>"));

    connectModelSignals();
    setShowDone(_showDone);
}

CombatantTemplateFrame::~CombatantTemplateFrame()
{
}

BattleDialogModelCombatant* CombatantTemplateFrame::getCombatant()
{
    return _combatant;
}

int CombatantTemplateFrame::getInitiative() const
{
    return _combatant ? _combatant->getInitiative() : 0;
}

bool CombatantTemplateFrame::isShown()
{
    return _combatant ? _combatant->getShown() : true;
}

bool CombatantTemplateFrame::isKnown()
{
    return _combatant ? _combatant->getKnown() : true;
}

void CombatantTemplateFrame::setShowDone(bool showDone)
{
    _showDone = showDone;
    if(_base)
    {
        if(QCheckBox* chk = _base->doneCheckbox())
            chk->setVisible(_showDone);
        if(QLabel* lbl = _base->doneIconLabel())
            lbl->setVisible(_showDone);
    }
}

void CombatantTemplateFrame::disconnectInternals()
{
    if(_combatant)
        disconnect(_combatant, nullptr, this, nullptr);

    if(BattleDialogModelMonsterBase* mb = dynamic_cast<BattleDialogModelMonsterBase*>(_combatant))
        disconnect(mb, nullptr, this, nullptr);
    if(BattleDialogModelCharacter* cm = dynamic_cast<BattleDialogModelCharacter*>(_combatant))
        disconnect(cm, nullptr, this, nullptr);
}

void CombatantTemplateFrame::updateData()
{
    rebuildBindings();

    if(_combatant)
    {
        const int currentHp = _combatant->getHitPoints();
        if(currentHp != _previousHitPoints)
        {
            const int delta = _previousHitPoints - currentHp;
            _previousHitPoints = currentHp;
            emit hitPointsChanged(_combatant, delta);
        }
    }
}

void CombatantTemplateFrame::updateMove()
{
    rebuildBindings();
}

void CombatantTemplateFrame::selectCombatant()
{
    emitDoubleClickSignal();
}

void CombatantTemplateFrame::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    emitDoubleClickSignal();
}

QObject* CombatantTemplateFrame::getFrameObject()
{
    return this;
}

bool CombatantTemplateFrame::eventFilter(QObject* watched, QEvent* event)
{
    if(localEventFilter(watched, event))
        return true;
    return CombatantWidget::eventFilter(watched, event);
}

bool CombatantTemplateFrame::localEventFilter(QObject* object, QEvent* event)
{
    if(event && (event->type() == QEvent::ContextMenu) && _combatant)
    {
        QContextMenuEvent* menuEvent = static_cast<QContextMenuEvent*>(event);

        // Right-click directly on a condition icon frame
        if(QFrame* frame = qobject_cast<QFrame*>(object))
        {
            const QString conditionId = frame->property(CONDITION_PROPERTY_KEY).toString();
            if(!conditionId.isEmpty())
            {
                showConditionContextMenu(conditionId, menuEvent->globalPos());
                return true;
            }
        }

        // Right-click on the conditions scroll-area background → "Add condition"
        if(_conditionStrip && (object == _conditionStrip->viewport()))
        {
            showAddConditionMenu(menuEvent->globalPos());
            return true;
        }

        // Any other right-click on the combatant widget → emit context menu signal
        // so the BattleFrame can show the standard combatant menu.
        emit contextMenu(_combatant, menuEvent->globalPos());
        return true;
    }

    return TemplateFrame::localEventFilter(object, event);
}

void CombatantTemplateFrame::showConditionContextMenu(const QString& conditionId, const QPoint& globalPos)
{
    if((!_combatant) || conditionId.isEmpty())
        return;

    Conditions* conditions = Conditions::activeConditions();
    const QString title = (conditions ? conditions->getConditionTitle(conditionId) : conditionId);

    QMenu menu;
    QAction* removeAction = menu.addAction(tr("Remove %1").arg(title));
    menu.addSeparator();
    QAction* clearAction = menu.addAction(tr("Clear all conditions"));
    QAction* addAction = menu.addAction(tr("Add condition..."));

    QAction* chosen = menu.exec(globalPos);
    if(chosen == removeAction)
        _combatant->removeConditionId(conditionId);
    else if(chosen == clearAction)
        _combatant->clearConditions();
    else if(chosen == addAction)
        showAddConditionMenu(globalPos);
}

void CombatantTemplateFrame::showAddConditionMenu(const QPoint& globalPos)
{
    if(!_combatant)
        return;

    Conditions* conditions = Conditions::activeConditions();
    if(!conditions)
        return;

    const QList<ConditionDefinition> definitions = conditions->getConditions();
    if(definitions.isEmpty())
        return;

    QMenu menu;
    QHash<QAction*, QString> actionToId;
    for(const ConditionDefinition& def : definitions)
    {
        if(_combatant->hasConditionId(def.id))
            continue;
        const QPixmap icon = conditions->getConditionPixmap(def.id, CONDITION_ICON_SIZE);
        QAction* action = menu.addAction(QIcon(icon), def.title.isEmpty() ? def.id : def.title);
        action->setToolTip(def.description);
        actionToId.insert(action, def.id);
    }

    if(actionToId.isEmpty())
        return;

    QAction* chosen = menu.exec(globalPos);
    if(chosen && actionToId.contains(chosen))
        _combatant->addConditionId(actionToId.value(chosen));
}

void CombatantTemplateFrame::handleResourceCountChanged(BattleDialogModelMonsterBase* monster, const QString& name, int value)
{
    Q_UNUSED(monster);
    Q_UNUSED(name);
    Q_UNUSED(value);
    rebuildBindings();
}

void CombatantTemplateFrame::handleConditionsChanged(BattleDialogModelCombatant* combatant)
{
    Q_UNUSED(combatant);
    rebuildBindings();
}

void CombatantTemplateFrame::handleModelImageChanged(BattleDialogModelCombatant* combatant)
{
    Q_UNUSED(combatant);
    applyIcon();
    emit imageChanged(_combatant);
}

void CombatantTemplateFrame::handleMonsterImageChanged(BattleDialogModelMonsterBase* monster)
{
    Q_UNUSED(monster);
    applyIcon();
    emit imageChanged(_combatant);
}

void CombatantTemplateFrame::handleCharacterImageChanged(BattleDialogModelCharacter* character)
{
    Q_UNUSED(character);
    applyIcon();
    emit imageChanged(_combatant);
}

bool CombatantTemplateFrame::loadTemplate(const QString& templateFile)
{
    const QString absoluteTemplateFile = TemplateFactory::getAbsoluteTemplateFile(templateFile);
    if(absoluteTemplateFile.isEmpty())
        return false;

    QWidget* newWidget = TemplateFactory::loadUITemplate(absoluteTemplateFile);
    if(!newWidget)
        return false;

    delete _uiWidget;
    _uiWidget = newWidget;
    _uiFilename = absoluteTemplateFile;

    postLoadConfiguration(this, _uiWidget);

    QFrame* host = _base ? _base->contentFrame() : nullptr;
    QLayout* hostLayout = host ? host->layout() : nullptr;
    if(hostLayout)
        hostLayout->addWidget(_uiWidget);
    else if(host)
        _uiWidget->setParent(host);
    else
        _uiWidget->setParent(this);

    rebuildBindings();
    return true;
}

void CombatantTemplateFrame::rebuildBindings()
{
    if((!_uiWidget) || (!_adapter) || (!_combatant))
        return;

    TemplateObject* inner = _adapter->getInner();
    if(!inner || !inner->getFactory())
    {
        qDebug() << "[CombatantTemplateFrame] WARNING: combatant " << _combatant->getName() << " has no inner template object/factory; skipping bindings";
        return;
    }

    inner->getFactory()->readObjectData(_uiWidget, _adapter, this, this);

    applyIcon();
    applyConditionDecorations();
    applyResourceDecorations();
}

void CombatantTemplateFrame::applyIcon()
{
    if(!_combatant)
        return;

    QLabel* iconLabel = _base ? _base->iconLabel() : nullptr;
    if((!iconLabel) && _uiWidget)
        iconLabel = _uiWidget->findChild<QLabel*>(QString::fromLatin1(COMBATANT_ICON_LABEL));
    if(!iconLabel)
        return;

    const QPixmap pixmap = _combatant->getIconPixmap(DMHelper::PixmapSize_Battle);
    if(pixmap.isNull())
        return;

    const qreal dpr = iconLabel->devicePixelRatioF();
    const QSize labelSize = iconLabel->size();
    if((labelSize.width() <= 0) || (labelSize.height() <= 0))
        return;

    const QSize targetPixelSize = labelSize * dpr;
    QPixmap scaled = pixmap.scaled(targetPixelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaled.setDevicePixelRatio(dpr);
    iconLabel->setPixmap(scaled);
}

void CombatantTemplateFrame::applyConditionDecorations()
{
    if(!_uiWidget)
        return;

    QScrollArea* scrollArea = findScrollArea(QString::fromLatin1(CombatantTemplateAdapter::KEY_CONDITIONS));
    if((!scrollArea) || (!scrollArea->widget()))
        return;

    if(_conditionStrip != scrollArea)
    {
        _conditionStrip = scrollArea;
        scrollArea->viewport()->installEventFilter(this);
    }

    QList<QFrame*> conditionFrames = scrollArea->widget()->findChildren<QFrame*>(QString(), Qt::FindDirectChildrenOnly);
    Conditions* conditions = Conditions::activeConditions();
    for(QFrame* frame : conditionFrames)
    {
        if(!frame)
            continue;

        QLineEdit* edit = frame->findChild<QLineEdit*>();
        QString conditionId = edit ? edit->text() : QString();
        if(conditionId.isEmpty())
            conditionId = frame->property(CONDITION_PROPERTY_KEY).toString();
        else
            frame->setProperty(CONDITION_PROPERTY_KEY, conditionId);

        if(edit)
            edit->hide();

        QLabel* iconLabel = frame->findChild<QLabel*>(QString::fromLatin1(CONDITION_ICON_LABEL));
        if((!iconLabel) || (!conditions))
            continue;

        const QPixmap iconPixmap = conditions->getConditionPixmap(conditionId, CONDITION_ICON_SIZE);
        iconLabel->setPixmap(iconPixmap);
        iconLabel->setToolTip(conditions->getConditionTitle(conditionId) + QStringLiteral("\n") + conditions->getConditionDescription(conditionId));

        frame->setContextMenuPolicy(Qt::DefaultContextMenu);
        frame->installEventFilter(this);
    }
}

void CombatantTemplateFrame::applyResourceDecorations()
{
    if(!_uiWidget)
        return;

    QScrollArea* scrollArea = findScrollArea(QString::fromLatin1(CombatantTemplateAdapter::KEY_PER_ROUND_RESOURCES));
    if((!scrollArea) || (!scrollArea->widget()))
        return;

    QList<QFrame*> resourceFrames = scrollArea->widget()->findChildren<QFrame*>(QString(), Qt::FindDirectChildrenOnly);
    BattleDialogModelMonsterBase* mb = dynamic_cast<BattleDialogModelMonsterBase*>(_combatant);
    MonsterClassv2* mc = mb ? mb->getMonsterClass() : nullptr;
    if(!mc)
        return;

    const QList<PerRoundResource> resources = mc->getPerRoundResources();
    int index = 0;
    for(QFrame* frame : resourceFrames)
    {
        if(!frame)
            continue;

        if(index >= resources.size())
        {
            ++index;
            continue;
        }

        const PerRoundResource& res = resources.at(index);

        QLabel* nameLabel = frame->findChild<QLabel*>(QString::fromLatin1(RESOURCE_NAME_LABEL_NAME));
        if(nameLabel)
            nameLabel->setText(res.name);

        QLabel* rechargeLabel = frame->findChild<QLabel*>(QString::fromLatin1(RESOURCE_RECHARGE_LABEL));
        if(rechargeLabel)
            rechargeLabel->setText(res.recharge);

        frame->setProperty(RESOURCE_PROPERTY_KEY, res.name);
        wireResourceButtons(frame, index);
        ++index;
    }
}

void CombatantTemplateFrame::wireResourceButtons(QWidget* resourceWidget, int index)
{
    if((!resourceWidget) || (!_combatant))
        return;

    BattleDialogModelMonsterBase* mb = dynamic_cast<BattleDialogModelMonsterBase*>(_combatant);
    if(!mb)
        return;

    const QString resourceName = resourceWidget->property(RESOURCE_PROPERTY_KEY).toString();
    if(resourceName.isEmpty())
        return;

    QPushButton* decrementBtn = resourceWidget->findChild<QPushButton*>(QString::fromLatin1(RESOURCE_DECREMENT_NAME));
    if(decrementBtn)
    {
        decrementBtn->disconnect();
        connect(decrementBtn, &QPushButton::clicked, this, [mb, resourceName]() {
            const int current = mb->getResourceCount(resourceName);
            if(current > 0)
                mb->setResourceCount(resourceName, current - 1);
        });
    }

    QPushButton* resetBtn = resourceWidget->findChild<QPushButton*>(QString::fromLatin1(RESOURCE_RESET_NAME));
    if(resetBtn)
    {
        MonsterClassv2* mc = mb->getMonsterClass();
        const int maxValue = (mc && (index >= 0) && (index < mc->getPerRoundResources().size())) ? mc->getPerRoundResources().at(index).max : 0;
        resetBtn->disconnect();
        connect(resetBtn, &QPushButton::clicked, this, [mb, resourceName, maxValue]() {
            mb->setResourceCount(resourceName, maxValue);
        });
    }
}

QScrollArea* CombatantTemplateFrame::findScrollArea(const QString& dmhValueKey) const
{
    if(!_uiWidget)
        return nullptr;

    QList<QScrollArea*> areas = _uiWidget->findChildren<QScrollArea*>();
    for(QScrollArea* area : areas)
    {
        if(area && (area->property(TemplateFactory::TEMPLATE_PROPERTY).toString() == dmhValueKey))
            return area;
    }
    return nullptr;
}

void CombatantTemplateFrame::connectModelSignals()
{
    if(!_combatant)
        return;

    connect(_combatant, &BattleDialogModelCombatant::initiativeChanged,    this, &CombatantTemplateFrame::updateData);
    connect(_combatant, &BattleDialogModelCombatant::conditionsChanged,    this, &CombatantTemplateFrame::handleConditionsChanged);
    connect(_combatant, &BattleDialogModelCombatant::moveUpdated,          this, &CombatantTemplateFrame::updateMove);
    connect(_combatant, &BattleDialogModelCombatant::combatantDoneChanged, this, &CombatantTemplateFrame::updateData);

    // Sync the base shell's checkboxes back from the model when the model
    // changes via another path (e.g. context menu, scripting).
    connect(_combatant, &BattleDialogModelCombatant::visibilityChanged, this, [this]() {
        if(!_base)
            return;
        if(QCheckBox* chk = _base->visibleCheckbox())
        {
            QSignalBlocker block(chk);
            chk->setChecked(_combatant->getShown());
        }
        if(QCheckBox* chk = _base->knownCheckbox())
        {
            QSignalBlocker block(chk);
            chk->setChecked(_combatant->getKnown());
        }
    });
    connect(_combatant, &BattleDialogModelCombatant::combatantDoneChanged, this, [this]() {
        if(!_base)
            return;
        if(QCheckBox* chk = _base->doneCheckbox())
        {
            QSignalBlocker block(chk);
            chk->setChecked(_combatant->getDone());
        }
    });

    if(BattleDialogModelMonsterBase* mb = dynamic_cast<BattleDialogModelMonsterBase*>(_combatant))
    {
        connect(mb, &BattleDialogModelMonsterBase::dataChanged,           this, &CombatantTemplateFrame::updateData);
        connect(mb, &BattleDialogModelMonsterBase::imageChanged,          this, &CombatantTemplateFrame::handleMonsterImageChanged);
        connect(mb, &BattleDialogModelMonsterBase::resourceCountChanged,  this, &CombatantTemplateFrame::handleResourceCountChanged);
    }
    else if(BattleDialogModelCharacter* cm = dynamic_cast<BattleDialogModelCharacter*>(_combatant))
    {
        connect(cm, &BattleDialogModelCharacter::imageChanged, this, &CombatantTemplateFrame::handleCharacterImageChanged);
    }
}

void CombatantTemplateFrame::emitDoubleClickSignal()
{
    if(!_combatant)
        return;

    if(BattleDialogModelMonsterBase* mb = dynamic_cast<BattleDialogModelMonsterBase*>(_combatant))
    {
        if(MonsterClassv2* mc = mb->getMonsterClass())
            emit clicked(mc->getStringValue(QStringLiteral("name")));
    }
    else if(BattleDialogModelCharacter* cm = dynamic_cast<BattleDialogModelCharacter*>(_combatant))
    {
        if(Characterv2* c = cm->getCharacter())
            emit clickedCharacter(c->getID());
    }
}
