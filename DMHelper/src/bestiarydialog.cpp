#include "bestiarydialog.h"
#include "bestiary.h"
#include "monsterclass.h"
#include "combatant.h"
#include "monsteraction.h"
#include "monsteractionframe.h"
#include "monsteractioneditdialog.h"
#include "publishbuttonframe.h"
#include "bestiaryfindtokendialog.h"
#include "tokeneditdialog.h"
#include "tokeneditor.h"
#include "optionscontainer.h"
#include <QIntValidator>
#include <QDoubleValidator>
#include <QInputDialog>
#include <QMouseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QAbstractItemView>
#include <QShortcut>
#include <QDebug>
#include "ui_bestiarydialog.h"

BestiaryDialog::BestiaryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BestiaryDialog),
    _actionsWidget(nullptr),
    _legendaryActionsWidget(nullptr),
    _specialAbilitiesWidget(nullptr),
    _reactionsWidget(nullptr),
    _options(nullptr),
    _monster(nullptr),
    _currentToken(0),
    _edit(false),
    _mouseDown(false),
    _searchString()
{
    ui->setupUi(this);

    connect(ui->btnLeft, SIGNAL(clicked()), this, SLOT(previousMonster()));
    connect(ui->btnRight, SIGNAL(clicked()), this, SLOT(nextMonster()));
    connect(ui->btnNewMonster, SIGNAL(clicked()), this, SLOT(createNewMonster()));
    connect(ui->btnDeleteMonster, SIGNAL(clicked()), this, SLOT(deleteCurrentMonster()));
    connect(ui->cmbSearch, &QComboBox::currentTextChanged, this, [=](const QString &newValue) {setMonster(newValue);});
    connect(ui->framePublish, SIGNAL(clicked()), this, SLOT(handlePublishButton()));
    QShortcut* publishShortcut = new QShortcut(QKeySequence(tr("Ctrl+P", "Publish")), this);
    connect(publishShortcut, SIGNAL(activated()), ui->framePublish, SLOT(clickPublish()));

    connect(ui->btnPreviousToken, &QPushButton::clicked, this, &BestiaryDialog::handlePreviousToken);
    connect(ui->btnAddToken, &QPushButton::clicked, this, &BestiaryDialog::handleAddToken);

    connect(ui->btnEditIcon, &QPushButton::clicked, this, &BestiaryDialog::handleEditToken);
    connect(ui->btnSearchToken, &QPushButton::clicked, this, &BestiaryDialog::handleSearchToken);
    connect(ui->btnReload, SIGNAL(clicked()), this, SLOT(handleReloadImage()));
    connect(ui->btnClear, SIGNAL(clicked()), this, SLOT(handleClearImage()));
    connect(ui->btnNextToken, &QPushButton::clicked, this, &BestiaryDialog::handleNextToken);

    connect(ui->edtStrength, SIGNAL(editingFinished()), this, SLOT(abilityChanged()));
    connect(ui->edtDexterity, SIGNAL(editingFinished()), this, SLOT(abilityChanged()));
    connect(ui->edtConstitution, SIGNAL(editingFinished()), this, SLOT(abilityChanged()));
    connect(ui->edtIntelligence, SIGNAL(editingFinished()), this, SLOT(abilityChanged()));
    connect(ui->edtWisdom, SIGNAL(editingFinished()), this, SLOT(abilityChanged()));
    connect(ui->edtCharisma, SIGNAL(editingFinished()), this, SLOT(abilityChanged()));

    connect(ui->edtName, SIGNAL(editingFinished()), this, SLOT(monsterRenamed()));
    connect(ui->edtHitDice, SIGNAL(editingFinished()), this, SLOT(hitDiceChanged()));

    connect(ui->btnAddAction, SIGNAL(clicked()), this, SLOT(addAction()));
    connect(ui->btnAddLegendaryAction, SIGNAL(clicked()), this, SLOT(addLegendaryAction()));
    connect(ui->btnAddSpecialAbility, SIGNAL(clicked()), this, SLOT(addSpecialAbility()));
    connect(ui->btnAddReaction, SIGNAL(clicked()), this, SLOT(addReaction()));

    ui->edtArmorClass->setValidator(new QIntValidator(0, 100));
    ui->edtAverageHitPoints->setValidator(new QIntValidator(0, 10000));

    connect(ui->edtName, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtMonsterSize, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtMonsterType, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtMonsterSubType, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtAlignment, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtArmorClass, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtHitDice, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtSpeed, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtSize, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtStrength, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtDexterity, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtConstitution, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtIntelligence, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtWisdom, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtCharisma, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtSkills, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtConditionImmunities, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtDamageImmunities, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtDamageResistances, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtDamageVulnerabilities, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtSenses, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtLanguages, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtChallenge, SIGNAL(editingFinished()), this, SLOT(handleChallengeEdited()));

    ui->cmbSearch->view()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    ui->chkPrivate->hide();
}

