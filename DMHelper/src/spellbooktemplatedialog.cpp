#include "spellbooktemplatedialog.h"
#include "ui_spellbooktemplatedialog.h"
#include "spellv2.h"
#include "spellbook.h"
#include "spellbookfactory.h"
#include "templatefactory.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QCompleter>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QAbstractItemView>
#include <QShortcut>
#include <QDebug>

SpellbookTemplateDialog::SpellbookTemplateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SpellbookTemplateDialog),
    _uiWidget(nullptr),
    _spell(nullptr),
    _edit(true)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
    setAttribute(Qt::WA_StyledBackground, true);

    connect(ui->btnLeft, &QPushButton::clicked, this, &SpellbookTemplateDialog::previousSpell);
    connect(ui->btnRight, &QPushButton::clicked, this, &SpellbookTemplateDialog::nextSpell);
    connect(ui->btnNewSpell, &QPushButton::clicked, this, &SpellbookTemplateDialog::createNewSpell);
    connect(ui->btnDeleteSpell, &QPushButton::clicked, this, &SpellbookTemplateDialog::deleteCurrentSpell);
    connect(ui->cmbSearch, &QComboBox::textActivated, this, static_cast<void (SpellbookTemplateDialog::*)(const QString&)>(&SpellbookTemplateDialog::setSpell));

    ui->cmbSearch->view()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    QCompleter *completer = new QCompleter(ui->cmbSearch->model(), this);
    completer->setFilterMode(Qt::MatchContains);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    ui->cmbSearch->setCompleter(completer);
}

SpellbookTemplateDialog::~SpellbookTemplateDialog()
{
    delete ui;
}

void SpellbookTemplateDialog::loadSpellUITemplate(const QString& templateFile)
{
    QString absoluteTemplateFile = TemplateFactory::getAbsoluteTemplateFile(templateFile);
    if(absoluteTemplateFile.isEmpty())
    {
        qDebug() << "[SpellbookTemplateDialog] ERROR: UI Template File " << templateFile << " could not be found!";
        return;
    }

    if(absoluteTemplateFile == _uiFilename)
    {
        qDebug() << "[SpellbookTemplateDialog] UI Template File " << absoluteTemplateFile << " already loaded";
        return;
    }

    QWidget* newWidget = TemplateFactory::loadUITemplate(absoluteTemplateFile);
    if(!newWidget)
    {
        qDebug() << "[SpellbookTemplateDialog] ERROR: UI Template File " << templateFile << " could not be loaded!";
        return;
    }

    delete _uiWidget;
    if(ui->scrollAreaWidgetContents->layout())
        delete ui->scrollAreaWidgetContents->layout();

    _uiWidget = newWidget;
    _uiFilename = absoluteTemplateFile;

    postLoadConfiguration(this, _uiWidget);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    layout->addWidget(_uiWidget);
    ui->scrollAreaWidgetContents->setLayout(layout);

    if((_spell) && (SpellbookFactory::Instance()))
        SpellbookFactory::Instance()->readObjectData(_uiWidget, _spell, this, this);
}

Spellv2* SpellbookTemplateDialog::getSpell() const
{
    return _spell;
}

void SpellbookTemplateDialog::setSpell(Spellv2* spell, bool edit)
{
    if((_spell == spell) || (!SpellbookFactory::Instance()))
        return;

    if(_spell && _uiWidget)
        SpellbookFactory::Instance()->disconnectWidget(_uiWidget);

    _spell = spell;
    _edit = edit;

    if(_spell && _uiWidget)
    {
        SpellbookFactory::Instance()->readObjectData(_uiWidget, _spell, this, this);

        if(ui->cmbSearch->currentText() != _spell->getName())
            ui->cmbSearch->setCurrentText(_spell->getName());
    }

    emit spellChanged();
}

void SpellbookTemplateDialog::setSpell(const QString& spellName, bool edit)
{
    if(!Spellbook::Instance())
        return;
    setSpell(Spellbook::Instance()->getSpell(spellName), edit);
}

void SpellbookTemplateDialog::setSpell(const QString& spellName)
{
    setSpell(spellName, true);
}

void SpellbookTemplateDialog::createNewSpell()
{
    if(!Spellbook::Instance())
        return;

    bool ok = false;
    QString spellName = QInputDialog::getText(this, QString("Enter New Spell Name"), QString("New Spell"), QLineEdit::Normal, QString(), &ok);
    if((!ok) || (spellName.isEmpty()))
        return;

    Spellv2* spell = nullptr;
    if(Spellbook::Instance()->exists(spellName))
    {
        spell = Spellbook::Instance()->getSpell(spellName);
    }
    else
    {
        spell = new Spellv2(spellName);
        Spellbook::Instance()->insertSpell(spell);
    }

    setSpell(spell);
    show();
    activateWindow();
}

