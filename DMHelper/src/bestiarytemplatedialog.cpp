#include "bestiarytemplatedialog.h"
#include "ui_bestiarytemplatedialog.h"
#include "monsterclassv2.h"
#include "monsterfactory.h"
#include <QMouseEvent>
#include <QTextEdit>
#include <QMenu>
#include <QDebug>

BestiaryTemplateDialog::BestiaryTemplateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BestiaryTemplateDialog),
    _uiWidget(nullptr),
    _monster(nullptr)
{
    ui->setupUi(this);
}

BestiaryTemplateDialog::~BestiaryTemplateDialog()
{
    delete ui;
}

void BestiaryTemplateDialog::loadMonsterUITemplate(const QString& templateFile)
{
    QWidget* newWidget = TemplateFactory::loadUITemplate(templateFile);
    if(!newWidget)
    {
        qDebug() << "[CharacterTemplateFrame] ERROR: UI Template File " << templateFile << " could not be loaded!";
        return;
    }

    delete _uiWidget;
    if(ui->scrollAreaWidgetContents->layout())
        delete ui->scrollAreaWidgetContents->layout();

    _uiWidget = newWidget;

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    layout->addWidget(_uiWidget);
    ui->scrollAreaWidgetContents->setLayout(layout);
}

MonsterClassv2* BestiaryTemplateDialog::getMonster() const
{
    return _monster;
}

void BestiaryTemplateDialog::setOptions(OptionsContainer* options)
{
    if(!options)
        return;

    _options = options;
}

