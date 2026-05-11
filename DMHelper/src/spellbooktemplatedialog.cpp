#include "spellbooktemplatedialog.h"
#include "ui_spellbooktemplatedialog.h"
#include "spellv2.h"
#include "spellbook.h"
#include "spellbookfactory.h"
#include "templatefactory.h"
#include "battledialogmodeleffect.h"
#include "colorpushbutton.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QCompleter>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QAbstractItemView>
#include <QShortcut>
#include <QFileDialog>
#include <QFileInfo>
#include <QPainter>
#include <QLabel>
#include <QIntValidator>
#include <QTransform>
#include <QDebug>

static constexpr int EFFECT_SIZE_VALIDATOR_MAX = 1000;
static constexpr int EFFECT_TOKEN_ROTATION_STEP = 90;
static constexpr int EFFECT_TOKEN_ROTATION_MAX = 360;
static constexpr int EFFECT_IMAGE_MARGIN_DIVISOR = 10;
static constexpr int EFFECT_IMAGE_CONTENT_DIVISOR = 8;

SpellbookTemplateDialog::SpellbookTemplateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SpellbookTemplateDialog),
    _uiWidget(nullptr),
    _spell(nullptr),
    _edit(true),
    _tokenRotation(0),
    _loadingEffectUI(false)
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

    // Effect panel wiring
    ui->btnEffectColor->setRotationVisible(false);

    ui->edtEffectWidth->setValidator(new QIntValidator(0, EFFECT_SIZE_VALIDATOR_MAX, this));
    ui->edtEffectWidth->setStyleSheet(QString("QLineEdit:disabled {color: rgb(196, 196, 196);}"));
    ui->edtEffectHeight->setValidator(new QIntValidator(0, EFFECT_SIZE_VALIDATOR_MAX, this));
    ui->edtEffectHeight->setStyleSheet(QString("QLineEdit:disabled {color: rgb(196, 196, 196);}"));
    ui->edtEffectToken->setStyleSheet(QString("QLineEdit:disabled {color: rgb(196, 196, 196);}"));

    connect(ui->cmbEffectType, &QComboBox::currentIndexChanged,
            this, &SpellbookTemplateDialog::handleEffectTypeChanged);
    connect(ui->edtEffectWidth, &QLineEdit::textEdited,
            this, &SpellbookTemplateDialog::handleEffectWidthChanged);
    connect(ui->edtEffectHeight, &QLineEdit::textEdited,
            this, &SpellbookTemplateDialog::handleEffectHeightChanged);
    connect(ui->btnEffectColor, &ColorPushButton::colorChanged,
            this, [this](const QColor&) { storeEffectData(); updateEffectImage(); });
    connect(ui->sliderOpacity, &QAbstractSlider::valueChanged,
            this, [this](int) { storeEffectData(); updateEffectImage(); });
    connect(ui->edtEffectToken, &QLineEdit::textChanged,
            this, [this](const QString& text) {
                ui->lblTwoMinute->setVisible(text.contains(QString("2-Minute Tabletop")));
                ui->lblTwoMinuteBlank->setVisible(ui->lblTwoMinute->isHidden());
                storeEffectData();
                updateEffectImage();
            });
    connect(ui->btnEffectTokenBrowse, &QAbstractButton::clicked,
            this, &SpellbookTemplateDialog::selectEffectToken);
    connect(ui->btnTokenCW, &QAbstractButton::clicked,
            this, &SpellbookTemplateDialog::handleTokenRotateCW);
    connect(ui->btnTokenCCW, &QAbstractButton::clicked,
            this, &SpellbookTemplateDialog::handleTokenRotateCCW);
    connect(ui->grpShape, &QGroupBox::clicked,
            this, [this](bool) { storeEffectData(); updateEffectImage(); });
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
        loadEffectUI();

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

// ---------------------------------------------------------------------------
// Effect-panel slots
// ---------------------------------------------------------------------------

void SpellbookTemplateDialog::handleEffectTypeChanged(int index)
{
    if((index < 0) || (index >= BattleDialogModelEffect::BattleDialogModelEffect_Count))
        return;

    ui->lblSize->setText(index == BattleDialogModelEffect::BattleDialogModelEffect_Radius
                             ? QString("Radius")
                             : QString("Size"));

    ui->edtEffectWidth->setEnabled(index != BattleDialogModelEffect::BattleDialogModelEffect_Base);
    ui->edtEffectHeight->setEnabled(index != BattleDialogModelEffect::BattleDialogModelEffect_Base);
    ui->grpShape->setEnabled(index != BattleDialogModelEffect::BattleDialogModelEffect_Base);

    storeEffectData();
    updateEffectImage();
}