void SpellbookTemplateDialog::deleteCurrentSpell()
{
    if((!_spell) || (!Spellbook::Instance()))
        return;

    QMessageBox::StandardButton confirm = QMessageBox::critical(this,
                                                                QString("Delete Spell"),
                                                                QString("Are you sure you want to delete the spell ") + _spell->getName(),
                                                                QMessageBox::Yes | QMessageBox::No);
    if(confirm == QMessageBox::No)
        return;

    Spellv2* removedSpell = _spell;

    Spellv2* nextSpell = Spellbook::Instance()->getNextSpell(_spell);
    if(nextSpell)
    {
        setSpell(nextSpell);
    }
    else
    {
        Spellv2* prevSpell = Spellbook::Instance()->getPreviousSpell(_spell);
        if(prevSpell)
            setSpell(prevSpell);
        else
            setSpell(Spellbook::Instance()->getFirstSpell());
    }

    Spellbook::Instance()->removeSpell(removedSpell);
}

void SpellbookTemplateDialog::previousSpell()
{
    if(!Spellbook::Instance())
        return;
    Spellv2* prev = Spellbook::Instance()->getPreviousSpell(_spell);
    if(prev)
        setSpell(prev);
}

void SpellbookTemplateDialog::nextSpell()
{
    if(!Spellbook::Instance())
        return;
    Spellv2* next = Spellbook::Instance()->getNextSpell(_spell);
    if(next)
        setSpell(next);
}

void SpellbookTemplateDialog::dataChanged()
{
    if(!Spellbook::Instance())
        return;

    QString previousSpell = ui->cmbSearch->currentText();

    setSpell(static_cast<Spellv2*>(nullptr));
    disconnect(ui->cmbSearch, &QComboBox::textActivated, this, static_cast<void (SpellbookTemplateDialog::*)(const QString&)>(&SpellbookTemplateDialog::setSpell));
    ui->cmbSearch->clear();

    QList<QString> spellList = Spellbook::Instance()->getSpellList();
    if(spellList.isEmpty())
    {
        connect(ui->cmbSearch, &QComboBox::textActivated, this, static_cast<void (SpellbookTemplateDialog::*)(const QString&)>(&SpellbookTemplateDialog::setSpell));
        return;
    }

    ui->cmbSearch->addItems(spellList);

    connect(ui->cmbSearch, &QComboBox::textActivated, this, static_cast<void (SpellbookTemplateDialog::*)(const QString&)>(&SpellbookTemplateDialog::setSpell));

    if(!previousSpell.isEmpty())
    {
        int index = ui->cmbSearch->findText(previousSpell);
        if(index >= 0)
        {
            if(ui->cmbSearch->currentIndex() == index)
                setSpell(previousSpell);
            else
                ui->cmbSearch->setCurrentIndex(index);
        }
    }
    else
    {
        setSpell(ui->cmbSearch->currentText());
    }
}

void SpellbookTemplateDialog::spellRenamed()
{
    if((!_spell) || (!Spellbook::Instance()))
        return;

    QLineEdit* nameEdit = getValueEdit("name");
    if(!nameEdit)
        return;

    if(nameEdit->text() == _spell->getName())
        return;

    Spellbook::Instance()->renameSpell(_spell, nameEdit->text());
}

void SpellbookTemplateDialog::showEvent(QShowEvent* event)
{
    setSpell(ui->cmbSearch->currentText());
    QDialog::showEvent(event);
}

void SpellbookTemplateDialog::hideEvent(QHideEvent* event)
{
    QDialog::hideEvent(event);
    emit dialogClosed();
}

void SpellbookTemplateDialog::focusOutEvent(QFocusEvent* event)
{
    QDialog::focusOutEvent(event);
    emit dialogClosed();
}

QObject* SpellbookTemplateDialog::getFrameObject()
{
    return this;
}

void SpellbookTemplateDialog::postLoadConfiguration(QWidget* owner, QWidget* uiWidget)
{
    // Wire the spell name QLineEdit to the rename handler so renames flow back
    // through the Spellbook map (matching BestiaryTemplateDialog behaviour).
    if(_uiWidget)
    {
        QList<QLineEdit*> lineEdits = _uiWidget->findChildren<QLineEdit*>();
        for(QLineEdit* lineEdit : lineEdits)
        {
            if(!lineEdit)
                continue;

            QString keyString = lineEdit->property(TemplateFactory::TEMPLATE_PROPERTY).toString();
            if(keyString == QString("name"))
                connect(lineEdit, &QLineEdit::editingFinished, this, &SpellbookTemplateDialog::spellRenamed);
        }
    }

    TemplateFrame::postLoadConfiguration(owner, uiWidget);
}

QLineEdit* SpellbookTemplateDialog::getValueEdit(const QString& key)
{
    if(!_uiWidget)
        return nullptr;

    QList<QLineEdit*> lineEdits = _uiWidget->findChildren<QLineEdit*>();
    for(QLineEdit* lineEdit : lineEdits)
    {
        if(!lineEdit)
            continue;
        if(lineEdit->property(TemplateFactory::TEMPLATE_PROPERTY).toString() == key)
            return lineEdit;
    }
    return nullptr;
}
