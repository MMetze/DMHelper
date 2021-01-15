#include "conditionseditdialog.h"
#include "ui_conditionseditdialog.h"
#include "ribbonframe.h"
#include "combatant.h"
#include "quickref.h"
#include <QPainter>

ConditionsEditDialog::ConditionsEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConditionsEditDialog)
{
    ui->setupUi(this);
    ui->frame->setEnabled(false);

    setConditionTooltip(*ui->btnBlinded, Combatant::Condition_Blinded);
    setConditionTooltip(*ui->btnCharmed, Combatant::Condition_Charmed);
    setConditionTooltip(*ui->btnDeafened, Combatant::Condition_Deafened);
    setConditionTooltip(*ui->btnFrightened, Combatant::Condition_Frightened);
    setConditionTooltip(*ui->btnGrappled, Combatant::Condition_Grappled);
    setConditionTooltip(*ui->btnIncapacitated, Combatant::Condition_Incapacitated);
    setConditionTooltip(*ui->btnInvisible, Combatant::Condition_Invisible);
    setConditionTooltip(*ui->btnParalyzed, Combatant::Condition_Paralyzed);
    setConditionTooltip(*ui->btnPetrified, Combatant::Condition_Petrified);
    setConditionTooltip(*ui->btnPoisoned, Combatant::Condition_Poisoned);
    setConditionTooltip(*ui->btnProne, Combatant::Condition_Prone);
    setConditionTooltip(*ui->btnRestrained, Combatant::Condition_Restrained);
    setConditionTooltip(*ui->btnStunned, Combatant::Condition_Stunned);
    setConditionTooltip(*ui->btnUnconscious, Combatant::Condition_Unconscious);
    setConditionTooltip(*ui->btnExhaustion, Combatant::Condition_Exhaustion_1);

    connect(ui->btnExhaustion, &QAbstractButton::toggled, this, &ConditionsEditDialog::setExhausted);
}

ConditionsEditDialog::~ConditionsEditDialog()
{
    delete ui;
}

void ConditionsEditDialog::setConditions(int conditions)
{
    ui->btnBlinded->setChecked(conditions & Combatant::Condition_Blinded);
    ui->btnCharmed->setChecked(conditions & Combatant::Condition_Charmed);
    ui->btnDeafened->setChecked(conditions & Combatant::Condition_Deafened);
    ui->btnFrightened->setChecked(conditions & Combatant::Condition_Frightened);
    ui->btnGrappled->setChecked(conditions & Combatant::Condition_Grappled);
    ui->btnIncapacitated->setChecked(conditions & Combatant::Condition_Incapacitated);
    ui->btnInvisible->setChecked(conditions & Combatant::Condition_Invisible);
    ui->btnParalyzed->setChecked(conditions & Combatant::Condition_Paralyzed);
    ui->btnPetrified->setChecked(conditions & Combatant::Condition_Petrified);
    ui->btnPoisoned->setChecked(conditions & Combatant::Condition_Poisoned);
    ui->btnProne->setChecked(conditions & Combatant::Condition_Prone);
    ui->btnRestrained->setChecked(conditions & Combatant::Condition_Restrained);
    ui->btnStunned->setChecked(conditions & Combatant::Condition_Stunned);
    ui->btnUnconscious->setChecked(conditions & Combatant::Condition_Unconscious);

    bool exhausted = conditions & (Combatant::Condition_Exhaustion_1 |
                                   Combatant::Condition_Exhaustion_2 |
                                   Combatant::Condition_Exhaustion_3 |
                                   Combatant::Condition_Exhaustion_4 |
                                   Combatant::Condition_Exhaustion_5);
    ui->btnExhaustion->setChecked(exhausted);
    if(exhausted)
    {
        ui->btnExhaustion1->setChecked(conditions & Combatant::Condition_Exhaustion_1);
        ui->btnExhaustion2->setChecked(conditions & Combatant::Condition_Exhaustion_2);
        ui->btnExhaustion3->setChecked(conditions & Combatant::Condition_Exhaustion_3);
        ui->btnExhaustion4->setChecked(conditions & Combatant::Condition_Exhaustion_4);
        ui->btnExhaustion5->setChecked(conditions & Combatant::Condition_Exhaustion_5);
    }
}