BestiaryDialog::~BestiaryDialog()
{
    clearActionWidgets();

    delete ui;
}

MonsterClass* BestiaryDialog::getMonster() const
{
    return _monster;
}

void BestiaryDialog::setOptions(OptionsContainer* options)
{
    if(!options)
        return;

    _options = options;
}

void BestiaryDialog::setMonster(MonsterClass* monster, bool edit)
{
    if((!monster) || (_monster == monster))
        return;

    qDebug() << "[Bestiary Dialog] Set Monster to " << monster->getName();

    if(_monster && _edit)
        storeMonsterData();

    _monster = monster;
    _edit = edit;

    if(ui->cmbSearch->currentText() != _monster->getName())
        ui->cmbSearch->setCurrentText(_monster->getName());

    if(_monster->getIconCount() == 0)
        _monster->searchForIcons();
    setTokenIndex(0);

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

    //ui->btnLeft->setEnabled(_monster != Bestiary::Instance()->getFirstMonsterClass());
    //ui->btnRight->setEnabled(_monster != Bestiary::Instance()->getLastMonsterClass());

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
}

void BestiaryDialog::setMonster(const QString& monsterName, bool edit)
{
    MonsterClass* monsterClass = nullptr;//Bestiary::Instance()->getMonsterClass(monsterName);
    if(monsterClass)
        setMonster(monsterClass, edit);
}

void BestiaryDialog::createNewMonster()
{
    qDebug() << "[Bestiary Dialog] Creating a new monster...";

    bool ok;
    QString monsterName = QInputDialog::getText(this, QString("Enter New Monster Name"), QString("New Monster"), QLineEdit::Normal, QString(), &ok);
    if((!ok)||(monsterName.isEmpty()))
    {
        qDebug() << "[Bestiary Dialog] New monster not created because the monster name dialog was cancelled";
        return;
    }

    MonsterClass* monsterClass;
    if(Bestiary::Instance()->exists(monsterName))
    {
        monsterClass = nullptr;//Bestiary::Instance()->getMonsterClass(monsterName);
        qDebug() << "[Bestiary Dialog] New Monster already exists, selecting new monster: " << monsterClass;
    }
    else
    {
        monsterClass = new MonsterClass(monsterName);

        if(Bestiary::Instance()->count() > 0)
        {
            QMessageBox::StandardButton templateQuestion = QMessageBox::question(this,
                                                                                 QString("New Monster"),
                                                                                 QString("Do you want to base this monster on an already existing monster?"));

            if(templateQuestion == QMessageBox::Yes)
            {
                QString templateName = QInputDialog::getItem(this,
                                                             QString("New Monster Selection"),
                                                             QString("Select the monster you would like to base the new monster on:"),
                                                             Bestiary::Instance()->getMonsterList(),
                                                             0,
                                                             false,
                                                             &ok);
                if((!ok) || (templateName.isEmpty()))
                {
                    qDebug() << "[Bestiary Dialog] New monster not created because the select template monster dialog was cancelled";
                    delete monsterClass;
                    return;
                }

                MonsterClass* templateClass = nullptr;//Bestiary::Instance()->getMonsterClass(templateName);
                if(!templateClass)
                {
                    qDebug() << "[Bestiary Dialog] New monster not created because not able to find selected template monster: " << templateName;
                    delete monsterClass;
                    return;
                }

                monsterClass->cloneMonster(*templateClass);
            }
        }

        //Bestiary::Instance()->insertMonsterClass(monsterClass);
        qDebug() << "[Bestiary Dialog] New Monster created: " << monsterClass;
    }

    setMonster(monsterClass);
    show();
    activateWindow();
}

