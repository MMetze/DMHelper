#include "charactertemplateframe.h"
#include "ui_charactertemplateframe.h"
#include "characterimporter.h"
#include "characterimportheroforgedialog.h"
#include "tokeneditdialog.h"
#include "optionscontainer.h"
#include "combatantfactory.h"
#include "conditions.h"
#include "conditionseditdialog.h"
#include "quickref.h"
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QMouseEvent>
#include <QTextEdit>
#include <QMenu>
#include <QMessageBox>
#include <QLabel>
#include <QGridLayout>
#include <QDebug>

CharacterTemplateFrame::CharacterTemplateFrame(OptionsContainer* options, QWidget *parent) :
    CampaignObjectFrame(parent),
    ui(new Ui::CharacterTemplateFrame),
    _uiWidget(nullptr),
    _options(options),
    _character(nullptr),
    _mouseDown(false),
    _reading(false),
    _rotation(0),
    _currentToken(0),
    _heroForgeToken(),
    _conditionGrid(nullptr)
{
    ui->setupUi(this);

    connect(ui->btnEditConditions, &QAbstractButton::clicked, this, &CharacterTemplateFrame::editConditions);
    connect(ui->btnRemoveConditions, &QAbstractButton::clicked, this, &CharacterTemplateFrame::clearConditions);
    connect(ui->btnEditIcon, &QAbstractButton::clicked, this, &CharacterTemplateFrame::editCharacterIcon);
    connect(ui->btnSync_2, &QAbstractButton::clicked, this, &CharacterTemplateFrame::syncDndBeyond);
    enableDndBeyondSync(false);
    connect(ui->btnHeroForge_2, &QAbstractButton::clicked, this, &CharacterTemplateFrame::importHeroForge);
    connect(ui->btnPreviousToken, &QAbstractButton::clicked, this, &CharacterTemplateFrame::handlePreviousToken);
    connect(ui->btnAddToken, &QAbstractButton::clicked, this, &CharacterTemplateFrame::handleAddToken);
    connect(ui->btnReload, &QAbstractButton::clicked, this, &CharacterTemplateFrame::handleReloadImage);
    connect(ui->btnClear, &QAbstractButton::clicked, this, &CharacterTemplateFrame::handleClearImage);
    connect(ui->btnNextToken, &QAbstractButton::clicked, this, &CharacterTemplateFrame::handleNextToken);

}

CharacterTemplateFrame::~CharacterTemplateFrame()
{
    delete ui;
}

void CharacterTemplateFrame::activateObject(CampaignObjectBase* object, PublishGLRenderer* currentRenderer)
{
    Q_UNUSED(currentRenderer);

    Characterv2* character = dynamic_cast<Characterv2*>(object);
    if(!character)
        return;

    if(_character != nullptr)
    {
        qDebug() << "[CharacterTemplateFrame] ERROR: New character object activated without deactivating the existing character object first!";
        deactivateObject();
    }

    setCharacter(character);
    connect(_character, &Characterv2::nameChanged, this, &CharacterTemplateFrame::updateCharacterName);

    emit checkableChanged(false);
    emit setPublishEnabled(true, false);
}

void CharacterTemplateFrame::deactivateObject()
{
    if(!_character)
    {
        qDebug() << "[CharacterTemplateFrame] WARNING: Invalid (nullptr) character object deactivated!";
        return;
    }

    disconnect(_character, &Characterv2::nameChanged, this, &CharacterTemplateFrame::updateCharacterName);
    setCharacter(nullptr);
}

void CharacterTemplateFrame::setCharacter(Characterv2* character)
{
    if((_character == character) || (!CombatantFactory::Instance()))
        return;

    if(_character)
        CombatantFactory::Instance()->disconnectWidget(_uiWidget);

    _character = character;
    readCharacterData();
    emit characterChanged();
}

void CharacterTemplateFrame::setHeroForgeToken(const QString& token)
{
    _heroForgeToken = token;
}