int ConditionsEditDialog::getConditions() const
{
    int conditions = 0;

    conditions |= ui->btnBlinded->isChecked() ? Combatant::Condition_Blinded : 0;
    conditions |= ui->btnCharmed->isChecked() ? Combatant::Condition_Charmed : 0;
    conditions |= ui->btnDeafened->isChecked() ? Combatant::Condition_Deafened : 0;
    conditions |= ui->btnFrightened->isChecked() ? Combatant::Condition_Frightened : 0;
    conditions |= ui->btnGrappled->isChecked() ? Combatant::Condition_Grappled : 0;
    conditions |= ui->btnIncapacitated->isChecked() ? Combatant::Condition_Incapacitated : 0;
    conditions |= ui->btnInvisible->isChecked() ? Combatant::Condition_Invisible : 0;
    conditions |= ui->btnParalyzed->isChecked() ? Combatant::Condition_Paralyzed : 0;
    conditions |= ui->btnPetrified->isChecked() ? Combatant::Condition_Petrified : 0;
    conditions |= ui->btnPoisoned->isChecked() ? Combatant::Condition_Poisoned : 0;
    conditions |= ui->btnProne->isChecked() ? Combatant::Condition_Prone : 0;
    conditions |= ui->btnRestrained->isChecked() ? Combatant::Condition_Restrained : 0;
    conditions |= ui->btnStunned->isChecked() ? Combatant::Condition_Stunned : 0;
    conditions |= ui->btnUnconscious->isChecked() ? Combatant::Condition_Unconscious : 0;

    if(ui->btnExhaustion->isChecked())
    {
        conditions |= ui->btnExhaustion1->isChecked() ? Combatant::Condition_Exhaustion_1 : 0;
        conditions |= ui->btnExhaustion2->isChecked() ? Combatant::Condition_Exhaustion_2 : 0;
        conditions |= ui->btnExhaustion3->isChecked() ? Combatant::Condition_Exhaustion_3 : 0;
        conditions |= ui->btnExhaustion4->isChecked() ? Combatant::Condition_Exhaustion_4 : 0;
        conditions |= ui->btnExhaustion5->isChecked() ? Combatant::Condition_Exhaustion_5 : 0;
    }

    return conditions;
}

void ConditionsEditDialog::setExhausted(bool exhausted)
{
    ui->frame->setEnabled(exhausted);
}

void ConditionsEditDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    int ribbonHeight = RibbonFrame::getRibbonHeight();
    QFontMetrics metrics = ui->lblIncapacitated->fontMetrics();
    int buttonWidth = metrics.horizontalAdvance(ui->lblIncapacitated->text());

    setButtonSize(*ui->lblBlinded, *ui->btnBlinded, ribbonHeight, buttonWidth);
    setButtonSize(*ui->lblCharmed, *ui->btnCharmed, ribbonHeight, buttonWidth);
    setButtonSize(*ui->lblDeafened, *ui->btnDeafened, ribbonHeight, buttonWidth);
    setButtonSize(*ui->lblFrightened, *ui->btnFrightened, ribbonHeight, buttonWidth);
    setButtonSize(*ui->lblExhaustion, *ui->btnExhaustion, ribbonHeight, buttonWidth);
    setButtonSize(*ui->lblGrappled, *ui->btnGrappled, ribbonHeight, buttonWidth);
    setButtonSize(*ui->lblIncapacitated, *ui->btnIncapacitated, ribbonHeight, buttonWidth);
    setButtonSize(*ui->lblInvisible, *ui->btnInvisible, ribbonHeight, buttonWidth);
    setButtonSize(*ui->lblParalyzed, *ui->btnParalyzed, ribbonHeight, buttonWidth);
    setButtonSize(*ui->lblPetrified, *ui->btnPetrified, ribbonHeight, buttonWidth);
    setButtonSize(*ui->lblPoisoned, *ui->btnPoisoned, ribbonHeight, buttonWidth);
    setButtonSize(*ui->lblProne, *ui->btnProne, ribbonHeight, buttonWidth);
    setButtonSize(*ui->lblRestrained, *ui->btnRestrained, ribbonHeight, buttonWidth);
    setButtonSize(*ui->lblStunned, *ui->btnStunned, ribbonHeight, buttonWidth);
    setButtonSize(*ui->lblUnconscious, *ui->btnUnconscious, ribbonHeight, buttonWidth);
}

void ConditionsEditDialog::setButtonSize(QLabel& label, QPushButton& button, int frameHeight, int buttonWidth)
{
    QFontMetrics metrics = label.fontMetrics();
    int labelHeight = RibbonFrame::getLabelHeight(metrics, frameHeight);
    int iconDim = frameHeight - labelHeight;

    RibbonFrame::setWidgetSize(label, buttonWidth, labelHeight);
    RibbonFrame::setButtonSize(button, buttonWidth, iconDim);
}

void ConditionsEditDialog::setConditionTooltip(QPushButton& button, Combatant::Condition condition)
{
    QString conditionText = QString("<b>") + Combatant::getConditionTitle(condition) + QString("</b>");
    if(QuickRef::Instance())
    {
        QuickRefData* conditionData = QuickRef::Instance()->getData(QString("Condition"), 0, Combatant::getConditionTitle(condition));
        if(conditionData)
            conditionText += QString("<p>") + conditionData->getOverview();
    }

    button.setToolTip(conditionText);
}