void BestiaryDialog::deleteCurrentMonster()
{
    if(!_monster)
        return;

    qDebug() << "[Bestiary Dialog] Deleting monster: " << _monster->getName();

    QMessageBox::StandardButton confirm = QMessageBox::critical(this,
                                                                QString("Delete Monster"),
                                                                QString("Are you sure you want to delete the monster ") + _monster->getName(),
                                                                QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));
    if(confirm == QMessageBox::No)
    {
        qDebug() << "[Bestiary Dialog] Delete of monster cancelled by user: " << _monster->getName();
        return;
    }

    //Bestiary::Instance()->removeMonsterClass(_monster);
    if(Bestiary::Instance()->count() > 0)
    {
        //setMonster(Bestiary::Instance()->getFirstMonsterClass());
    }
    else
    {
        _monster = nullptr;
        hide();
    }
}

void BestiaryDialog::dataChanged()
{
    _monster = nullptr;
    ui->cmbSearch->clear();
    ui->cmbSearch->addItems(Bestiary::Instance()->getMonsterList());
}

void BestiaryDialog::hitDiceChanged()
{
    Dice newHitDice(ui->edtHitDice->text());
    ui->edtAverageHitPoints->setText(QString::number(newHitDice.average()));
}

void BestiaryDialog::abilityChanged()
{
    if(!_monster)
        return;

    _monster->setStrength(ui->edtStrength->text().toInt());
    _monster->setDexterity(ui->edtDexterity->text().toInt());
    _monster->setConstitution(ui->edtConstitution->text().toInt());
    _monster->setIntelligence(ui->edtIntelligence->text().toInt());
    _monster->setWisdom(ui->edtWisdom->text().toInt());
    _monster->setCharisma(ui->edtCharisma->text().toInt());

    updateAbilityMods();
}

void BestiaryDialog::updateAbilityMods()
{
    if(!_monster)
        return;

    ui->lblStrengthBonus->setText(QString("(") + Combatant::getAbilityModStr(_monster->getStrength()) + QString(")"));
    ui->lblDexterityBonus->setText(QString("(") + Combatant::getAbilityModStr(_monster->getDexterity()) + QString(")"));
    ui->lblConstitutionBonus->setText(QString("(") + Combatant::getAbilityModStr(_monster->getConstitution()) + QString(")"));
    ui->lblIntelligenceBonus->setText(QString("(") + Combatant::getAbilityModStr(_monster->getIntelligence()) + QString(")"));
    ui->lblWisdomBonus->setText(QString("(") + Combatant::getAbilityModStr(_monster->getWisdom()) + QString(")"));
    ui->lblCharismaBonus->setText(QString("(") + Combatant::getAbilityModStr(_monster->getCharisma()) + QString(")"));
}

void BestiaryDialog::monsterRenamed()
{
    if((!_monster) || (ui->edtName->text() == _monster->getName()))
        return;

    //Bestiary::Instance()->renameMonster(_monster, ui->edtName->text());
}

void BestiaryDialog::handlePublishButton()
{
    if(!_monster)
        return;

    QImage iconImg;
    QString iconFile = _monster->getIcon(_currentToken);
    QString iconPath = Bestiary::Instance()->getDirectory().filePath(iconFile);
    if((!iconPath.isEmpty()) && (iconImg.load(iconPath) == true))
    {
        if(ui->framePublish->getRotation() != 0)
            iconImg = iconImg.transformed(QTransform().rotate(ui->framePublish->getRotation()), Qt::SmoothTransformation);

        emit publishMonsterImage(iconImg, ui->framePublish->getColor());
    }
}

