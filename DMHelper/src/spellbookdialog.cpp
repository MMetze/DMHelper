#include "spellbookdialog.h"
#include "spell.h"
#include "spellbook.h"
#include "battledialogmodeleffect.h"
#include "conditionseditdialog.h"
#include "ui_spellbookdialog.h"
#include <QAbstractItemView>
#include <QInputDialog>
#include <QFileDialog>
#include <QComboBox>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QPainter>
#include <QDebug>

const int CONDITION_FRAME_SPACING = 8;

SpellbookDialog::SpellbookDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SpellbookDialog),
    _spell(nullptr),
    _tokenRotation(0),
    _conditionLayout(nullptr)
{
    ui->setupUi(this);

    connect(ui->btnLeft, SIGNAL(clicked()), this, SLOT(previousSpell()));
    connect(ui->btnRight, SIGNAL(clicked()), this, SLOT(nextSpell()));
    connect(ui->btnNewSpell, SIGNAL(clicked()), this, SLOT(createNewSpell()));
    connect(ui->btnDeleteSpell, SIGNAL(clicked()), this, SLOT(deleteCurrentSpell()));
    connect(ui->cmbSearch, SIGNAL(activated(QString)), this, SLOT(setSpell(QString)));

    ui->edtLevel->setValidator(new QIntValidator(0,100));
    connect(ui->edtName, SIGNAL(editingFinished()), this, SIGNAL(spellDataEdit()));
    connect(ui->edtLevel, SIGNAL(editingFinished()), this, SIGNAL(spellDataEdit()));
    connect(ui->edtSchool, SIGNAL(editingFinished()), this, SIGNAL(spellDataEdit()));
    connect(ui->edtClasses, SIGNAL(editingFinished()), this, SIGNAL(spellDataEdit()));
    connect(ui->edtCastingTime, SIGNAL(editingFinished()), this, SIGNAL(spellDataEdit()));
    connect(ui->edtDuration, SIGNAL(editingFinished()), this, SIGNAL(spellDataEdit()));
    connect(ui->edtRange, SIGNAL(editingFinished()), this, SIGNAL(spellDataEdit()));
    connect(ui->edtComponents, SIGNAL(editingFinished()), this, SIGNAL(spellDataEdit()));
    connect(ui->chkRitual, SIGNAL(stateChanged(int)), this, SIGNAL(spellDataEdit()));
    connect(ui->edtDescription, SIGNAL(textChanged()), this, SIGNAL(spellDataEdit()));
    connect(this, SIGNAL(spellDataEdit()), this, SLOT(handleEditedData()));

    ui->cmbSearch->view()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    ui->btnEffectColor->setRotationVisible(false);
    ui->edtEffectWidth->setValidator(new QIntValidator(0,1000));
    ui->edtEffectWidth->setStyleSheet(QString("QLineEdit:disabled {color: rgb(196, 196, 196);}"));
    ui->edtEffectHeight->setValidator(new QIntValidator(0,1000));
    ui->edtEffectHeight->setStyleSheet(QString("QLineEdit:disabled {color: rgb(196, 196, 196);}"));
    ui->edtEffectToken->setStyleSheet(QString("QLineEdit:disabled {color: rgb(196, 196, 196);}"));
    ui->btnEffectTokenBrowse->setStyleSheet(QString("QPushButton:disabled {color: rgb(196, 196, 196);}"));
    connect(ui->edtEffectWidth, &QLineEdit::textEdited, this, &SpellbookDialog::handleWidthChanged);
    connect(ui->edtEffectHeight, &QLineEdit::textEdited, this, &SpellbookDialog::handleHeightChanged);
    connect(ui->btnEffectColor, SIGNAL(colorChanged(QColor)), this, SIGNAL(spellDataEdit()));
    connect(ui->sliderOpacity, &QAbstractSlider::valueChanged, this, &SpellbookDialog::spellDataEdit);
    connect(ui->edtEffectToken, SIGNAL(textChanged()), this, SIGNAL(spellDataEdit()));
    connect(ui->cmbEffectType, SIGNAL(currentIndexChanged(int)), this, SLOT(handleEffectChanged(int)));
    connect(ui->btnEditConditions, &QAbstractButton::clicked, this, &SpellbookDialog::editConditions);
    connect(ui->btnEffectTokenBrowse, &QAbstractButton::clicked, this, &SpellbookDialog::selectToken);
    connect(ui->grpShape, &QGroupBox::clicked, this, &SpellbookDialog::spellDataEdit);
    connect(ui->btnTokenCW, &QAbstractButton::clicked, this, &SpellbookDialog::handleTokenRotateCW);
    connect(ui->btnTokenCCW, &QAbstractButton::clicked, this, &SpellbookDialog::handleTokenRotateCCW);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
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

    disconnect(this, SIGNAL(spellDataEdit()), this, SLOT(handleEditedData()));

    ui->edtName->setText(_spell->getName());
    ui->edtName->setCursorPosition(0);
    ui->edtLevel->setText(QString::number(_spell->getLevel()));
    ui->edtLevel->setCursorPosition(0);
    ui->edtSchool->setText(_spell->getSchool());
    ui->edtSchool->setCursorPosition(0);
    ui->edtClasses->setText(_spell->getClasses());
    ui->edtClasses->setCursorPosition(0);
    ui->edtClasses->setToolTip(_spell->getClasses());
    ui->edtCastingTime->setText(_spell->getTime());
    ui->edtCastingTime->setCursorPosition(0);
    ui->edtDuration->setText(_spell->getDuration());
    ui->edtDuration->setCursorPosition(0);
    ui->edtRange->setText(_spell->getRange());
    ui->edtRange->setCursorPosition(0);
    ui->edtComponents->setText(_spell->getComponents());
    ui->edtComponents->setCursorPosition(0);
    ui->edtComponents->setToolTip(_spell->getComponents());

    ui->chkRitual->setChecked(_spell->isRitual());

    ui->edtDescription->setPlainText(_spell->getDescription());

    ui->btnLeft->setEnabled(_spell != Spellbook::Instance()->getFirstSpell());
    ui->btnRight->setEnabled(_spell != Spellbook::Instance()->getLastSpell());

    ui->grpShape->setChecked(_spell->getEffectShapeActive());
    QColor shapeColor = _spell->getEffectColor();
    ui->sliderOpacity->setValue(shapeColor.alpha());
    shapeColor.setAlpha(255);
    ui->btnEffectColor->setColor(shapeColor);
    ui->edtEffectHeight->setText(QString::number(_spell->getEffectSize().height()));
    if((ui->cmbEffectType->currentIndex() == BattleDialogModelEffect::BattleDialogModelEffect_Radius) ||
       (ui->cmbEffectType->currentIndex() == BattleDialogModelEffect::BattleDialogModelEffect_Cone) ||
       (ui->cmbEffectType->currentIndex() == BattleDialogModelEffect::BattleDialogModelEffect_Cube))
    {
        ui->edtEffectWidth->setText(QString::number(_spell->getEffectSize().height()));
    }
    else
    {
        ui->edtEffectWidth->setText(QString::number(_spell->getEffectSize().width()));
    }

    ui->edtEffectToken->setText(_spell->getEffectToken());
    ui->cmbEffectType->setCurrentIndex(_spell->getEffectType());
    _tokenRotation = _spell->getEffectTokenRotation();
    handleEffectChanged(_spell->getEffectType());
    updateLayout();

    connect(this, SIGNAL(spellDataEdit()), this, SLOT(handleEditedData()));

    updateImage();
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
    updateImage();
}

