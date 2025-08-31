#include "bestiarytemplatedialog.h"
#include "ui_bestiarytemplatedialog.h"
#include "monsterclassv2.h"
#include "monsterfactory.h"
#include "bestiary.h"
#include "tokeneditdialog.h"
#include "optionscontainer.h"
#include "bestiarypopulatetokensdialog.h"
#include <QMouseEvent>
#include <QTextEdit>
#include <QMenu>
#include <QShortcut>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QAbstractItemView>
#include <QCompleter>
#include <QDebug>

BestiaryTemplateDialog::BestiaryTemplateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BestiaryTemplateDialog),
    _uiWidget(nullptr),
    _monster(nullptr)
{
    ui->setupUi(this);

    connect(ui->btnLeft, &QPushButton::clicked, this, &BestiaryTemplateDialog::previousMonster);
    connect(ui->btnRight, &QPushButton::clicked, this, &BestiaryTemplateDialog::nextMonster);
    connect(ui->btnNewMonster, &QPushButton::clicked, this, &BestiaryTemplateDialog::createNewMonster);
    connect(ui->btnDeleteMonster, &QPushButton::clicked, this, &BestiaryTemplateDialog::deleteCurrentMonster);
    connect(ui->cmbSearch, &QComboBox::textActivated, this, static_cast<void (BestiaryTemplateDialog::*)(const QString&)>(&BestiaryTemplateDialog::setMonster));
    connect(ui->framePublish, &PublishButtonFrame::clicked, this, &BestiaryTemplateDialog::handlePublishButton);
    connect(ui->framePublish, &PublishButtonFrame::colorChanged, this, &BestiaryTemplateDialog::handleBackgroundColorChanged);
    QShortcut* publishShortcut = new QShortcut(QKeySequence(tr("Ctrl+P", "Publish")), this);
    connect(publishShortcut, &QShortcut::activated, ui->framePublish, &PublishButtonFrame::clickPublish);

    connect(ui->btnPreviousToken, &QPushButton::clicked, this, &BestiaryTemplateDialog::handlePreviousToken);
    connect(ui->btnAddToken, &QPushButton::clicked, this, &BestiaryTemplateDialog::handleAddToken);

    connect(ui->btnEditIcon, &QPushButton::clicked, this, &BestiaryTemplateDialog::handleEditToken);
    connect(ui->btnSearchToken, &QPushButton::clicked, this, &BestiaryTemplateDialog::handleSearchToken);
    connect(ui->btnReload, &QPushButton::clicked, this, &BestiaryTemplateDialog::handleReloadImage);
    connect(ui->btnClear, &QPushButton::clicked, this, &BestiaryTemplateDialog::handleClearImage);
    connect(ui->btnNextToken, &QPushButton::clicked, this, &BestiaryTemplateDialog::handleNextToken);

    connect(ui->btnPopulateTokens, &QPushButton::clicked, this, &BestiaryTemplateDialog::handlePopulateTokens);

    ui->cmbSearch->view()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    // Create a completer and attach it to the search combo box
    QCompleter *completer = new QCompleter(ui->cmbSearch->model(), this);
    completer->setFilterMode(Qt::MatchContains);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    ui->cmbSearch->setCompleter(completer);
}

BestiaryTemplateDialog::~BestiaryTemplateDialog()
{
    delete ui;
}