void CharacterTemplateFrame::loadCharacterUITemplate(const QString& templateFile)
{
    if(!CombatantFactory::Instance())
        return;

    QString absoluteTemplateFile = TemplateFactory::getAbsoluteTemplateFile(templateFile);
    if(absoluteTemplateFile.isEmpty())
    {
        qDebug() << "[CharacterTemplateFrame] ERROR: UI Template File " << templateFile << " could not be found!";
        return;
    }

    qDebug() << "[CharacterTemplateFrame] Reading Character UI Template File " << absoluteTemplateFile;

    if(absoluteTemplateFile == _uiFilename)
    {
        qDebug() << "[CharacterTemplateFrame] UI Template File " << absoluteTemplateFile << " already loaded, no further action required";
        return;
    }

    QWidget* newWidget = TemplateFactory::loadUITemplate(absoluteTemplateFile);
    if(!newWidget)
    {
        qDebug() << "[CharacterTemplateFrame] ERROR: UI Template File " << templateFile << " could not be loaded!";
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

    if(_character)
        readCharacterData();
}

void CharacterTemplateFrame::publishClicked(bool checked)
{
    Q_UNUSED(checked);
    handlePublishClicked();
}

void CharacterTemplateFrame::setRotation(int rotation)
{
    if(_rotation != rotation)
    {
        _rotation = rotation;
        emit rotationChanged(rotation);
    }
}

void CharacterTemplateFrame::setBackgroundColor(const QColor& color)
{
    if(_character)
        _character->setBackgroundColor(color);
}

bool CharacterTemplateFrame::eventFilter(QObject *object, QEvent *event)
{
    if((event) && (event->type() == QEvent::MouseButtonPress))
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if(mouseEvent->button() == Qt::RightButton)
        {
            QMenu* popupMenu = new QMenu();

            QAction* addItem = new QAction(QString("Add..."), popupMenu);
            connect(addItem, &QAction::triggered, this, [=](){ this->handleAddResource(dynamic_cast<QWidget*>(object), this->_character); });
            popupMenu->addAction(addItem);

            QScrollArea* scrollArea = dynamic_cast<QScrollArea*>(object);
            if(!scrollArea)
            {
                QAction* removeItem = new QAction(QString("Remove..."), popupMenu);
                connect(removeItem, &QAction::triggered, this, [=](){ this->handleRemoveResource(dynamic_cast<QWidget*>(object), this->_character); });
                popupMenu->addAction(removeItem);
            }

            popupMenu->exec(mouseEvent->globalPosition().toPoint());
            popupMenu->deleteLater();
        }
    }

    if(localEventFilter(object, event))
        return true;
    else
        return CampaignObjectFrame::eventFilter(object, event);
}

void CharacterTemplateFrame::mousePressEvent(QMouseEvent * event)
{
    Q_UNUSED(event);
    if((_character) && (ui->lblIcon->frameGeometry().contains(event->pos())))
        ui->lblIcon->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _mouseDown = true;
}

void CharacterTemplateFrame::mouseReleaseEvent(QMouseEvent * event)
{
    if(!_mouseDown)
        return;

    ui->lblIcon->setFrameStyle(QFrame::Panel | QFrame::Raised);
    _mouseDown = false;

    if((!_character) || (!ui->lblIcon->frameGeometry().contains(event->pos())))
        return;

    QString filename = QFileDialog::getOpenFileName(this, QString("Select New Image..."));
    if(filename.isEmpty())
        return;

    _character->addIcon(filename);
    setTokenIndex(_character->getIconCount() - 1);
}

QObject* CharacterTemplateFrame::getFrameObject()
{
    return this;
}

void CharacterTemplateFrame::readCharacterData()
{
    if((!_character) || (!_uiWidget) || (!CombatantFactory::Instance()))
        return;

    _reading = true;

    CombatantFactory::Instance()->readObjectData(_uiWidget, _character, this, this);

    emit backgroundColorChanged(_character->getBackgroundColor());
    _currentToken = 0;
    setTokenIndex(0);
    enableDndBeyondSync(_character->getDndBeyondID() != -1);
    updateConditionLayout();

    _reading = false;
}

void CharacterTemplateFrame::handlePublishClicked()
{
    if(!_character)
        return;

    QImage iconImg;
    QString iconFile = _character->getIcon(_currentToken);
    if((!iconFile.isEmpty()) && (!iconImg.load(iconFile)))
        iconImg = _character->getIconPixmap(DMHelper::PixmapSize_Full, _currentToken).toImage();

    if(iconImg.isNull())
        return;

    if(_rotation != 0)
        iconImg = iconImg.transformed(QTransform().rotate(_rotation), Qt::SmoothTransformation);

    emit publishCharacterImage(iconImg);
}