void SpellbookDialog::handleEffectChanged(int index)
{
    if((index < 0) || (index >= BattleDialogModelEffect::BattleDialogModelEffect_Count))
        return;

    ui->lblSize->setText(index == BattleDialogModelEffect::BattleDialogModelEffect_Radius ? QString("Radius") : QString("Size"));

    ui->edtEffectWidth->setEnabled(index != BattleDialogModelEffect::BattleDialogModelEffect_Base);
    ui->edtEffectHeight->setEnabled(index != BattleDialogModelEffect::BattleDialogModelEffect_Base);
    ui->grpShape->setEnabled(index != BattleDialogModelEffect::BattleDialogModelEffect_Base);
    ui->grpConditions->setEnabled(index != BattleDialogModelEffect::BattleDialogModelEffect_Base);

    emit spellDataEdit();
}

void SpellbookDialog::handleWidthChanged()
{
    if((ui->edtEffectWidth->text() != ui->edtEffectHeight->text()) &&
       ((ui->cmbEffectType->currentIndex() == BattleDialogModelEffect::BattleDialogModelEffect_Radius) ||
        (ui->cmbEffectType->currentIndex() == BattleDialogModelEffect::BattleDialogModelEffect_Cone) ||
        (ui->cmbEffectType->currentIndex() == BattleDialogModelEffect::BattleDialogModelEffect_Cube)))
    {
        ui->edtEffectHeight->setText(ui->edtEffectWidth->text());
    }

    emit spellDataEdit();
}