void BestiaryTemplateDialog::loadMonsterUITemplate(const QString& templateFile)
{
    QString absoluteTemplateFile = TemplateFactory::getAbsoluteTemplateFile(templateFile);
    if(absoluteTemplateFile.isEmpty())
    {
        qDebug() << "[BestiaryTemplateDialog] ERROR: UI Template File " << templateFile << " could not be found!";
        return;
    }

    qDebug() << "[BestiaryTemplateDialog] Reading Monster UI Template File " << absoluteTemplateFile;

    if(absoluteTemplateFile == _uiFilename)
    {
        qDebug() << "[BestiaryTemplateDialog] UI Template File " << absoluteTemplateFile << " already loaded, no further action required";
        return;
    }

    QWidget* newWidget = TemplateFactory::loadUITemplate(absoluteTemplateFile);
    if(!newWidget)
    {
        qDebug() << "[BestiaryTemplateDialog] ERROR: UI Template File " << templateFile << " could not be loaded!";
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

    if(_monster)
        MonsterFactory::Instance()->readObjectData(_uiWidget, _monster, this, this);
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
    if((_monster == monster) || (!MonsterFactory::Instance()))
        return;

    qDebug() << "[BestiaryTemplateDialog] Set Monster to " << (monster ? monster->getStringValue("name") : QString("nullptr"));

    if(_monster)
        MonsterFactory::Instance()->disconnectWidget(_uiWidget);

    _monster = monster;
    _edit = edit;

    if(_monster)
    {
        MonsterFactory::Instance()->readObjectData(_uiWidget, _monster, this, this);

        if(ui->cmbSearch->currentText() != _monster->getStringValue("name"))
            ui->cmbSearch->setCurrentText(_monster->getStringValue("name"));

        if(_monster->getIconCount() == 0)
            _monster->searchForIcons();
        setTokenIndex(0);

        ui->framePublish->setColor(_monster->getBackgroundColor());
    }

    emit monsterChanged();
}

void BestiaryTemplateDialog::setMonster(const QString& monsterName, bool edit)
{
    setMonster(Bestiary::Instance()->getMonsterClass(monsterName), edit);
}

void BestiaryTemplateDialog::setMonster(const QString& monsterName)
{
    setMonster(Bestiary::Instance()->getMonsterClass(monsterName), true);
}

void BestiaryTemplateDialog::createNewMonster()
{
    qDebug() << "[BestiaryTemplateDialog] Creating a new monster...";

    bool ok;
    QString monsterName = QInputDialog::getText(this, QString("Enter New Monster Name"), QString("New Monster"), QLineEdit::Normal, QString(), &ok);
    if((!ok)||(monsterName.isEmpty()))
    {
        qDebug() << "[BestiaryTemplateDialog] New monster not created because the monster name dialog was cancelled";
        return;
    }

    MonsterClassv2* monsterClass;
    if(Bestiary::Instance()->exists(monsterName))
    {
        monsterClass = Bestiary::Instance()->getMonsterClass(monsterName);
        qDebug() << "[BestiaryTemplateDialog] New Monster already exists, selecting new monster: " << monsterClass;
    }
    else
    {
        monsterClass = new MonsterClassv2(monsterName);

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
                    qDebug() << "[BestiaryTemplateDialog] New monster not created because the select template monster dialog was cancelled";
                    delete monsterClass;
                    return;
                }

                MonsterClassv2* templateClass = Bestiary::Instance()->getMonsterClass(templateName);
                if(!templateClass)
                {
                    qDebug() << "[BestiaryTemplateDialog] New monster not created because not able to find selected template monster: " << templateName;
                    delete monsterClass;
                    return;
                }

                monsterClass->cloneMonster(*templateClass);
                monsterClass->setStringValue("name", monsterName);
            }
        }

        Bestiary::Instance()->insertMonsterClass(monsterClass);
        qDebug() << "[BestiaryTemplateDialog] New Monster created: " << monsterClass;
    }

    setMonster(monsterClass);
    show();
    activateWindow();
}

void BestiaryTemplateDialog::deleteCurrentMonster()
{
    if(!_monster)
        return;

    qDebug() << "[BestiaryTemplateDialog] Deleting monster: " << _monster->getStringValue("name");

    QMessageBox::StandardButton confirm = QMessageBox::critical(this,
                                                                QString("Delete Monster"),
                                                                QString("Are you sure you want to delete the monster ") + _monster->getStringValue("name"),
                                                                QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No));
    if(confirm == QMessageBox::No)
    {
        qDebug() << "[BestiaryTemplateDialog] Delete of monster cancelled by user: " << _monster->getStringValue("name");
        return;
    }

    MonsterClassv2* removedMonster = _monster;

    MonsterClassv2* nextClass = Bestiary::Instance()->getNextMonsterClass(_monster);
    if(nextClass)
    {
        setMonster(nextClass);
    }
    else
    {
        MonsterClassv2* previousClass = Bestiary::Instance()->getPreviousMonsterClass(_monster);
        if(previousClass)
            setMonster(previousClass);
        else
            setMonster(Bestiary::Instance()->getFirstMonsterClass());
    }

    Bestiary::Instance()->removeMonsterClass(removedMonster);
}

void BestiaryTemplateDialog::previousMonster()
{
    MonsterClassv2* previousClass = Bestiary::Instance()->getPreviousMonsterClass(_monster);
    if(previousClass)
        setMonster(previousClass);
}