void BestiaryTemplateDialog::setMonster(MonsterClassv2* monster, bool edit)
{
    if((!monster) || (_monster == monster))
        return;

    qDebug() << "[BestiaryTemplateDialog] Set Monster to " << monster->getStringValue("name");

    if(_monster && _edit)
        storeMonsterData();

    _monster = monster;
    _edit = edit;

    MonsterFactory::Instance()->readObjectData(_uiWidget, _monster, this);

    if(ui->cmbSearch->currentText() != _monster->getStringValue("name"))
        ui->cmbSearch->setCurrentText(_monster->getStringValue("name"));

    if(_monster->getIconCount() == 0)
        _monster->searchForIcons();
    setTokenIndex(0);

    emit monsterChanged();

    /*
    ui->chkPrivate->setChecked(_monster->getPrivate());
    ui->chkLegendary->setChecked(_monster->getLegendary());
    ui->edtName->setText(_monster->getName());
    ui->edtMonsterSize->setText(_monster->getMonsterSize());
    ui->edtMonsterType->setText(_monster->getMonsterType());
    ui->edtMonsterSubType->setText(_monster->getMonsterSubType());
    ui->edtAlignment->setText(_monster->getAlignment());
    ui->edtArmorClass->setText(QString::number(_monster->getArmorClass()));
    ui->edtHitDice->setText(_monster->getHitDice().toString());
    ui->edtAverageHitPoints->setText(QString::number(_monster->getAverageHitPoints()));
    ui->edtSpeed->setText(_monster->getSpeed());
    ui->edtSize->setText(_monster->getMonsterSize());
    ui->edtStrength->setText(QString::number(_monster->getStrength()));
    ui->edtDexterity->setText(QString::number(_monster->getDexterity()));
    ui->edtConstitution->setText(QString::number(_monster->getConstitution()));
    ui->edtIntelligence->setText(QString::number(_monster->getIntelligence()));
    ui->edtWisdom->setText(QString::number(_monster->getWisdom()));
    ui->edtCharisma->setText(QString::number(_monster->getCharisma()));
    ui->edtSkills->setText(_monster->getSkillString());
    ui->edtConditionImmunities->setText(_monster->getConditionImmunities());
    ui->edtDamageImmunities->setText(_monster->getDamageImmunities());
    ui->edtDamageResistances->setText(_monster->getDamageResistances());
    ui->edtDamageVulnerabilities->setText(_monster->getDamageVulnerabilities());
    ui->edtSenses->setText(_monster->getSenses());
    ui->edtLanguages->setText(_monster->getLanguages());

    if(_monster->getLanguages().isEmpty())
        ui->edtLanguages->setText("---");

    interpretChallengeRating(_monster->getChallenge());

    clearActionWidgets();

    QList<MonsterAction> actionList = _monster->getActions();
    ui->scrollActions->setVisible(actionList.count() > 0);
    {
        _actionsWidget = new QWidget;
        QVBoxLayout* actionsLayout = new QVBoxLayout(_actionsWidget);
        actionsLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        for(int i = 0; i < actionList.count(); ++i)
        {
            MonsterActionFrame* newFrame = new MonsterActionFrame(actionList.at(i));
            connect(newFrame, SIGNAL(deleteAction(const MonsterAction&)), this, SLOT(deleteAction(const MonsterAction&)));
            connect(newFrame, SIGNAL(frameChanged()), this, SLOT(handleEditedData()));
            actionsLayout->addWidget(newFrame);
        }
        ui->scrollActions->setWidget(_actionsWidget);
    }

    actionList = _monster->getLegendaryActions();
    ui->scrollLegendaryActions->setVisible(actionList.count() > 0);
    {
        _legendaryActionsWidget = new QWidget;
        QVBoxLayout* actionsLayout = new QVBoxLayout(_legendaryActionsWidget);
        actionsLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        for(int i = 0; i < actionList.count(); ++i)
        {
            MonsterActionFrame* newFrame = new MonsterActionFrame(actionList.at(i));
            connect(newFrame, SIGNAL(deleteAction(const MonsterAction&)), this, SLOT(deleteLegendaryAction(const MonsterAction&)));
            connect(newFrame, SIGNAL(frameChanged()), this, SLOT(handleEditedData()));
            actionsLayout->addWidget(newFrame);
        }
        ui->scrollLegendaryActions->setWidget(_legendaryActionsWidget);
    }

    actionList = _monster->getSpecialAbilities();
    ui->scrollSpecialAbilities->setVisible(actionList.count() > 0);
    {
        _specialAbilitiesWidget = new QWidget;
        QVBoxLayout* actionsLayout = new QVBoxLayout(_specialAbilitiesWidget);
        actionsLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        for(int i = 0; i < actionList.count(); ++i)
        {
            MonsterActionFrame* newFrame = new MonsterActionFrame(actionList.at(i));
            connect(newFrame, SIGNAL(deleteAction(const MonsterAction&)), this, SLOT(deleteSpecialAbility(const MonsterAction&)));
            connect(newFrame, SIGNAL(frameChanged()), this, SLOT(handleEditedData()));
            actionsLayout->addWidget(newFrame);
        }
        ui->scrollSpecialAbilities->setWidget(_specialAbilitiesWidget);
    }

    actionList = _monster->getReactions();
    ui->scrollReactions->setVisible(actionList.count() > 0);
    {
        _reactionsWidget = new QWidget;
        QVBoxLayout* actionsLayout = new QVBoxLayout(_reactionsWidget);
        actionsLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        for(int i = 0; i < actionList.count(); ++i)
        {
            MonsterActionFrame* newFrame = new MonsterActionFrame(actionList.at(i));
            connect(newFrame, SIGNAL(deleteAction(const MonsterAction&)), this, SLOT(deleteReaction(const MonsterAction&)));
            connect(newFrame, SIGNAL(frameChanged()), this, SLOT(handleEditedData()));
            actionsLayout->addWidget(newFrame);
        }
        ui->scrollReactions->setWidget(_reactionsWidget);
    }

    updateAbilityMods();

    if(!_edit)
    {
        ui->btnLeft->hide();
        ui->btnRight->hide();

        if(_monster->getSenses().isEmpty())
        {
            ui->edtSenses->hide();
            ui->lblSenses->hide();
        }
    }

    ui->btnLeft->setEnabled(_monster != Bestiary::Instance()->getFirstMonsterClass());
    ui->btnRight->setEnabled(_monster != Bestiary::Instance()->getLastMonsterClass());

    ui->edtName->setReadOnly(!_edit);
    ui->edtMonsterSize->setReadOnly(!_edit);
    ui->edtMonsterType->setReadOnly(!_edit);
    ui->edtMonsterSubType->setReadOnly(!_edit);
    ui->edtAlignment->setReadOnly(!_edit);
    ui->edtArmorClass->setReadOnly(!_edit);
    ui->edtHitDice->setReadOnly(!_edit);
    ui->edtSpeed->setReadOnly(!_edit);
    ui->edtSize->setReadOnly(!_edit);
    ui->edtStrength->setReadOnly(!_edit);
    ui->edtDexterity->setReadOnly(!_edit);
    ui->edtConstitution->setReadOnly(!_edit);
    ui->edtIntelligence->setReadOnly(!_edit);
    ui->edtWisdom->setReadOnly(!_edit);
    ui->edtCharisma->setReadOnly(!_edit);
    ui->edtSkills->setReadOnly(!_edit);
    ui->edtConditionImmunities->setReadOnly(!_edit);
    ui->edtDamageImmunities->setReadOnly(!_edit);
    ui->edtDamageResistances->setReadOnly(!_edit);
    ui->edtDamageVulnerabilities->setReadOnly(!_edit);
    ui->edtSenses->setReadOnly(!_edit);
    ui->edtLanguages->setReadOnly(!_edit);
    ui->edtChallenge->setReadOnly(!_edit);
*/
}

bool BestiaryTemplateDialog::eventFilter(QObject *object, QEvent *event)
{
    if((event) && (event->type() == QEvent::MouseButtonPress))
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if(mouseEvent->button() == Qt::RightButton)
        {
            QMenu* popupMenu = new QMenu();

            QAction* addItem = new QAction(QString("Add..."), popupMenu);
            connect(addItem, &QAction::triggered, this, [=](){ this->handleAddResource(dynamic_cast<QWidget*>(object), this->_monster); });
            popupMenu->addAction(addItem);

            QScrollArea* scrollArea = dynamic_cast<QScrollArea*>(object);
            if(!scrollArea)
            {
                QAction* removeItem = new QAction(QString("Remove..."), popupMenu);
                connect(removeItem, &QAction::triggered, this, [=](){ this->handleRemoveResource(dynamic_cast<QWidget*>(object), this->_monster); });
                popupMenu->addAction(removeItem);
            }

            popupMenu->exec(mouseEvent->globalPosition().toPoint());
            popupMenu->deleteLater();
        }
    }

    return QDialog::eventFilter(object, event);
}

QObject* BestiaryTemplateDialog::getFrameObject()
{
    return this;
}