void SpellbookDialog::handleHeightChanged()
{
    if((ui->edtEffectWidth->text() != ui->edtEffectHeight->text()) &&
       ((ui->cmbEffectType->currentIndex() == BattleDialogModelEffect::BattleDialogModelEffect_Radius) ||
        (ui->cmbEffectType->currentIndex() == BattleDialogModelEffect::BattleDialogModelEffect_Cone) ||
        (ui->cmbEffectType->currentIndex() == BattleDialogModelEffect::BattleDialogModelEffect_Cube)))
    {
        ui->edtEffectWidth->setText(ui->edtEffectHeight->text());
    }

    emit spellDataEdit();
}

void SpellbookDialog::handleTokenRotateCW()
{
    _tokenRotation += 90;
    if(_tokenRotation >= 360)
        _tokenRotation = 0;

    emit spellDataEdit();
}

void SpellbookDialog::handleTokenRotateCCW()
{
    _tokenRotation -= 90;
    if(_tokenRotation < 0)
        _tokenRotation = 270;

    emit spellDataEdit();
}

void SpellbookDialog::editConditions()
{
    if(!_spell)
        return;

    ConditionsEditDialog dlg;
    dlg.setConditions(_spell->getEffectConditions());
    int result = dlg.exec();
    if(result == QDialog::Accepted)
    {
        _spell->setEffectConditions(dlg.getConditions());
        emit spellDataEdit();
        updateLayout();
    }
}

void SpellbookDialog::selectToken()
{
    if(!_spell)
        return;

    QString searchDir;
    QFileInfo currentToken(_spell->getEffectTokenPath());
    if(currentToken.exists())
        searchDir = currentToken.absolutePath();

    QString tokenFile = QFileDialog::getOpenFileName(nullptr,
                                                     QString("Select a token file for the spell"),
                                                     searchDir);
    if(tokenFile.isEmpty())
    {
        qDebug() << "[SpellbookDialog] Not able to select a token file; selection dialog cancelled.";
        return;
    }

    ui->edtEffectToken->setText(tokenFile);

    emit spellDataEdit();
}

void SpellbookDialog::updateLayout()
{
    clearGrid();

    if(!_spell)
        return;

    qDebug() << "[SpellbookDialog] Creating a new condition layout";

    _conditionLayout = new QHBoxLayout;
    _conditionLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    _conditionLayout->setContentsMargins(0, 0, 0, 0);
    _conditionLayout->setSpacing(CONDITION_FRAME_SPACING);
    ui->frameConditions->setLayout(_conditionLayout);

    int conditions = _spell->getEffectConditions();

    qDebug() << "[SpellbookDialog] Adding conditions: " << conditions;

    for(int i = 0; i < Combatant::getConditionCount(); ++i)
    {
        Combatant::Condition condition = Combatant::getConditionByIndex(i);
        if(conditions & condition)
            addCondition(condition);
    }

    _conditionLayout->addStretch();

    update();
}