void BestiaryDialog::handlePreviousToken()
{
    setTokenIndex(_currentToken - 1);
}

void BestiaryDialog::handleAddToken()
{
    if(!_monster)
        return;

    QString filename = selectToken();
    if(!filename.isEmpty())
    {
        _monster->addIcon(filename);
        setTokenIndex(_monster->getIconList().indexOf(filename));
    }
}

void BestiaryDialog::handleEditToken()
{
    // Use the TokenEditDialog to edit the character icon
    if((!_monster) || (!_options))
        return;

    TokenEditDialog* dlg = new TokenEditDialog(_monster->getIconPixmap(DMHelper::PixmapSize_Full).toImage(),
                                               *_options,
                                               1.0,
                                               QPoint(),
                                               false);
    if(dlg->exec() == QDialog::Accepted)
    {
        QImage newToken = dlg->getFinalImage();
        if(newToken.isNull())
            return;

        QString tokenPath = QFileDialog::getExistingDirectory(this, tr("Select Token Directory"), _monster->getIcon(_currentToken).isEmpty() ? QString() : QFileInfo(_monster->getIcon(_currentToken)).absolutePath());
        if(tokenPath.isEmpty())
            return;

        QDir tokenDir(tokenPath);

        int fileIndex = 1;
        QString tokenFile = _monster->getName() + QString(".png");
        while(tokenDir.exists(tokenFile))
            tokenFile = _monster->getName() + QString::number(fileIndex++) + QString(".png");

        QString finalTokenPath = tokenDir.absoluteFilePath(tokenFile);
        newToken.save(finalTokenPath);

        _monster->setIcon(_currentToken, finalTokenPath);
        loadMonsterImage();

        if(dlg->getEditor())
            dlg->getEditor()->applyEditorToOptions(*_options);

    }

    dlg->deleteLater();
}

void BestiaryDialog::handleSearchToken()
{
    if((!_monster) || (!_options))
        return;

    BestiaryFindTokenDialog* dlg = new BestiaryFindTokenDialog(_monster->getName(), _searchString, *_options);
    dlg->resize(width() * 9 / 10, height() * 9 / 10);
    if(dlg->exec() == QDialog::Accepted)
    {
        if(dlg->getEditor())
            dlg->getEditor()->applyEditorToOptions(*_options);

        createTokenFiles(dlg);
    }

    dlg->deleteLater();

    handleReloadImage();
    update();
}

void BestiaryDialog::handleReloadImage()
{
    if(!_monster)
        return;

    _monster->searchForIcons();
    _monster->refreshIconPixmaps();
    setTokenIndex(_currentToken);
}

void BestiaryDialog::handleClearImage()
{
    if(!_monster)
        return;

    QString currentIconPath = Bestiary::Instance()->getDirectory().absoluteFilePath(_monster->getIcon(_currentToken));
    if((currentIconPath.isEmpty()) || (!QFile::exists(currentIconPath)))
    {
        qDebug() << "[BestiaryDialog] Unable to find token file to remove from monster: " << _monster->getName() << ", " << currentIconPath;
        return;
    }

    // Ask if the app should remove the file
    QMessageBox::StandardButton result = QMessageBox::question(this, tr("Delete Image"), tr("Do you want to also delete the token file from the disk?\n\n") + currentIconPath);
    if(result == QMessageBox::Yes)
    {
        qDebug() << "[BestiaryDialog] Removing token file for monster: " << _monster->getName() << ", " << currentIconPath;
        QFile::remove(currentIconPath);
    }

    _monster->removeIcon(_currentToken);
    if(_monster->getIconCount() == 0)
        ui->lblIcon->clear();
    else
        setTokenIndex(_currentToken > 0 ? _currentToken - 1 : 0);
}

void BestiaryDialog::handleNextToken()
{
    setTokenIndex(_currentToken + 1);
}

