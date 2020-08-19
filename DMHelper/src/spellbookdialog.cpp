#include "spellbookdialog.h"
#include "spell.h"
#include "spellbook.h"
#include "ui_spellbookdialog.h"
#include <QAbstractItemView>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>

SpellbookDialog::SpellbookDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SpellbookDialog),
    _spell(nullptr)
{
    ui->setupUi(this);

    connect(ui->btnLeft, SIGNAL(clicked()), this, SLOT(previousSpell()));
    connect(ui->btnRight, SIGNAL(clicked()), this, SLOT(nextSpell()));
    connect(ui->btnNewSpell, SIGNAL(clicked()), this, SLOT(createNewSpell()));
    connect(ui->btnDeleteSpell, SIGNAL(clicked()), this, SLOT(deleteCurrentSpell()));
    connect(ui->cmbSearch, SIGNAL(activated(QString)), this, SLOT(setSpell(QString)));

    ui->edtLevel->setValidator(new QIntValidator(0,100));
    connect(ui->edtName, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtLevel, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtSchool, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtClasses, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtCastingTime, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtDuration, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtRange, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));
    connect(ui->edtComponents, SIGNAL(editingFinished()), this, SLOT(handleEditedData()));

    connect(ui->chkRitual, SIGNAL(stateChanged(int)), this, SLOT(handleEditedData()));

    connect(ui->edtDescription, SIGNAL(textChanged()), this, SLOT(handleEditedData()));
    connect(ui->edtRoll, SIGNAL(textChanged()), this, SLOT(handleEditedData()));

    ui->cmbSearch->view()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
}

SpellbookDialog::~SpellbookDialog()
{
    delete ui;
}

Spell* SpellbookDialog::getSpell() const
{
    return _spell;
}

void SpellbookDialog::setSpell(Spell* spell)
{
    if(!spell)
        return;

    qDebug() << "[Spellbook Dialog] Set Spell to " << spell->getName();

    if(_spell)
        storeSpellData();

    _spell = spell;

    if(ui->cmbSearch->currentText() != _spell->getName())
        ui->cmbSearch->setCurrentText(_spell->getName());

    ui->edtName->setText(_spell->getName());
    ui->edtLevel->setText(QString::number(_spell->getLevel()));
    ui->edtSchool->setText(_spell->getSchool());
    ui->edtClasses->setText(_spell->getClasses());
    ui->edtCastingTime->setText(_spell->getTime());
    ui->edtDuration->setText(_spell->getDuration());
    ui->edtRange->setText(_spell->getRange());
    ui->edtComponents->setText(_spell->getComponents());

    ui->chkRitual->setChecked(_spell->isRitual());

    ui->edtDescription->setPlainText(_spell->getDescription());
    ui->edtRoll->setPlainText(_spell->getRolls().join(QChar::LineFeed));

    ui->btnLeft->setEnabled(_spell != Spellbook::Instance()->getFirstSpell());
    ui->btnRight->setEnabled(_spell != Spellbook::Instance()->getLastSpell());
}

void SpellbookDialog::setSpell(const QString& spellName)
{
    Spell* spell = Spellbook::Instance()->getSpell(spellName);
    if(spell)
        setSpell(spell);
}

void SpellbookDialog::previousSpell()
{
    Spell* previousSpell = Spellbook::Instance()->getPreviousSpell(_spell);
    if(previousSpell)
        setSpell(previousSpell);
}

void SpellbookDialog::nextSpell()
{
    Spell* nextSpell = Spellbook::Instance()->getNextSpell(_spell);
    if(nextSpell)
        setSpell(nextSpell);
}