void SpellbookDialog::clearGrid()
{
    if(!_conditionLayout)
        return;

    qDebug() << "[SpellbookDialog] Clearing the condition layout";

    // Delete the grid entries
    QLayoutItem *child = nullptr;
    while((child = _conditionLayout->takeAt(0)) != nullptr)
    {
        delete child->widget();
        delete child;
    }

    delete _conditionLayout;
    _conditionLayout = nullptr;

    ui->frameConditions->update();
}

void SpellbookDialog::addCondition(Combatant::Condition condition)
{
    if(!_conditionLayout)
        return;

    QString resourceIcon = QString(":/img/data/img/") + Combatant::getConditionIcon(condition) + QString(".png");
    QLabel* conditionLabel = new QLabel(this);
    int frameHeight = ui->frameConditions->height();
    int iconSize = frameHeight - (2 * conditionLabel->margin());

    conditionLabel->setPixmap(QPixmap(resourceIcon).scaled(iconSize, iconSize));
    conditionLabel->setToolTip(Combatant::getConditionDescription(condition));
    conditionLabel->setMinimumWidth(frameHeight);
    conditionLabel->setMaximumWidth(frameHeight);
    conditionLabel->setMinimumHeight(frameHeight);
    conditionLabel->setMaximumHeight(frameHeight);

    _conditionLayout->addWidget(conditionLabel);
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

    ui->edtEffectWidth->setMinimumWidth(ui->lblSize->width());
    ui->edtEffectWidth->setMaximumWidth(ui->lblSize->width());
    ui->edtEffectHeight->setMinimumWidth(ui->lblSize->width());
    ui->edtEffectHeight->setMaximumWidth(ui->lblSize->width());

    QFontMetrics metrics = ui->btnEditConditions->fontMetrics();
    QRect buttonTextRect = metrics.boundingRect(ui->btnEditConditions->text());
    ui->btnEditConditions->setMinimumWidth(buttonTextRect.width() * 2);
    ui->btnEditConditions->setMaximumWidth(buttonTextRect.width() * 2);
    ui->btnEditConditions->setMinimumHeight((buttonTextRect.height() * 3) / 2);
    ui->btnEditConditions->setMaximumHeight((buttonTextRect.height() * 3) / 2);

    ui->frameConditions->setMinimumHeight(buttonTextRect.height() * 3);
    ui->frameConditions->setMaximumHeight(buttonTextRect.height() * 3);

    ui->grpConditions->setMinimumHeight(buttonTextRect.height() * 4);
    ui->grpConditions->setMaximumHeight(buttonTextRect.height() * 4);

    updateLayout();
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
    _spell->setDescription(ui->edtDescription->toPlainText());

    // TODO: rolls,

    _spell->setEffectType(ui->cmbEffectType->currentIndex());
    _spell->setEffectShapeActive(ui->grpShape->isChecked());
    _spell->setEffectSize(QSize(ui->edtEffectWidth->text().toInt(), ui->edtEffectHeight->text().toInt()));
    QColor newColor = ui->btnEffectColor->getColor();
    newColor.setAlpha(ui->sliderOpacity->value());
    _spell->setEffectColor(newColor);
    _spell->setEffectToken(ui->edtEffectToken->text());
    _spell->setEffectTokenRotation(_tokenRotation);
    // Conditions are set directly

    _spell->endBatchChanges();
}

void SpellbookDialog::updateImage()
{
    QPixmap result(ui->lblEffectImage->size());

    int x = result.width() / 10;
    int y = result.height() / 10;
    int w = 8 * result.width() / 10;
    int h = 8 * result.height() / 10;

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
                    poly << QPointF(x + (w/2), y) << QPointF(x, y + h) << QPoint(x + w, y + h) << QPoint(x + (w/2), y);
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
                    int rotatePoint = qMax(result.width(),result.height()) / 2;

                    QTransform tokenTransform;
                    tokenTransform.translate(rotatePoint, rotatePoint);
                    tokenTransform.rotate(_tokenRotation);
                    tokenTransform.translate(-rotatePoint, -rotatePoint);
                    qDebug() << "[Spellbook Dialog] Image transform set: " << tokenTransform;
                    painter.setTransform(tokenTransform);

                    if(_tokenRotation != 180)
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