void BestiaryDialog::addAction()
{
    if((!_monster) || (!ui->scrollActions->widget()) || (!ui->scrollActions->widget()->layout()))
        return;

    MonsterActionEditDialog dlg(MonsterAction(0, QString(), QString(), Dice()));
    if(dlg.exec() == QDialog::Accepted)
    {
        _monster->addAction(dlg.getAction());
        MonsterActionFrame* newFrame = new MonsterActionFrame(dlg.getAction());
        ui->scrollActions->widget()->layout()->addWidget(newFrame);
        connect(newFrame, SIGNAL(deleteAction(const MonsterAction&)), this, SLOT(deleteAction(const MonsterAction&)));
        connect(newFrame, SIGNAL(frameChanged()), this, SLOT(handleEditedData()));
        ui->scrollActions->setVisible(true);
        handleEditedData();
    }
}

void BestiaryDialog::deleteAction(const MonsterAction& action)
{
    if(_monster)
    {
        if(_monster->removeAction(action) <= 0)
        {
            ui->scrollActions->setVisible(false);
        }
        handleEditedData();
    }
}

void BestiaryDialog::addLegendaryAction()
{
    if((!_monster) || (!ui->scrollLegendaryActions->widget()) || (!ui->scrollLegendaryActions->widget()->layout()))
        return;

    MonsterActionEditDialog dlg(MonsterAction(0, QString(), QString(), Dice()));
    if(dlg.exec() == QDialog::Accepted)
    {
        _monster->addLegendaryAction(dlg.getAction());
        MonsterActionFrame* newFrame = new MonsterActionFrame(dlg.getAction());
        ui->scrollLegendaryActions->widget()->layout()->addWidget(newFrame);
        connect(newFrame, SIGNAL(deleteAction(const MonsterAction&)), this, SLOT(deleteLegendaryAction(const MonsterAction&)));
        connect(newFrame, SIGNAL(frameChanged()), this, SLOT(handleEditedData()));
        ui->scrollLegendaryActions->setVisible(true);
        handleEditedData();
    }
}

void BestiaryDialog::deleteLegendaryAction(const MonsterAction& action)
{
    if(_monster)
    {
        if(_monster->removeLegendaryAction(action) <= 0)
        {
            ui->scrollLegendaryActions->setVisible(false);
        }
        handleEditedData();
    }
}

void BestiaryDialog::addSpecialAbility()
{
    if((!_monster) || (!ui->scrollSpecialAbilities->widget()) || (!ui->scrollSpecialAbilities->widget()->layout()))
        return;

    MonsterActionEditDialog dlg(MonsterAction(0, QString(), QString(), Dice()));
    if(dlg.exec() == QDialog::Accepted)
    {
        _monster->addSpecialAbility(dlg.getAction());
        MonsterActionFrame* newFrame = new MonsterActionFrame(dlg.getAction());
        ui->scrollSpecialAbilities->widget()->layout()->addWidget(newFrame);
        connect(newFrame, SIGNAL(deleteAction(const MonsterAction&)), this, SLOT(deleteSpecialAbility(const MonsterAction&)));
        connect(newFrame, SIGNAL(frameChanged()), this, SLOT(handleEditedData()));
        ui->scrollSpecialAbilities->setVisible(true);
        handleEditedData();
    }
}

void BestiaryDialog::deleteSpecialAbility(const MonsterAction& action)
{
    if(_monster)
    {
        if(_monster->removeSpecialAbility(action) <= 0)
        {
            ui->scrollSpecialAbilities->setVisible(false);
        }
        handleEditedData();
    }
}

void BestiaryDialog::addReaction()
{
    if((!_monster) || (!ui->scrollReactions->widget()) || (!ui->scrollReactions->widget()->layout()))
        return;

    MonsterActionEditDialog dlg(MonsterAction(0, QString(), QString(), Dice()));
    if(dlg.exec() == QDialog::Accepted)
    {
        _monster->addReaction(dlg.getAction());
        MonsterActionFrame* newFrame = new MonsterActionFrame(dlg.getAction());
        ui->scrollReactions->widget()->layout()->addWidget(newFrame);
        connect(newFrame, SIGNAL(deleteAction(const MonsterAction&)), this, SLOT(deleteReaction(const MonsterAction&)));
        connect(newFrame, SIGNAL(frameChanged()), this, SLOT(handleEditedData()));
        ui->scrollReactions->setVisible(true);
        handleEditedData();
    }
}

