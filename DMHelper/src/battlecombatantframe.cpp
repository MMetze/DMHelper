#include "battlecombatantframe.h"
#include "ui_battlecombatantframe.h"
#include "battledialogmodelcombatant.h"
#include "character.h"
#include "conditionseditdialog.h"
#include "quickref.h"
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

    if(_combatant)
    {
        disconnect(_combatant, &BattleDialogModelCombatant::campaignObjectDestroyed, this, &BattleCombatantFrame::clearCombatant);
        if(_combatant->getCombatant())
            disconnect(_combatant->getCombatant(), &Combatant::dirty, this, &BattleCombatantFrame::readCombatant);
    }

    ui->edtName->setEnabled(combatant != nullptr);
    ui->frameInfoContents->setEnabled(combatant != nullptr);
    ui->frameStatsContents->setEnabled(combatant != nullptr);

    if(!combatant)
    {
        ui->edtName->setText(QString());
        clearGrid();
        ui->edtStr->setText(QString());
        ui->edtDex->setText(QString());
        ui->edtCon->setText(QString());
        ui->edtInt->setText(QString());
        ui->edtWis->setText(QString());
        ui->edtCha->setText(QString());
        return;
    }

    _combatant = combatant;

    connect(_combatant, &BattleDialogModelCombatant::campaignObjectDestroyed, this, &BattleCombatantFrame::clearCombatant);
    if(_combatant->getCombatant())
        connect(_combatant->getCombatant(), &Combatant::dirty, this, &BattleCombatantFrame::readCombatant);

    readCombatant();
}

BattleDialogModelCombatant* BattleCombatantFrame::getCombatant() const
{
    return _combatant;
}

void BattleCombatantFrame::readCombatant()
{
    if(!_combatant)
        return;

    ui->edtName->setText(_combatant->getName());
    updateLayout();

    ui->edtStr->setText(Combatant::convertModToStr(_combatant->getSkillModifier(Combatant::Skills_strengthSave)));
    ui->edtDex->setText(Combatant::convertModToStr(_combatant->getSkillModifier(Combatant::Skills_dexteritySave)));
    ui->edtCon->setText(Combatant::convertModToStr(_combatant->getSkillModifier(Combatant::Skills_constitutionSave)));
    ui->edtInt->setText(Combatant::convertModToStr(_combatant->getSkillModifier(Combatant::Skills_intelligenceSave)));
    ui->edtWis->setText(Combatant::convertModToStr(_combatant->getSkillModifier(Combatant::Skills_wisdomSave)));
    ui->edtCha->setText(Combatant::convertModToStr(_combatant->getSkillModifier(Combatant::Skills_charismaSave)));
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
        if(dlg.getConditions() != _combatant->getConditions())
        {
            _combatant->setConditions(dlg.getConditions());
            updateLayout();
            emit conditionsChanged(_combatant);
        }
    }
}

void BattleCombatantFrame::updateLayout()
{
    clearGrid();

    if(!_combatant)
        return;

    _conditionGrid = new QGridLayout;
    _conditionGrid->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    _conditionGrid->setContentsMargins(CONDITION_FRAME_SPACING, CONDITION_FRAME_SPACING, CONDITION_FRAME_SPACING, CONDITION_FRAME_SPACING);
    _conditionGrid->setSpacing(CONDITION_FRAME_SPACING);
    ui->scrollAreaWidgetContents->setLayout(_conditionGrid);

    int conditions = _combatant->getConditions();

    for(int i = 0; i < Combatant::getConditionCount(); ++i)
    {
        Combatant::Condition condition = Combatant::getConditionByIndex(i);
        if(conditions & condition)
            addCondition(condition);
    }

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
    if(!_conditionGrid)
        return;

    QString resourceIcon = QString(":/img/data/img/") + Combatant::getConditionIcon(condition) + QString(".png");
    QLabel* conditionLabel = new QLabel(this);
    conditionLabel->setPixmap(QPixmap(resourceIcon).scaled(40, 40));

    QString conditionText = QString("<b>") + Combatant::getConditionDescription(condition) + QString("</b>");
    if(QuickRef::Instance())
    {
        QuickRefData* conditionData = QuickRef::Instance()->getData(QString("Condition"), 0, Combatant::getConditionTitle(condition));
        if(conditionData)
            conditionText += QString("<p>") + conditionData->getOverview();
    }
    conditionLabel->setToolTip(conditionText);

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