void BestiaryTemplateDialog::nextMonster()
{
    MonsterClassv2* nextClass = Bestiary::Instance()->getNextMonsterClass(_monster);
    if(nextClass)
        setMonster(nextClass);
}

void BestiaryTemplateDialog::dataChanged()
{
    QString previousMonster = ui->cmbSearch->currentText();
    setMonster(nullptr);

    QList<QString> monsterList = Bestiary::Instance()->getMonsterList();

    disconnect(ui->cmbSearch, &QComboBox::textActivated, this, static_cast<void (BestiaryTemplateDialog::*)(const QString&)>(&BestiaryTemplateDialog::setMonster));

    ui->cmbSearch->clear();
    ui->cmbSearch->addItems(Bestiary::Instance()->getMonsterList());

    connect(ui->cmbSearch, &QComboBox::textActivated, this, static_cast<void (BestiaryTemplateDialog::*)(const QString&)>(&BestiaryTemplateDialog::setMonster));

    if(!previousMonster.isEmpty())
    {
        int index = ui->cmbSearch->findText(previousMonster);
        if(index >= 0)
            ui->cmbSearch->setCurrentIndex(index);
    }
    else
    {
        setMonster(ui->cmbSearch->currentText());
    }
}

void BestiaryTemplateDialog::monsterRenamed()
{
    if(!_monster)
        return;

    QLineEdit* nameEdit = getValueEdit("name");
    if(!nameEdit)
        return;

    if(nameEdit->text() == _monster->getStringValue("name"))
        return;

    Bestiary::Instance()->renameMonster(_monster, nameEdit->text());
}

void BestiaryTemplateDialog::handlePublishButton()
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

void BestiaryTemplateDialog::handleBackgroundColorChanged(const QColor& color)
{
    if(_monster)
        _monster->setBackgroundColor(color);
}

void BestiaryTemplateDialog::handlePreviousToken()
{
    setTokenIndex(_currentToken - 1);
}

void BestiaryTemplateDialog::handleAddToken()
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

void BestiaryTemplateDialog::handleEditToken()
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
        QString tokenFile = _monster->getStringValue("name") + QString(".png");
        while(tokenDir.exists(tokenFile))
            tokenFile = _monster->getStringValue("name") + QString::number(fileIndex++) + QString(".png");

        QString finalTokenPath = tokenDir.absoluteFilePath(tokenFile);
        newToken.save(finalTokenPath);

        _monster->setIcon(_currentToken, finalTokenPath);
        loadMonsterImage();

        if(dlg->getEditor())
            dlg->getEditor()->applyEditorToOptions(*_options);

    }

    dlg->deleteLater();
}

void BestiaryTemplateDialog::handleSearchToken()
{
    if((!_monster) || (!_options))
        return;

    BestiaryFindTokenDialog* dlg = new BestiaryFindTokenDialog(_monster->getStringValue("name"), _searchString, *_options);
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

void BestiaryTemplateDialog::handleReloadImage()
{
    if(!_monster)
        return;

    _monster->searchForIcons();
    _monster->refreshIconPixmaps();
    setTokenIndex(_currentToken);
}

void BestiaryTemplateDialog::handleClearImage()
{
    if(!_monster)
        return;

    QString currentIconPath = Bestiary::Instance()->getDirectory().absoluteFilePath(_monster->getIcon(_currentToken));
    if((currentIconPath.isEmpty()) || (!QFile::exists(currentIconPath)))
    {
        qDebug() << "[BestiaryTemplateDialog] Unable to find token file to remove from monster: " << _monster->getStringValue("name") << ", " << currentIconPath;
        return;
    }

    // Ask if the app should remove the file
    QMessageBox::StandardButton result = QMessageBox::question(this, tr("Delete Image"), tr("Do you want to also delete the token file from the disk?\n\n") + currentIconPath);
    if(result == QMessageBox::Yes)
    {
        qDebug() << "[BestiaryTemplateDialog] Removing token file for monster: " << _monster->getStringValue("name") << ", " << currentIconPath;
        QFile::remove(currentIconPath);
    }

    _monster->removeIcon(_currentToken);
    if(_monster->getIconCount() == 0)
        ui->lblIcon->clear();
    else
        setTokenIndex(_currentToken > 0 ? _currentToken - 1 : 0);
}

void BestiaryTemplateDialog::handleNextToken()
{
    setTokenIndex(_currentToken + 1);
}

void BestiaryTemplateDialog::handlePopulateTokens()
{
    if(!_options)
        return;

    BestiaryPopulateTokensDialog* dlg = new BestiaryPopulateTokensDialog(*_options);
    dlg->exec();
    dlg->deleteLater();

    loadMonsterImage();
}

void BestiaryTemplateDialog::loadMonsterImage()
{
    ui->lblIcon->setPixmap(_monster->getIconPixmap(DMHelper::PixmapSize_Showcase, _currentToken));
}

bool BestiaryTemplateDialog::eventFilter(QObject* object, QEvent* event)
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

    if(localEventFilter(object, event))
        return true;
    else
        return QDialog::eventFilter(object, event);
}