void BestiaryDialog::deleteReaction(const MonsterAction& action)
{
    if(_monster)
    {
        if(_monster->removeReaction(action) <= 0)
        {
            ui->scrollReactions->setVisible(false);
        }
        handleEditedData();
    }
}

void BestiaryDialog::handleChallengeEdited()
{
    interpretChallengeRating(ui->edtChallenge->text());
    handleEditedData();
}

void BestiaryDialog::handleEditedData()
{
    qDebug() << "[Bestiary Dialog] Bestiary Dialog edit detected... storing data";
    storeMonsterData();
}

void BestiaryDialog::closeEvent(QCloseEvent * event)
{
    Q_UNUSED(event);
    qDebug() << "[Bestiary Dialog] Bestiary Dialog closing... storing data";
    storeMonsterData();
    QDialog::closeEvent(event);
}

void BestiaryDialog::mousePressEvent(QMouseEvent * event)
{
    Q_UNUSED(event);
    if(_edit && _monster)
    {
        if(ui->lblIcon->frameGeometry().contains(event->pos()))
        {
            ui->lblIcon->setFrameStyle(QFrame::Panel | QFrame::Sunken);
            _mouseDown = true;
        }
    }
}

void BestiaryDialog::mouseReleaseEvent(QMouseEvent * event)
{
    if((!_edit) || (!_mouseDown) || (!_monster))
        return;

    if(!ui->lblIcon->frameGeometry().contains(event->pos()))
        return;

    ui->lblIcon->setFrameStyle(QFrame::Panel | QFrame::Raised);
    _mouseDown = false;
    QString filename = selectToken();
    if(filename.isEmpty())
        return;

    if(_monster->getIconCount() == 0)
        _monster->addIcon(filename);
    else
        _monster->setIcon(_currentToken, filename);

    loadMonsterImage();
}

void BestiaryDialog::showEvent(QShowEvent * event)
{
    Q_UNUSED(event);
    qDebug() << "[Bestiary Dialog] Bestiary Dialog shown";
    connect(Bestiary::Instance(), SIGNAL(changed()), this, SLOT(dataChanged()));
    setMonster(ui->cmbSearch->currentText());
    QDialog::showEvent(event);
}

void BestiaryDialog::hideEvent(QHideEvent * event)
{
    Q_UNUSED(event);

    // TODO: Confirm that this is not actually needed since we store data at focusOut
    qDebug() << "[Bestiary Dialog] Bestiary Dialog hidden... storing data";
    storeMonsterData();
    QDialog::hideEvent(event);

    emit dialogClosed();
}

void BestiaryDialog::focusOutEvent(QFocusEvent * event)
{
    Q_UNUSED(event);

    qDebug() << "[Bestiary Dialog] Bestiary Dialog lost focus... storing data";
    storeMonsterData();
    QDialog::focusOutEvent(event);

    emit dialogClosed();
}

void BestiaryDialog::previousMonster()
{
    MonsterClass* previousClass = nullptr;//Bestiary::Instance()->getPreviousMonsterClass(_monster);
    if(previousClass)
        setMonster(previousClass);
}

void BestiaryDialog::nextMonster()
{
    MonsterClass* nextClass = nullptr;//Bestiary::Instance()->getNextMonsterClass(_monster);
    if(nextClass)
        setMonster(nextClass);
}