void SpellbookTemplateDialog::handleEffectWidthChanged()
{
    if((ui->edtEffectWidth->text() != ui->edtEffectHeight->text()) &&
       ((ui->cmbEffectType->currentIndex() == BattleDialogModelEffect::BattleDialogModelEffect_Radius) ||
        (ui->cmbEffectType->currentIndex() == BattleDialogModelEffect::BattleDialogModelEffect_Cone) ||
        (ui->cmbEffectType->currentIndex() == BattleDialogModelEffect::BattleDialogModelEffect_Cube)))
    {
        ui->edtEffectHeight->setText(ui->edtEffectWidth->text());
    }

    storeEffectData();
    updateEffectImage();
}

void SpellbookTemplateDialog::handleEffectHeightChanged()
{
    if((ui->edtEffectWidth->text() != ui->edtEffectHeight->text()) &&
       ((ui->cmbEffectType->currentIndex() == BattleDialogModelEffect::BattleDialogModelEffect_Radius) ||
        (ui->cmbEffectType->currentIndex() == BattleDialogModelEffect::BattleDialogModelEffect_Cone) ||
        (ui->cmbEffectType->currentIndex() == BattleDialogModelEffect::BattleDialogModelEffect_Cube)))
    {
        ui->edtEffectWidth->setText(ui->edtEffectHeight->text());
    }

    storeEffectData();
    updateEffectImage();
}

void SpellbookTemplateDialog::handleTokenRotateCW()
{
    _tokenRotation += EFFECT_TOKEN_ROTATION_STEP;
    if(_tokenRotation >= EFFECT_TOKEN_ROTATION_MAX)
        _tokenRotation = 0;

    storeEffectData();
    updateEffectImage();
}

void SpellbookTemplateDialog::handleTokenRotateCCW()
{
    _tokenRotation -= EFFECT_TOKEN_ROTATION_STEP;
    if(_tokenRotation < 0)
        _tokenRotation = EFFECT_TOKEN_ROTATION_MAX - EFFECT_TOKEN_ROTATION_STEP;

    storeEffectData();
    updateEffectImage();
}

void SpellbookTemplateDialog::selectEffectToken()
{
    if(!_spell)
        return;

    QString searchDir;
    QFileInfo currentToken(_spell->getEffectTokenPath());
    if(currentToken.exists())
        searchDir = currentToken.absolutePath();

    QString tokenFile = QFileDialog::getOpenFileName(this,
                                                     QString("Select a token file for the spell"),
                                                     searchDir);
    if(tokenFile.isEmpty())
    {
        qDebug() << "[SpellbookTemplateDialog] Token selection cancelled.";
        return;
    }

    ui->edtEffectToken->setText(tokenFile);
    // textChanged signal handles the rest (lblTwoMinute, storeEffectData, updateEffectImage)
}

// ---------------------------------------------------------------------------
// Effect-panel helpers
// ---------------------------------------------------------------------------

void SpellbookTemplateDialog::loadEffectUI()
{
    if(!_spell)
    {
        ui->grpShape->setChecked(false);
        ui->sliderOpacity->setValue(0);
        ui->edtEffectWidth->clear();
        ui->edtEffectHeight->clear();
        ui->edtEffectToken->clear();
        ui->lblTwoMinute->setVisible(false);
        ui->lblTwoMinuteBlank->setVisible(true);
        ui->cmbEffectType->setCurrentIndex(0);
        _tokenRotation = 0;
        updateEffectImage();
        return;
    }

    _loadingEffectUI = true;

    ui->grpShape->setChecked(_spell->getEffectShapeActive());

    QColor shapeColor = _spell->getEffectColor();
    ui->sliderOpacity->setValue(shapeColor.alpha());
    shapeColor.setAlpha(255);
    ui->btnEffectColor->setColor(shapeColor);

    ui->edtEffectHeight->setText(QString::number(_spell->getEffectSize().height()));
    if((_spell->getEffectType() == BattleDialogModelEffect::BattleDialogModelEffect_Radius) ||
       (_spell->getEffectType() == BattleDialogModelEffect::BattleDialogModelEffect_Cone) ||
       (_spell->getEffectType() == BattleDialogModelEffect::BattleDialogModelEffect_Cube))
    {
        ui->edtEffectWidth->setText(QString::number(_spell->getEffectSize().height()));
    }
    else
    {
        ui->edtEffectWidth->setText(QString::number(_spell->getEffectSize().width()));
    }

    ui->edtEffectToken->setText(_spell->getEffectToken());
    ui->lblTwoMinute->setVisible(ui->edtEffectToken->text().contains(QString("2-Minute Tabletop")));
    ui->lblTwoMinuteBlank->setVisible(ui->lblTwoMinute->isHidden());

    ui->cmbEffectType->setCurrentIndex(_spell->getEffectType());
    _tokenRotation = _spell->getEffectTokenRotation();

    // Refresh enable-states while still guarded (storeEffectData will early-return)
    handleEffectTypeChanged(ui->cmbEffectType->currentIndex());

    _loadingEffectUI = false;

    updateEffectImage();
}

