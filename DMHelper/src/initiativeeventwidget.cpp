#include "initiativeeventwidget.h"
#include "ui_initiativeeventwidget.h"
#include "battledialogmodelinitiativeevent.h"
#include <QIntValidator>
#include <QContextMenuEvent>
#include <QEvent>

InitiativeEventWidget::InitiativeEventWidget(BattleDialogModelInitiativeEvent* event, QWidget* parent) :
    CombatantWidget(parent),
    ui(new Ui::InitiativeEventWidget),
    _event(event)
{
    ui->setupUi(this);

    if(_event)
    {
        ui->edtName->setText(_event->getName());
        ui->edtInitiative->setText(QString::number(_event->getInitiative()));
        ui->chkActive->setChecked(_event->getKnown());
    }

    ui->edtInitiative->setValidator(new QIntValidator(-99, 99, this));

    connect(ui->chkActive, &QCheckBox::toggled, this, &InitiativeEventWidget::handleActiveToggled);
    connect(ui->edtInitiative, &QLineEdit::editingFinished, this, &InitiativeEventWidget::handleInitiativeEdited);
    connect(ui->edtName, &QLineEdit::editingFinished, this, &InitiativeEventWidget::handleNameEdited);

    // Route right-clicks from child editors/checkbox to the battle row menu.
    ui->chkActive->installEventFilter(this);
    ui->edtInitiative->installEventFilter(this);
    ui->edtName->installEventFilter(this);

    if(_event)
        connect(_event, &BattleDialogModelCombatant::initiativeChanged, this, &InitiativeEventWidget::handleCombatantInitiativeChanged);
}

InitiativeEventWidget::~InitiativeEventWidget()
{
    delete ui;
}

BattleDialogModelCombatant* InitiativeEventWidget::getCombatant()
{
    return _event;
}

int InitiativeEventWidget::getInitiative() const
{
    return ui->edtInitiative->text().toInt();
}

bool InitiativeEventWidget::isShown()
{
    return _event ? _event->getKnown() : true;
}

bool InitiativeEventWidget::isKnown()
{
    return _event ? _event->getKnown() : true;
}

void InitiativeEventWidget::setShowDone(bool showDone)
{
    Q_UNUSED(showDone);
}

void InitiativeEventWidget::disconnectInternals()
{
    if(_event)
        disconnect(_event, nullptr, this, nullptr);
}

void InitiativeEventWidget::setInitiative(int initiative)
{
    ui->edtInitiative->setText(QString::number(initiative));
    update();
}

void InitiativeEventWidget::selectCombatant()
{
}

void InitiativeEventWidget::handleActiveToggled(bool checked)
{
    if(_event)
        _event->setKnown(checked);
    emit isKnownChanged(checked);
}

void InitiativeEventWidget::handleInitiativeEdited()
{
    if(_event)
        _event->setInitiative(ui->edtInitiative->text().toInt());
}

void InitiativeEventWidget::handleNameEdited()
{
    if(_event)
        _event->setName(ui->edtName->text());
}

void InitiativeEventWidget::handleCombatantInitiativeChanged()
{
    if(_event)
        ui->edtInitiative->setText(QString::number(_event->getInitiative()));
}

void InitiativeEventWidget::contextMenuEvent(QContextMenuEvent* event)
{
    if((event) && (_event))
    {
        emit contextMenu(_event, event->globalPos());
        event->accept();
        return;
    }

    CombatantWidget::contextMenuEvent(event);
}

bool InitiativeEventWidget::eventFilter(QObject* watched, QEvent* event)
{
    if((event) && (event->type() == QEvent::ContextMenu) && (_event) &&
       ((watched == ui->chkActive) || (watched == ui->edtInitiative) || (watched == ui->edtName)))
    {
        QContextMenuEvent* menuEvent = static_cast<QContextMenuEvent*>(event);
        emit contextMenu(_event, menuEvent->globalPos());
        return true;
    }

    return CombatantWidget::eventFilter(watched, event);
}
