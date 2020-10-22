#include "battlecombatantframe.h"
#include "ui_battlecombatantframe.h"
#include "battledialogmodelcombatant.h"
#include "character.h"
#include "conditionseditdialog.h"
#include <QDebug>

const int CONDITION_FRAME_SPACING = 8;

BattleCombatantFrame::BattleCombatantFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::BattleCombatantFrame),
    _combatant(nullptr),
    _conditionGrid(nullptr)
{
    ui->setupUi(this);

    ui->edtName->setEnabled(false);
    ui->frameInfoContents->setEnabled(false);
    ui->frameStatsContents->setEnabled(false);

    connect(ui->btnEditConditions, &QAbstractButton::clicked, this, &BattleCombatantFrame::editConditions);
}

BattleCombatantFrame::BattleCombatantFrame(BattleDialogModelCombatant* combatant, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::BattleCombatantFrame),
    _combatant(nullptr),
    _conditionGrid(nullptr)
{
    ui->setupUi(this);

    ui->edtName->setEnabled(false);
    ui->frameInfoContents->setEnabled(false);
    ui->frameStatsContents->setEnabled(false);

    connect(ui->btnEditConditions, &QAbstractButton::clicked, this, &BattleCombatantFrame::editConditions);

    setCombatant(combatant);
}

BattleCombatantFrame::~BattleCombatantFrame()
{
    delete ui;
}

void BattleCombatantFrame::setCombatant(BattleDialogModelCombatant* combatant)
{
    qDebug() << "[BattleCombatantFrame] Reading combatant: " << combatant;

    disconnect(_combatant, &BattleDialogModelCombatant::campaignObjectDestroyed, this, &BattleCombatantFrame::clearCombatant);
    clearGrid();

    ui->edtName->setEnabled(combatant != nullptr);
    ui->frameInfoContents->setEnabled(combatant != nullptr);
    ui->frameStatsContents->setEnabled(combatant != nullptr);

    _combatant = combatant;

    if(!combatant)
    {
        ui->edtName->setText(QString());
        return;
    }

    connect(_combatant, &BattleDialogModelCombatant::campaignObjectDestroyed, this, &BattleCombatantFrame::clearCombatant);

    ui->edtName->setText(combatant->getName());
    updateLayout();

    ui->edtStr->setText(Combatant::convertModToStr(combatant->getSkillModifier(Combatant::Skills_strengthSave)));
    ui->edtDex->setText(Combatant::convertModToStr(combatant->getSkillModifier(Combatant::Skills_dexteritySave)));
    ui->edtCon->setText(Combatant::convertModToStr(combatant->getSkillModifier(Combatant::Skills_constitutionSave)));
    ui->edtInt->setText(Combatant::convertModToStr(combatant->getSkillModifier(Combatant::Skills_intelligenceSave)));
    ui->edtWis->setText(Combatant::convertModToStr(combatant->getSkillModifier(Combatant::Skills_wisdomSave)));
    ui->edtCha->setText(Combatant::convertModToStr(combatant->getSkillModifier(Combatant::Skills_charismaSave)));
}

BattleDialogModelCombatant* BattleCombatantFrame::getCombatant() const
{
    return _combatant;
}

void BattleCombatantFrame::clearCombatant()
{
    setCombatant(nullptr);
}

void BattleCombatantFrame::editConditions()
{
    if(!_combatant)
        return;

    ConditionsEditDialog dlg;
    dlg.setConditions(_combatant->getConditions());
    int result = dlg.exec();
    if(result == QDialog::Accepted)
    {
        _combatant->setConditions(dlg.getConditions());
        updateLayout();
    }
}

void BattleCombatantFrame::updateLayout()
{
    clearGrid();

    if(!_combatant)
        return;

    qDebug() << "[BattleCombatantFrame] Creating a new condition grid";

    _conditionGrid = new QGridLayout;
    _conditionGrid->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    _conditionGrid->setContentsMargins(CONDITION_FRAME_SPACING, CONDITION_FRAME_SPACING, CONDITION_FRAME_SPACING, CONDITION_FRAME_SPACING);
    _conditionGrid->setSpacing(CONDITION_FRAME_SPACING);
    ui->scrollAreaWidgetContents->setLayout(_conditionGrid);

    int conditions = _combatant->getConditions();

    qDebug() << "[BattleCombatantFrame] Adding conditions: " << conditions;

    for(int i = 0; i < Combatant::getConditionCount(); ++i)
    {
        Combatant::Condition condition = Combatant::getConditionByIndex(i);
        if(conditions & condition)
            addCondition(condition);
    }

    /*
    addCondition(Combatant::Condition_Blinded, conditions);
    addCondition(Combatant::Condition_Charmed, conditions);
    addCondition(Combatant::Condition_Deafened, conditions);
    addCondition(Combatant::Condition_Exhaustion_1, conditions);
    addCondition(Combatant::Condition_Exhaustion_2, conditions);
    addCondition(Combatant::Condition_Exhaustion_3, conditions);
    addCondition(Combatant::Condition_Exhaustion_4, conditions);
    addCondition(Combatant::Condition_Exhaustion_5, conditions);
    addCondition(Combatant::Condition_Frightened, conditions);
    addCondition(Combatant::Condition_Grappled, conditions);
    addCondition(Combatant::Condition_Incapacitated, conditions);
    addCondition(Combatant::Condition_Invisible, conditions);
    addCondition(Combatant::Condition_Paralyzed, conditions);
    addCondition(Combatant::Condition_Petrified, conditions);
    addCondition(Combatant::Condition_Poisoned, conditions);
    addCondition(Combatant::Condition_Prone, conditions);
    addCondition(Combatant::Condition_Restrained, conditions);
    addCondition(Combatant::Condition_Stunned, conditions);
    addCondition(Combatant::Condition_Unconscious, conditions);
    */

    qDebug() << "[BattleCombatantFrame] Total grid entries created: " << _conditionGrid->count();

    int spacingColumn = _conditionGrid->columnCount();

    _conditionGrid->addItem(new QSpacerItem(20, 40, QSizePolicy::Expanding), 0, spacingColumn);

    for(int i = 0; i < spacingColumn; ++i)
        _conditionGrid->setColumnStretch(i, 1);

    _conditionGrid->setColumnStretch(spacingColumn, 10);

    update();
}

void BattleCombatantFrame::clearGrid()
{
    if(!_conditionGrid)
        return;

//    _conditionGrid->invalidate();

    qDebug() << "[BattleCombatantFrame] Clearing the condition grid";

    // Delete the grid entries
    QLayoutItem *child = nullptr;
    while((child = _conditionGrid->takeAt(0)) != nullptr)
    {
        delete child->widget();
        delete child;
    }

    delete _conditionGrid;
    _conditionGrid = nullptr;

    ui->scrollAreaWidgetContents->update();
}

void BattleCombatantFrame::addCondition(Combatant::Condition condition)
{
    QString resourceIcon = QString(":/img/data/img/") + Combatant::getConditionIcon(condition) + QString(".png");
    QLabel* conditionLabel = new QLabel(this);
    conditionLabel->setPixmap(QPixmap(resourceIcon).scaled(40, 40));
    conditionLabel->setToolTip(Combatant::getConditionDescription(condition));

    int columnCount = (ui->scrollAreaWidgetContents->width() - CONDITION_FRAME_SPACING) / (40 + CONDITION_FRAME_SPACING);
    int row = _conditionGrid->count() / columnCount;
    int column = _conditionGrid->count() % columnCount;

    _conditionGrid->addWidget(conditionLabel, row, column);

}

void BattleCombatantFrame::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    updateLayout();
}