void BestiaryDialog::createTokenFiles(BestiaryFindTokenDialog* dialog)
{
    if((!dialog) || (!_options))
        return;

    QList<QImage> resultList = dialog->retrieveSelection(!dialog->isEditingToken());
    int fileIndex = 1;
    QString tokenPath = QFileDialog::getExistingDirectory(this, tr("Select Token Directory"), Bestiary::Instance()->getDirectory().absolutePath());
    if(tokenPath.isEmpty())
        return;

    QDir tokenDir(tokenPath);

    // Create a default token editor from the bestiary dialog
    TokenEditDialog* editDialog = nullptr;
    if(dialog->isEditingToken())
    {
        editDialog = new TokenEditDialog(QString(),
                                         *_options,
                                         1.0,
                                         QPoint(),
                                         false,
                                         this);
    }

    QString tokenFile = _monster->getName() + QString(".png");
    foreach(QImage image, resultList)
    {
        if(!image.isNull())
        {
            bool saveOk = true;
            if((dialog->isEditingToken() && editDialog))
            {
                editDialog->setSourceImage(image);
                if(_options->getTokenBackgroundFill())
                    editDialog->setBackgroundFillColor(image.pixelColor(0,0));
                editDialog->updateImage();
                if(editDialog->exec() == QDialog::Accepted)
                    image = editDialog->getFinalImage();
                else
                    saveOk = false;
            }

            if(saveOk)
            {
                while(tokenDir.exists(tokenFile))
                    tokenFile = _monster->getName() + QString::number(fileIndex++) + QString(".png");

                image.save(tokenDir.absoluteFilePath(tokenFile));
            }
        }
    }

    _monster->searchForIcons();
    setTokenIndex(_monster->getIconList().indexOf(tokenFile));

    if(editDialog)
        editDialog->deleteLater();
}

QString BestiaryDialog::selectToken()
{
    return QFileDialog::getOpenFileName(this, QString("Select New Image..."));
}

void BestiaryDialog::setTokenIndex(int index)
{
    if(!_monster)
    {
        ui->btnPreviousToken->setVisible(false);
        ui->btnNextToken->setVisible(false);
        ui->btnClear->setEnabled(false);
        return;
    }

    if((index < 0) || (index >= _monster->getIconCount()))
    {
        ui->lblIcon->setPixmap(QPixmap());
        ui->btnPreviousToken->setVisible(false);
        ui->btnNextToken->setVisible(false);
        return;
    }

    _currentToken = index;
    loadMonsterImage();
    ui->btnPreviousToken->setEnabled(_currentToken > 0);
    ui->btnNextToken->setEnabled(_currentToken < _monster->getIconCount() - 1);
    ui->btnClear->setEnabled(_monster->getIconCount() > 0);
    ui->btnPreviousToken->setVisible(_monster->getIconCount() > 1);
    ui->btnNextToken->setVisible(_monster->getIconCount() > 1);
}

void BestiaryDialog::loadMonsterImage()
{
    ui->lblIcon->setPixmap(_monster->getIconPixmap(DMHelper::PixmapSize_Showcase, _currentToken));
}