void BestiaryTemplateDialog::mousePressEvent(QMouseEvent* event)
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

void BestiaryTemplateDialog::mouseReleaseEvent(QMouseEvent* event)
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

void BestiaryTemplateDialog::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);
    qDebug() << "[BestiaryTemplateDialog] Bestiary Dialog shown";
    connect(Bestiary::Instance(), &Bestiary::changed, this, &BestiaryTemplateDialog::dataChanged);
    setMonster(ui->cmbSearch->currentText());
    QDialog::showEvent(event);
}

void BestiaryTemplateDialog::hideEvent(QHideEvent* event)
{
    Q_UNUSED(event);
    qDebug() << "[BestiaryTemplateDialog] Bestiary Dialog hidden";
    QDialog::hideEvent(event);
    emit dialogClosed();
}

void BestiaryTemplateDialog::focusOutEvent(QFocusEvent* event)
{
    Q_UNUSED(event);

    qDebug() << "[BestiaryTemplateDialog] Bestiary Dialog lost focus";
    QDialog::focusOutEvent(event);

    emit dialogClosed();
}

QObject* BestiaryTemplateDialog::getFrameObject()
{
    return this;
}

void BestiaryTemplateDialog::postLoadConfiguration(QWidget* owner, QWidget* uiWidget)
{
    // Activate hyperlinks for any included text edits
    if(_uiWidget)
    {
        QList<QLineEdit*> lineEdits = _uiWidget->findChildren<QLineEdit*>();
        for(QLineEdit* lineEdit : lineEdits)
        {
            if(!lineEdit)
                continue;

            QString keyString = lineEdit->property(TemplateFactory::TEMPLATE_PROPERTY).toString();
            if(keyString == QString("name"))
                connect(lineEdit, &QLineEdit::editingFinished, this, &BestiaryTemplateDialog::monsterRenamed);
        }
    }

    TemplateFrame::postLoadConfiguration(owner, uiWidget);
}

void BestiaryTemplateDialog::createTokenFiles(BestiaryFindTokenDialog* dialog)
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

    QString tokenFile = _monster->getStringValue("name") + QString(".png");
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
                    tokenFile = _monster->getStringValue("name") + QString::number(fileIndex++) + QString(".png");

                image.save(tokenDir.absoluteFilePath(tokenFile));
            }
        }
    }

    _monster->searchForIcons();
    setTokenIndex(_monster->getIconList().indexOf(tokenFile));

    if(editDialog)
        editDialog->deleteLater();
}

QString BestiaryTemplateDialog::selectToken()
{
    return QFileDialog::getOpenFileName(this, QString("Select New Image..."));
}

void BestiaryTemplateDialog::setTokenIndex(int index)
{
    if(!_monster)
    {
        ui->btnPreviousToken->setEnabled(false);
        ui->btnNextToken->setEnabled(false);
        ui->btnClear->setEnabled(false);
        return;
    }

    if((index < 0) || (index >= _monster->getIconCount()))
    {
        ui->lblIcon->setPixmap(QPixmap());
        ui->btnPreviousToken->setEnabled(false);
        ui->btnNextToken->setEnabled(false);
        return;
    }

    _currentToken = index;
    loadMonsterImage();
    ui->btnPreviousToken->setEnabled(_currentToken > 0);
    ui->btnNextToken->setEnabled(_currentToken < _monster->getIconCount() - 1);
    ui->btnClear->setEnabled(_monster->getIconCount() > 0);
    ui->btnPreviousToken->setEnabled(_monster->getIconCount() > 1);
    ui->btnNextToken->setEnabled(_monster->getIconCount() > 1);
}

QLineEdit* BestiaryTemplateDialog::getValueEdit(const QString& key)
{
    if((!_uiWidget) || (key.isEmpty()))
        return nullptr;

    // Activate hyperlinks for any included text edits
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