void CharacterTemplateFrame::editCharacterIcon()
{
    // Use the TokenEditDialog to edit the character icon
    if((!_character) || (!_options))
        return;

    TokenEditDialog* dlg = new TokenEditDialog(_character->getIconPixmap(DMHelper::PixmapSize_Full, _currentToken).toImage(),
                                               *_options,
                                               1.0,
                                               QPoint(),
                                               false,
                                               this);
    if(dlg->exec() == QDialog::Accepted)
    {
        QImage newToken = dlg->getFinalImage();
        if(newToken.isNull())
            return;

        QString currentFile = _character->getIcon(_currentToken);
        QString tokenPath = QFileDialog::getExistingDirectory(this, tr("Select Token Directory"), currentFile.isEmpty() ? QString() : QFileInfo(currentFile).absolutePath());
        if(tokenPath.isEmpty())
            return;

        QDir tokenDir(tokenPath);

        int fileIndex = 1;
        QString tokenFile = _character->getName() + QString(".png");
        while(tokenDir.exists(tokenFile))
            tokenFile = _character->getName() + QString::number(fileIndex++) + QString(".png");

        QString finalTokenPath = tokenDir.absoluteFilePath(tokenFile);
        newToken.save(finalTokenPath);

        if(_character->getIconCount() > 0 && _currentToken < _character->getIconCount())
            _character->setIcon(_currentToken, finalTokenPath);
        else
            _character->addIcon(finalTokenPath);

        setTokenIndex(_currentToken);

        if(dlg->getEditor())
            dlg->getEditor()->applyEditorToOptions(*_options);

    }

    dlg->deleteLater();
}

void CharacterTemplateFrame::syncDndBeyond()
{
    if(!_character)
        return;

    CharacterImporter* importer = new CharacterImporter();
    connect(importer, &CharacterImporter::characterImported, this, &CharacterTemplateFrame::readCharacterData);
    connect(importer, &CharacterImporter::characterImported, importer, &CharacterImporter::deleteLater);
    connect(this, &CharacterTemplateFrame::characterChanged, importer, &CharacterImporter::campaignChanged);
    importer->updateCharacter(_character);
    //importer->deleteLater();
}

void CharacterTemplateFrame::importHeroForge()
{
    if(!_character)
        return;

    QString token = _heroForgeToken;
    if(token.isEmpty())
    {
        token = QInputDialog::getText(this, QString("Enter Hero Forge Access Key"), QString("Please enter your Hero Forge Access Key. You can find this in your Hero Forge account information."));
        if(!token.isEmpty())
        {
            if(QMessageBox::question(this,
                                      QString("Confirm Store Access Key"),
                                      QString("Should DMHelper store your access key for ease of use in the future?") + QChar::LineFeed + QChar::LineFeed + QString("Please note: the Access Key will be stored locally on your computer without encryption, it is possible that other applications will be able to access it.")) == QMessageBox::Yes)
            {
                _heroForgeToken = token;
                emit heroForgeTokenChanged(_heroForgeToken);
            }
        }
    }

    if(token.isEmpty())
    {
        qDebug() << "[CharacterTemplateFrame] No Hero Forge token provided, importer can't be started.";
        return;
    }

    CharacterImportHeroForgeDialog importDialog(token);
    importDialog.resize(width() * 3 / 4, height() * 3 / 4);
    if(importDialog.exec() != QDialog::Accepted)
        return;

    QImage selectedImage = importDialog.getSelectedImage();
    if(selectedImage.isNull())
        return;

    QString filename = QFileDialog::getSaveFileName(this, QString("Choose a filename for the selected token"), importDialog.getSelectedName() + QString(".png"));
    if(filename.isEmpty())
        return;

    if(!selectedImage.save(filename))
        return;

    _character->setIcon(filename);
    loadCharacterImage();
}

void CharacterTemplateFrame::updateCharacterName()
{
    if(!_character)
        return;

    // TODO: Implement me!
    //ui->edtName->setText(_character->getName());
}

void CharacterTemplateFrame::handlePreviousToken()
{
    setTokenIndex(_currentToken - 1);
}

void CharacterTemplateFrame::handleNextToken()
{
    setTokenIndex(_currentToken + 1);
}

void CharacterTemplateFrame::handleAddToken()
{
    if(!_character)
        return;

    QString filename = QFileDialog::getOpenFileName(this, QString("Select Token Image..."));
    if(filename.isEmpty())
        return;

    _character->addIcon(filename);
    setTokenIndex(_character->getIconCount() - 1);
}

void CharacterTemplateFrame::handleReloadImage()
{
    if(!_character)
        return;

    _character->refreshIconPixmaps();
    setTokenIndex(_currentToken);
}

void CharacterTemplateFrame::handleClearImage()
{
    if(!_character)
        return;

    int iconCount = _character->getIconCount();
    if(iconCount <= 0)
        return;

    if(QMessageBox::question(this,
                              QString("Confirm Delete"),
                              QString("Are you sure you want to remove this token image?")) != QMessageBox::Yes)
        return;

    _character->removeIcon(_currentToken);

    if(_currentToken >= _character->getIconCount())
        _currentToken = _character->getIconCount() - 1;
    if(_currentToken < 0)
        _currentToken = 0;

    setTokenIndex(_currentToken);
}

void CharacterTemplateFrame::loadCharacterImage()
{
    if(_character)
        ui->lblIcon->setPixmap(_character->getIconPixmap(DMHelper::PixmapSize_Showcase, _currentToken));
}