void BestiaryDialog::storeMonsterData()
{
    if((!_monster) || (!_edit))
        return;

    qDebug() << "[Bestiary Dialog] Storing monster data for " << _monster->getName();

    _monster->beginBatchChanges();

    _monster->setPrivate(ui->chkPrivate->isChecked());
    _monster->setName(ui->edtName->text());
    _monster->setMonsterSize(ui->edtMonsterSize->text());
    _monster->setMonsterType(ui->edtMonsterType->text());
    _monster->setMonsterSubType(ui->edtMonsterSubType->text());
    _monster->setAlignment(ui->edtAlignment->text());
    _monster->setArmorClass(ui->edtArmorClass->text().toInt());
    _monster->setHitDice(Dice(ui->edtHitDice->text()));
    _monster->setSpeed(ui->edtSpeed->text());
    _monster->setMonsterSize(ui->edtSize->text());
    _monster->setConditionImmunities(ui->edtConditionImmunities->text());
    _monster->setDamageImmunities(ui->edtDamageImmunities->text());
    _monster->setDamageResistances(ui->edtDamageResistances->text());
    _monster->setDamageVulnerabilities(ui->edtDamageVulnerabilities->text());
    _monster->setSenses(ui->edtSenses->text());
    _monster->setLanguages(ui->edtLanguages->text());
    _monster->setChallenge(ui->edtChallenge->text());
    _monster->setSkillString(ui->edtSkills->text());

    if((ui->scrollActions->widget()) && (ui->scrollActions->widget()->layout()))
    {
        for(int i = 0; i < ui->scrollActions->widget()->layout()->count(); ++i)
        {
            QLayoutItem* item = ui->scrollActions->widget()->layout()->itemAt(i);
            MonsterActionFrame* frame = dynamic_cast<MonsterActionFrame*>(item->widget());
            if(frame)
            {
                _monster->setAction(i, frame->getAction());
            }
        }
    }

    if((ui->scrollLegendaryActions->widget()) && (ui->scrollLegendaryActions->widget()->layout()))
    {
        for(int i = 0; i < ui->scrollLegendaryActions->widget()->layout()->count(); ++i)
        {
            QLayoutItem* item = ui->scrollLegendaryActions->widget()->layout()->itemAt(i);
            MonsterActionFrame* frame = dynamic_cast<MonsterActionFrame*>(item->widget());
            if(frame)
            {
                _monster->setLegendaryAction(i, frame->getAction());
            }
        }
    }

    if((ui->scrollSpecialAbilities->widget()) && (ui->scrollSpecialAbilities->widget()->layout()))
    {
        for(int i = 0; i < ui->scrollSpecialAbilities->widget()->layout()->count(); ++i)
        {
            QLayoutItem* item = ui->scrollSpecialAbilities->widget()->layout()->itemAt(i);
            MonsterActionFrame* frame = dynamic_cast<MonsterActionFrame*>(item->widget());
            if(frame)
            {
                _monster->setSpecialAbility(i, frame->getAction());
            }
        }
    }

    if((ui->scrollReactions->widget()) && (ui->scrollReactions->widget()->layout()))
    {
        for(int i = 0; i < ui->scrollReactions->widget()->layout()->count(); ++i)
        {
            QLayoutItem* item = ui->scrollReactions->widget()->layout()->itemAt(i);
            MonsterActionFrame* frame = dynamic_cast<MonsterActionFrame*>(item->widget());
            if(frame)
            {
                _monster->setReaction(i, frame->getAction());
            }
        }
    }

    _monster->endBatchChanges();
}

void BestiaryDialog::clearActionWidgets()
{
    ui->scrollActions->takeWidget();
    clearWidget(_actionsWidget);
    delete _actionsWidget;
    _actionsWidget = nullptr;

    ui->scrollLegendaryActions->takeWidget();
    clearWidget(_legendaryActionsWidget);
    delete _legendaryActionsWidget;
    _legendaryActionsWidget = nullptr;

    ui->scrollSpecialAbilities->takeWidget();
    clearWidget(_specialAbilitiesWidget);
    delete _specialAbilitiesWidget;
    _specialAbilitiesWidget = nullptr;

    ui->scrollReactions->takeWidget();
    clearWidget(_reactionsWidget);
    delete _reactionsWidget;
    _reactionsWidget = nullptr;

}

void BestiaryDialog::clearWidget(QWidget* widget)
{
    if(!widget)
        return;

    QLayout* layout = widget->layout();
    if(!layout)
        return;

    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr)
        delete child;
}

void BestiaryDialog::interpretChallengeRating(const QString& inputCR)
{
    if(inputCR.isEmpty())
        return;

    QString resultCR;

    if((inputCR == QString("0.125")) || (inputCR == QString("1/8")))
        resultCR = QString("1/8");
    else if((inputCR == QString("0.25")) || (inputCR == QString("1/4")))
        resultCR = QString("1/4");
    else if((inputCR == QString("0.5")) || (inputCR == QString("1/2")))
        resultCR = QString("1/2");
    else
    {
        bool convertResult = false;
        int intCR = inputCR.toInt(&convertResult);
        if((convertResult) && (intCR >= 0) && (intCR <= 30))
        {
            resultCR = QString::number(intCR);
        }
    }

    if(resultCR != ui->edtChallenge->text())
        ui->edtChallenge->setText(resultCR);
    ui->edtXP->setText(QString::number(MonsterClass::getExperienceByCR(resultCR)));
}