void SpellbookTemplateDialog::storeEffectData()
{
    if(!_spell || _loadingEffectUI)
        return;

    _spell->beginBatchChanges();

    _spell->setEffectType(ui->cmbEffectType->currentIndex());
    _spell->setEffectShapeActive(ui->grpShape->isChecked());
    _spell->setEffectSize(QSize(ui->edtEffectWidth->text().toInt(),
                                ui->edtEffectHeight->text().toInt()));
    QColor newColor = ui->btnEffectColor->getColor();
    newColor.setAlpha(ui->sliderOpacity->value());
    _spell->setEffectColor(newColor);
    _spell->setEffectToken(ui->edtEffectToken->text());
    _spell->setEffectTokenRotation(_tokenRotation);

    _spell->endBatchChanges();
}

void SpellbookTemplateDialog::updateEffectImage()
{
    QPixmap result(ui->lblEffectImage->size());

    int x = result.width() / EFFECT_IMAGE_MARGIN_DIVISOR;
    int y = result.height() / EFFECT_IMAGE_MARGIN_DIVISOR;
    int w = EFFECT_IMAGE_CONTENT_DIVISOR * result.width() / EFFECT_IMAGE_MARGIN_DIVISOR;
    int h = EFFECT_IMAGE_CONTENT_DIVISOR * result.height() / EFFECT_IMAGE_MARGIN_DIVISOR;

    QPainter painter;
    painter.begin(&result);

    painter.drawPixmap(0, 0, result.width(), result.height(), QPixmap(":/img/data/parchment.jpg"));

    if(ui->cmbEffectType->currentIndex() != BattleDialogModelEffect::BattleDialogModelEffect_Base)
    {
        if(ui->grpShape->isChecked())
        {
            QColor shapeColor = ui->btnEffectColor->getColor();
            painter.setPen(QPen(shapeColor, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            shapeColor.setAlpha(ui->sliderOpacity->value());
            painter.setBrush(QBrush(shapeColor));

            switch(ui->cmbEffectType->currentIndex())
            {
                case BattleDialogModelEffect::BattleDialogModelEffect_Radius:
                    painter.drawEllipse(x, y, w, h);
                    break;
                case BattleDialogModelEffect::BattleDialogModelEffect_Cone:
                {
                    QPolygonF poly;
                    poly << QPointF(x + (w / 2), y)
                         << QPointF(x, y + h)
                         << QPointF(x + w, y + h)
                         << QPointF(x + (w / 2), y);
                    painter.drawPolygon(poly);
                    break;
                }
                case BattleDialogModelEffect::BattleDialogModelEffect_Cube:
                    painter.drawRect(x, y, w, h);
                    break;
                case BattleDialogModelEffect::BattleDialogModelEffect_Line:
                {
                    QSize lineSize = QSize(ui->edtEffectWidth->text().toInt(),
                                          ui->edtEffectHeight->text().toInt()).scaled(w, h, Qt::KeepAspectRatio);
                    x = (result.width() / 2) - (lineSize.width() / 2);
                    y = (result.height() / 2) - (lineSize.height() / 2);
                    w = lineSize.width();
                    h = lineSize.height();
                    painter.drawRect(x, y, w, h);
                    break;
                }
                case BattleDialogModelEffect::BattleDialogModelEffect_Base:
                case BattleDialogModelEffect::BattleDialogModelEffect_Object:
                case BattleDialogModelEffect::BattleDialogModelEffect_ObjectVideo:
                default:
                    break;
            }
        }

        if(!ui->edtEffectToken->text().isEmpty())
        {
            QPixmap imagePmp;
            if(imagePmp.load(Spellbook::Instance()->getDirectory().filePath(ui->edtEffectToken->text())))
            {
                if(_tokenRotation != 0)
                {
                    int rotatePoint = qMax(result.width(), result.height()) / 2;

                    QTransform tokenTransform;
                    tokenTransform.translate(rotatePoint, rotatePoint);
                    tokenTransform.rotate(_tokenRotation);
                    tokenTransform.translate(-rotatePoint, -rotatePoint);
                    qDebug() << "[SpellbookTemplateDialog] Image transform set: " << tokenTransform;
                    painter.setTransform(tokenTransform);

                    if(_tokenRotation != EFFECT_TOKEN_ROTATION_MAX / 2)
                    {
                        std::swap(w, h);
                        std::swap(x, y);
                    }
                }

                painter.drawPixmap(x, y, w, h, imagePmp);
            }
        }
    }

    painter.end();

    ui->lblEffectImage->setPixmap(result);
}