void CharacterTemplateFrame::enableDndBeyondSync(bool enabled)
{
    ui->btnSync_2->setVisible(enabled);
    ui->lblDndBeyondLink->setVisible(enabled);

    if(_character)
    {
        QString characterUrl = QString("https://www.dndbeyond.com/characters/") + QString::number(_character->getDndBeyondID());
        QString fullLink = QString("<a href=\"") + characterUrl + QString("\">") + characterUrl + QString("</a>");
        qDebug() << "[CharacterTemplateFrame] Setting Dnd Beyond link for character to: " << fullLink;
        ui->lblDndBeyondLink->setText(fullLink);
    }
}

void CharacterTemplateFrame::setTokenIndex(int index)
{
    if(!_character)
        return;

    int count = _character->getIconCount();
    if(count <= 0)
    {
        _currentToken = 0;
        ui->lblIcon->setPixmap(QPixmap());
        ui->btnPreviousToken->setEnabled(false);
        ui->btnNextToken->setEnabled(false);
        ui->btnClear->setEnabled(false);
        ui->btnEditIcon->setEnabled(false);
        return;
    }

    if(index < 0)
        index = 0;
    if(index >= count)
        index = count - 1;

    _currentToken = index;

    ui->btnPreviousToken->setEnabled((count > 1) && (index > 0));
    ui->btnNextToken->setEnabled((count > 1) && (index < count - 1));
    ui->btnClear->setEnabled(true);
    ui->btnEditIcon->setEnabled(true);

    loadCharacterImage();
}

void CharacterTemplateFrame::editConditions()
{
    if(!_character)
        return;

    ConditionsEditDialog dlg(this);
    dlg.setConditionList(_character->getConditionList());
    int result = dlg.exec();
    if(result == QDialog::Accepted)
    {
        if(dlg.getConditionList() != _character->getConditionList())
        {
            _character->setConditionList(dlg.getConditionList());
            updateConditionLayout();
        }
    }
}

void CharacterTemplateFrame::clearConditions()
{
    if(!_character)
        return;

    _character->clearConditions();
    updateConditionLayout();
}

void CharacterTemplateFrame::updateConditionLayout()
{
    clearConditionGrid();

    if(!_character)
        return;

    _conditionGrid = new QGridLayout;
    _conditionGrid->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    _conditionGrid->setContentsMargins(2, 2, 2, 2);
    _conditionGrid->setSpacing(2);
    ui->scrollAreaWidgetContents_2->setLayout(_conditionGrid);

    QStringList conditionList = _character->getConditionList();
    for(const QString& condId : conditionList)
        addCondition(condId);

    int spacingColumn = _conditionGrid->columnCount();
    _conditionGrid->addItem(new QSpacerItem(20, 40, QSizePolicy::Expanding), 0, spacingColumn);

    for(int i = 0; i < spacingColumn; ++i)
        _conditionGrid->setColumnStretch(i, 1);
    _conditionGrid->setColumnStretch(spacingColumn, 10);

    update();
}

void CharacterTemplateFrame::clearConditionGrid()
{
    if(!_conditionGrid)
        return;

    QLayoutItem* child = nullptr;
    while((child = _conditionGrid->takeAt(0)) != nullptr)
    {
        delete child->widget();
        delete child;
    }

    delete _conditionGrid;
    _conditionGrid = nullptr;

    ui->scrollAreaWidgetContents_2->update();
}

void CharacterTemplateFrame::addCondition(const QString& conditionId)
{
    if(!_conditionGrid)
        return;

    Conditions* conds = Conditions::activeConditions();
    if(!conds)
        return;

    QString iconPath = conds->getConditionIconPath(conditionId);
    QLabel* conditionLabel = new QLabel(this);
    if(!iconPath.isEmpty())
        conditionLabel->setPixmap(QPixmap(iconPath).scaled(40, 40));

    QString conditionText = QString("<b>") + conds->getConditionDescription(conditionId) + QString("</b>");
    if(QuickRef::Instance())
    {
        QuickRefData* conditionData = QuickRef::Instance()->getData(QString("Condition"), 0, conds->getConditionTitle(conditionId));
        if(conditionData)
            conditionText += QString("<p>") + conditionData->getOverview();
    }
    conditionLabel->setToolTip(conditionText);

    int columnCount = (ui->scrollAreaWidgetContents_2->width() - 2) / (40 + 2);
    if(columnCount < 1)
        columnCount = 1;
    int row = _conditionGrid->count() / columnCount;
    int column = _conditionGrid->count() % columnCount;

    _conditionGrid->addWidget(conditionLabel, row, column);
}