void SpellbookDialog::createNewSpell()
{
    qDebug() << "[Spellbook Dialog] Creating a new spell...";

    bool ok;
    QString spellName = QInputDialog::getText(this, QString("Enter New Spell Name"),QString("New Spell"),QLineEdit::Normal,QString(),&ok);
    if((!ok)||(spellName.isEmpty()))
    {
        qDebug() << "[Spellbook Dialog] New monster not created because the monster name dialog was cancelled";
        return;
    }

    Spell* spell;
    if(Spellbook::Instance()->exists(spellName))
    {
        spell = Spellbook::Instance()->getSpell(spellName);
        qDebug() << "[Spellbook Dialog] New spell already exists, selecting new spell: " << spell;
    }
    else
    {
        spell = new Spell(spellName);

        if(Spellbook::Instance()->count() > 0)
        {
            QMessageBox::StandardButton templateQuestion = QMessageBox::question(this,
                                                                                 QString("New Spell"),
                                                                                 QString("Do you want to base this spell on an already existing spell?"));

            if(templateQuestion == QMessageBox::Yes)
            {
                QString templateName = QInputDialog::getItem(this,
                                                             QString("New Spell Selection"),
                                                             QString("Select the spell you would like to base the new spell on:"),
                                                             Spellbook::Instance()->getSpellList(),
                                                             0,
                                                             false,
                                                             &ok);
                if((!ok) || (templateName.isEmpty()))
                {
                    qDebug() << "[Spellbook Dialog] New spell not created because the select template spell dialog was cancelled";
                    return;
                }

                Spell* templateClass = Spellbook::Instance()->getSpell(templateName);
                if(!templateClass)
                {
                    qDebug() << "[Spellbook Dialog] New spell not created because not able to find selected template spell: " << templateName;
                    return;
                }

                spell->cloneSpell(*templateClass);
            }
        }

        Spellbook::Instance()->insertSpell(spell);
        qDebug() << "[Spellbook Dialog] New Spell created: " << spell;
    }

    setSpell(spell);
    show();
    activateWindow();
}

void SpellbookDialog::deleteCurrentSpell()
{
    if(!_spell)
        return;

    qDebug() << "[Spellbook Dialog] Deleting spell: " << _spell->getName();

    QMessageBox::StandardButton confirm = QMessageBox::critical(this,
                                                                QString("Delete Spell"),
                                                                QString("Are you sure you want to delete the spell ") + _spell->getName(),
                                                                QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));
    if(confirm == QMessageBox::No)
    {
        qDebug() << "[Spellbook Dialog] Delete of spell cancelled by user: " << _spell->getName();
        return;
    }

    Spellbook::Instance()->removeSpell(_spell);
    if(Spellbook::Instance()->count() > 0)
    {
        setSpell(Spellbook::Instance()->getFirstSpell());
    }
    else
    {
        _spell = nullptr;
        hide();
    }
}

void SpellbookDialog::dataChanged()
{
    _spell = nullptr;
    ui->cmbSearch->clear();
    ui->cmbSearch->addItems(Spellbook::Instance()->getSpellList());
}

void SpellbookDialog::handleEditedData()
{
    qDebug() << "[Spellbook Dialog] Spellbook Dialog edit detected... storing data";
    storeSpellData();
}

void SpellbookDialog::closeEvent(QCloseEvent * event)
{
    Q_UNUSED(event);
    qDebug() << "[Spellbook Dialog] Spellbook Dialog closing... storing data";
    storeSpellData();
    QDialog::closeEvent(event);
}

void SpellbookDialog::showEvent(QShowEvent * event)
{
    Q_UNUSED(event);
    qDebug() << "[Spellbook Dialog] Spellbook Dialog shown";
    connect(Spellbook::Instance(),SIGNAL(changed()),this,SLOT(dataChanged()));
    QDialog::showEvent(event);
}

void SpellbookDialog::hideEvent(QHideEvent * event)
{
    Q_UNUSED(event);
    qDebug() << "[Spellbook Dialog] Spellbook Dialog hidden... storing data";
    storeSpellData();
    QDialog::hideEvent(event);
}

void SpellbookDialog::focusOutEvent(QFocusEvent * event)
{
    Q_UNUSED(event);
    qDebug() << "[Spellbook Dialog] Spellbook Dialog lost focus... storing data";
    storeSpellData();
    QDialog::focusOutEvent(event);
}

void SpellbookDialog::storeSpellData()
{
    if(!_spell)
        return;

    qDebug() << "[Spellbook Dialog] Storing spell data for " << _spell->getName();

    _spell->beginBatchChanges();

    _spell->setName(ui->edtName->text());
    _spell->setLevel(ui->edtLevel->text().toInt());
    _spell->setSchool(ui->edtSchool->text());
    _spell->setClasses(ui->edtClasses->text());
    _spell->setTime(ui->edtCastingTime->text());
    _spell->setDuration(ui->edtDuration->text());
    _spell->setRange(ui->edtRange->text());
    _spell->setComponents(ui->edtComponents->text());
    _spell->setRitual(ui->chkRitual->isChecked());

    // TODO: description & rolls

    _spell->endBatchChanges();
}
