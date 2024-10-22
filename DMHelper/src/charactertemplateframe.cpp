#include "charactertemplateframe.h"
#include "ui_charactertemplateframe.h"
#include "charactertemplateresourcelayout.h"
#include "characterimporter.h"
#include "characterimportheroforgedialog.h"
#include "tokeneditdialog.h"
#include "optionscontainer.h"
#include "combatantfactory.h"
#include "rulefactory.h"
#include <QUiLoader>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QMouseEvent>
#include <QStandardPaths>
#include <QTextEdit>
#include <QCheckBox>
#include <QMenu>
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
    _heroForgeToken()
{
    ui->setupUi(this);

    connect(ui->btnEditIcon, &QAbstractButton::clicked, this, &CharacterTemplateFrame::editCharacterIcon);
    connect(ui->btnSync, &QAbstractButton::clicked, this, &CharacterTemplateFrame::syncDndBeyond);
    enableDndBeyondSync(false);
    connect(ui->btnHeroForge, &QAbstractButton::clicked, this, &CharacterTemplateFrame::importHeroForge);

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
    writeCharacterData();
    setCharacter(nullptr);
}

void CharacterTemplateFrame::setCharacter(Characterv2* character)
{
    if(_character == character)
        return;

    if(_character)
        disconnectTemplate();

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
    if(!RuleFactory::Instance())
    {
        qDebug() << "[CharacterTemplateFrame] ERROR: No rule factory exists, cannot load the character UI template file: " << templateFile;
        return;
    }

    // Try our best to load the given character template file
    QString appFile;
    if(QFileInfo(templateFile).isRelative())
    {
        QDir relativeDir = RuleFactory::Instance()->getRulesetDir();
        appFile = relativeDir.absoluteFilePath(templateFile);
        if(!QFileInfo::exists(appFile))
        {
#ifdef Q_OS_MAC
            QDir fileDirPath(QCoreApplication::applicationDirPath());
            fileDirPath.cdUp();
            appFile = fileDirPath.path() + QString("/Resources/") + templateFile;
#else
            QDir fileDirPath(QCoreApplication::applicationDirPath());
            appFile = fileDirPath.path() + QString("/resources/") + templateFile;
#endif
            if(!QFileInfo::exists(appFile))
            {
                qDebug() << "[CharacterTemplateFrame] ERROR: Relative Character UI Template File not found: " << templateFile;
                return;
            }
        }
    }
    else
    {
        appFile = templateFile;
        if(!QFileInfo::exists(appFile))
        {
            qDebug() << "[CharacterTemplateFrame] ERROR: Absolute Character UI Template File not found: " << templateFile;
            return;
        }
    }

    QUiLoader loader;
    QFile file(appFile);
    file.open(QFile::ReadOnly);
    QWidget* newWidget = loader.load(&file, this);
    file.close();

    if(!newWidget)
    {
        qDebug() << "[CharacterTemplateFrame] ERROR: UI Template File " << appFile << " could not be loaded: " << loader.errorString();
        return;
    }

    if(_character)
        disconnectTemplate();

    delete _uiWidget;
    if(ui->scrollAreaWidgetContents->layout())
        delete ui->scrollAreaWidgetContents->layout();

    _uiWidget = newWidget;

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    layout->addWidget(_uiWidget);
    ui->scrollAreaWidgetContents->setLayout(layout);

    // Activate hyperlinks for any included text edits
    QList<QTextEdit*> textEdits = _uiWidget->findChildren<QTextEdit*>();
    for(QTextEdit* edit : textEdits)
    {
        auto &clist = edit->children();
        for(QObject *pObj : clist)
        {
            QString cname = pObj->metaObject()->className();
            if(cname == "QWidgetTextControl")
                pObj->setProperty("openExternalLinks", true);
        }
    }

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

bool CharacterTemplateFrame::eventFilter(QObject *object, QEvent *event)
{
    if((event) && (event->type() == QEvent::MouseButtonPress))
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if(mouseEvent->button() == Qt::RightButton)
        {
            QMenu* popupMenu = new QMenu();

            QAction* addItem = new QAction(QString("Add..."), popupMenu);
            connect(addItem, &QAction::triggered, this, [=](){ this->handleAddResource(dynamic_cast<QWidget*>(object)); });
            popupMenu->addAction(addItem);

            QScrollArea* scrollArea = dynamic_cast<QScrollArea*>(object);
            if(!scrollArea)
            {
                QAction* removeItem = new QAction(QString("Remove..."), popupMenu);
                connect(removeItem, &QAction::triggered, this, [=](){ this->handleRemoveResource(dynamic_cast<QWidget*>(object)); });
                popupMenu->addAction(removeItem);
            }

            popupMenu->exec(mouseEvent->globalPosition().toPoint());
            popupMenu->deleteLater();
        }
    }

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

    _character->setIcon(filename);
    loadCharacterImage();
}

void CharacterTemplateFrame::disconnectTemplate()
{
    if(!_uiWidget)
        return;

    // Walk through the loaded UI Widget and allocate the appropriate character values to the UI elements
    QList<QLineEdit*> lineEdits = _uiWidget->findChildren<QLineEdit*>();
    for(auto lineEdit : lineEdits)
    {
        if(lineEdit)
            disconnect(lineEdit, &QLineEdit::editingFinished, nullptr, nullptr);
    }

    QList<QTextEdit*> textEdits = _uiWidget->findChildren<QTextEdit*>();
    for(auto textEdit : textEdits)
    {
        if(textEdit)
            disconnect(textEdit, &QTextEdit::textChanged, nullptr, nullptr);
    }
}

void CharacterTemplateFrame::readCharacterData()
{
    if((!_character) || (!_uiWidget) || (!CombatantFactory::Instance()))
        return;

    _reading = true;

    // Walk through the loaded UI Widget and allocate the appropriate character values to the UI elements
    populateWidget(_uiWidget, _character, nullptr);

    QList<QScrollArea*> scrollAreas = _uiWidget->findChildren<QScrollArea*>();
    for(auto scrollArea : scrollAreas)
    {
        if(!scrollArea)
            continue;

        QString keyString = scrollArea->property(CombatantFactory::TEMPLATE_PROPERTY).toString();
        QString widgetString = scrollArea->property(CombatantFactory::TEMPLATE_WIDGET).toString();
        if((!keyString.isEmpty()) && (!widgetString.isEmpty()))
        {
            if(QWidget* oldWidget = scrollArea->takeWidget())
            {
                if(QLayout* oldLayout = oldWidget->layout())
                {
                    QLayoutItem *child;
                    while((child = scrollArea->layout()->takeAt(0)) != nullptr)
                    {
                        if(child->widget())
                            child->widget()->deleteLater();
                        delete child;
                    }
                    delete oldLayout;
                }
                oldWidget->deleteLater();
            }

            scrollArea->setWidgetResizable(true);
            QFrame* scrollWidget = new QFrame;
            QVBoxLayout* scrollLayout = new QVBoxLayout;
            scrollLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
            scrollWidget->setLayout(scrollLayout);
            scrollArea->setWidget(scrollWidget);

            QList<QVariant> listValue = _character->getListValue(keyString);
            if(!listValue.isEmpty())
            {
                for(int i = 0; i < listValue.count(); ++i)
                {
                    QVariant listEntry = listValue.at(i);
                    if(listEntry.isNull())
                        continue;

                    QWidget* newWidget = createResourceWidget(keyString, widgetString);
                    if(!newWidget)
                    {
                        qDebug() << "[CharacterTemplateFrame] ERROR: Unable to create the character widget: " << widgetString << ", for scroll area: " << keyString;
                        return;
                    }

                    QHash<QString, QVariant> hashValue = listEntry.toHash();

                    // Walk through the loaded UI Widget and allocate the appropriate character values to the UI elements
                    populateWidget(newWidget, nullptr, &hashValue, i, keyString);

                    newWidget->installEventFilter(this);
                    scrollLayout->addWidget(newWidget);
                }
            }
            scrollArea->installEventFilter(this);
        }
    }

    loadCharacterImage();
    enableDndBeyondSync(_character->getDndBeyondID() != -1);

    _reading = false;
}

void CharacterTemplateFrame::writeCharacterData()
{
    if((!_character) || (_reading))
        return;

    // TODO: Implement me!
}

void CharacterTemplateFrame::handlePublishClicked()
{
    if(!_character)
        return;

    QImage iconImg;
    QString iconFile = _character->getIconFile();
    if(!iconImg.load(iconFile))
        iconImg = _character->getIconPixmap(DMHelper::PixmapSize_Full).toImage();

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

    TokenEditDialog* dlg = new TokenEditDialog(_character->getIconPixmap(DMHelper::PixmapSize_Full).toImage(),
                                               *_options,
                                               1.0,
                                               QPoint(),
                                               false);
    if(dlg->exec() == QDialog::Accepted)
    {
        QImage newToken = dlg->getFinalImage();
        if(newToken.isNull())
            return;

        QString tokenPath = QFileDialog::getExistingDirectory(this, tr("Select Token Directory"), _character->getIconFile().isEmpty() ? QString() : QFileInfo(_character->getIconFile()).absolutePath());
        if(tokenPath.isEmpty())
            return;

        QDir tokenDir(tokenPath);

        int fileIndex = 1;
        QString tokenFile = _character->getName() + QString(".png");
        while(tokenDir.exists(tokenFile))
            tokenFile = _character->getName() + QString::number(fileIndex++) + QString(".png");

        QString finalTokenPath = tokenDir.absoluteFilePath(tokenFile);
        newToken.save(finalTokenPath);

        _character->setIcon(finalTokenPath);
        loadCharacterImage();

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

void CharacterTemplateFrame::handleLineEditFinished(QLineEdit* lineEdit)
{
    if(!lineEdit)
        return;

    handleEditBoxChange(lineEdit, lineEdit->text());
}

void CharacterTemplateFrame::handleTextEditChanged(QTextEdit* textEdit)
{
    if(!textEdit)
        return;

    handleEditBoxChange(textEdit, textEdit->toHtml());
}

void CharacterTemplateFrame::handleResourceChanged(QFrame* resourceFrame)
{
    if((!_character) || (!resourceFrame))
        return;

    int checkboxCount = 0;
    int checkedCount = 0;
    for(auto child : resourceFrame->findChildren<QCheckBox*>())
    {
        ++checkboxCount;
        if(child->isChecked())
            ++checkedCount;
    }

    QString keyString = resourceFrame->property(CombatantFactory::TEMPLATE_PROPERTY).toString();
    _character->setResourceValue(keyString, ResourcePair(checkedCount, checkboxCount));
}

void CharacterTemplateFrame::handleAddResource(QWidget* widget)
{
    if((!widget) || (!widget->parentWidget()))
        return;

    int widgetIndex = 0;
    QBoxLayout* scrollLayout = nullptr;
    QScrollArea* scrollArea = dynamic_cast<QScrollArea*>(widget);

    if(scrollArea)
    {
        if(scrollArea->widget())
            scrollLayout = dynamic_cast<QBoxLayout*>(scrollArea->widget()->layout());
    }
    else
    {
        scrollLayout = dynamic_cast<QBoxLayout*>(widget->parentWidget()->layout());
        if(!scrollLayout)
            return;

        widgetIndex = scrollLayout->indexOf(widget);
        if(widgetIndex < 0)
            return;

        // First parent is the dynamically created frame withing the scroll area
        if((widget->parentWidget()->parentWidget()) && (widget->parentWidget()->parentWidget()->parentWidget()))
            scrollArea = dynamic_cast<QScrollArea*>(widget->parentWidget()->parentWidget()->parentWidget());
    }

    if((!scrollLayout) || (widgetIndex < 0) || (!scrollArea))
        return;

    QString keyString = scrollArea->property(CombatantFactory::TEMPLATE_PROPERTY).toString();
    QString widgetString = scrollArea->property(CombatantFactory::TEMPLATE_WIDGET).toString();

    QWidget* newWidget = createResourceWidget(keyString, widgetString);
    if(!newWidget)
    {
        qDebug() << "[CharacterTemplateFrame] ERROR: Unable to create the character widget: " << widgetString << ", for scroll area: " << keyString;
        return;
    }

    QHash<QString, QVariant> newHash = _character->createListEntry(keyString, widgetIndex);

    // Walk through the loaded UI Widget and allocate the appropriate character values to the UI elements
    populateWidget(newWidget, nullptr, &newHash, widgetIndex, keyString);

    newWidget->installEventFilter(this);
    scrollLayout->insertWidget(widgetIndex, newWidget);
}

void CharacterTemplateFrame::handleRemoveResource(QWidget* widget)
{
    if((!widget) || (!widget->parentWidget()))
        return;

    QLayout* parentLayout = dynamic_cast<QLayout*>(widget->parentWidget()->layout());
    if(!parentLayout)
        return;

    int widgetIndex = parentLayout->indexOf(widget);
    if(widgetIndex < 0)
        return;

    // First parent is the dynamically created frame withing the scroll area
    QScrollArea* scrollArea = nullptr;
    if((widget->parentWidget()->parentWidget()) && (widget->parentWidget()->parentWidget()->parentWidget()))
        scrollArea = dynamic_cast<QScrollArea*>(widget->parentWidget()->parentWidget()->parentWidget());
    if(scrollArea == nullptr)
        return;

    if(QMessageBox::question(nullptr, tr("Remove Resource"), tr("Are you sure you want to remove this resource?")) != QMessageBox::Yes)
        return;

    QString keyString = scrollArea->property(CombatantFactory::TEMPLATE_PROPERTY).toString();
    QList<QVariant> listValue = _character->getListValue(keyString);
    if(listValue.isEmpty())
        return;

    listValue.remove(widgetIndex);
    _character->setValue(keyString, listValue);

    parentLayout->removeWidget(widget);
    widget->deleteLater();
}

void CharacterTemplateFrame::loadCharacterImage()
{
    if(_character)
        ui->lblIcon->setPixmap(_character->getIconPixmap(DMHelper::PixmapSize_Showcase));
}

void CharacterTemplateFrame::enableDndBeyondSync(bool enabled)
{
    ui->btnSync->setVisible(enabled);
    ui->lblDndBeyondLink->setVisible(enabled);

    if(_character)
    {
        QString characterUrl = QString("https://www.dndbeyond.com/characters/") + QString::number(_character->getDndBeyondID());
        QString fullLink = QString("<a href=\"") + characterUrl + QString("\">") + characterUrl + QString("</a>");
        qDebug() << "[CharacterTemplateFrame] Setting Dnd Beyond link for character to: " << fullLink;
        ui->lblDndBeyondLink->setText(fullLink);
    }
}

QWidget* CharacterTemplateFrame::createResourceWidget(const QString& keyString, const QString& widgetString)
{
    if(widgetString.isEmpty())
        return createResourceWidgetInternal(keyString);

#ifdef Q_OS_MAC
    QDir fileDirPath(QCoreApplication::applicationDirPath());
    fileDirPath.cdUp();
    QString appFile = fileDirPath.path() + QString("/Resources/ui/") + widgetString;
#else
    QDir fileDirPath(QCoreApplication::applicationDirPath());
    QString appFile = fileDirPath.path() + QString("/resources/ui/") + widgetString;
#endif

    if(QFileInfo::exists(appFile))
    {
        return createResourceWidgetFile(appFile);
    }
    else
    {
        qDebug() << "[CharacterTemplateFrame] ERROR: UI Widget Template File not found: " << appFile << ", for the widget name: " << widgetString;
        return createResourceWidgetInternal(keyString);
    }
}

QWidget* CharacterTemplateFrame::createResourceWidgetFile(const QString& widgetFilename)
{
    if(widgetFilename.isEmpty())
        return nullptr;

    QUiLoader loader;
    QFile file(widgetFilename);
    if(!file.open(QFile::ReadOnly))
    {
        qDebug() << "[CharacterTemplateFrame] ERROR: Unable to open UI Widget Template File: " << widgetFilename;
        return nullptr;
    }

    QWidget* newWidget = loader.load(&file, this);
    file.close();

    if(!newWidget)
        qDebug() << "[CharacterTemplateFrame] ERROR: UI Widget Template File could not be loaded: " << widgetFilename << ", error: " << loader.errorString();

    return newWidget;
}

QWidget* CharacterTemplateFrame::createResourceWidgetInternal(const QString& keyString)
{
    if(!CombatantFactory::Instance())
        return nullptr;

    QHash<QString, DMHAttribute> elementList = CombatantFactory::Instance()->getElementList(keyString);
    if(elementList.isEmpty())
        return nullptr;

    // Interate through the keys of the elementList and create the appropriate UI elements
    QWidget* newWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(newWidget);

    QStringList keys = elementList.keys();
    for(const auto &elementName : keys)
    {
        if(elementName.isEmpty())
            continue;

        layout->addWidget(new QLabel(elementName));
        QWidget* lineEditWidget = new QLineEdit();
        lineEditWidget->setProperty(CombatantFactory::TEMPLATE_PROPERTY, keyString);
        layout->addWidget(lineEditWidget);
    }

    return newWidget;
}

void CharacterTemplateFrame::populateWidget(QWidget* widget, Characterv2* character, QHash<QString, QVariant>* hash, int listIndex, const QString& listKey)
{
    if(!widget)
        return;

    QList<QLineEdit*> lineEdits = widget->findChildren<QLineEdit*>();
    for(auto lineEdit : lineEdits)
    {
        if(!lineEdit)
            continue;

        QString keyString = lineEdit->property(CombatantFactory::TEMPLATE_PROPERTY).toString();
        if(!keyString.isEmpty())
        {
            QString valueString;
            if(character)
                valueString = character->getValueAsString(keyString);
            else if(hash)
                valueString = hash->value(keyString).toString();

            lineEdit->setText(valueString.isEmpty() ? getDefaultValue(keyString) : valueString);
            //if(!valueString.isEmpty())
                connect(lineEdit, &QLineEdit::editingFinished, [this, lineEdit](){handleLineEditFinished(lineEdit);});
        }
    }

    QList<QTextEdit*> textEdits = widget->findChildren<QTextEdit*>();
    for(auto textEdit : textEdits)
    {
        if(!textEdit)
            continue;

        QString keyString = textEdit->property(CombatantFactory::TEMPLATE_PROPERTY).toString();
        if(!keyString.isEmpty())
        {
            QString valueString;
            if(character)
                valueString = character->getValueAsString(keyString);
            else if(hash)
                valueString = hash->value(keyString).toString();

            textEdit->setHtml(valueString.isEmpty() ? getDefaultValue(keyString) : valueString);
            //if(!valueString.isEmpty())
                connect(textEdit, &QTextEdit::textChanged, [this, textEdit](){handleTextEditChanged(textEdit);});
        }
    }

    QList<QFrame*> frames = widget->findChildren<QFrame*>();
    for(auto frame : frames)
    {
        if((!frame) || (dynamic_cast<QTextEdit*>(frame)) || (dynamic_cast<QScrollArea*>(frame)) || (dynamic_cast<QScrollArea*>(frame->parentWidget())))
            continue;

        if(QLayout* oldLayout = frame->layout())
        {
            frame->removeEventFilter(oldLayout);
            delete oldLayout;
        }

        QString keyString = frame->property(CombatantFactory::TEMPLATE_PROPERTY).toString();
        if(keyString.isEmpty())
            continue;

        ResourcePair valuePair;
        if(character)
            valuePair = character->getResourceValue(keyString);
        else if(hash)
            valuePair = hash->value(keyString).value<ResourcePair>();

        CharacterTemplateResourceLayout* layout = nullptr;
        if(hash)
        {
            layout = new CharacterTemplateResourceLayout(listKey, listIndex, keyString, valuePair);
            connect(layout, &CharacterTemplateResourceLayout::resourceListValueChanged, _character, &Characterv2::setListValue);
            connect(layout, &CharacterTemplateResourceLayout::addResource, this, [this, widget](){ this->handleAddResource(widget); });
            connect(layout, &CharacterTemplateResourceLayout::removeResource, this, [this, widget](){ this->handleRemoveResource(widget); });
        }
        else // character or nulls
        {
            layout = new CharacterTemplateResourceLayout(keyString, valuePair);
            connect(layout, &CharacterTemplateResourceLayout::resourceValueChanged, _character, &Characterv2::setResourceValue);
        }
        frame->installEventFilter(layout);
        frame->setLayout(layout);
    }
}

QString CharacterTemplateFrame::getDefaultValue(const QString& keyString)
{
    if(CombatantFactory::Instance()->hasAttribute(keyString))
        return CombatantFactory::Instance()->getAttribute(keyString)._default;
    else if(CombatantFactory::Instance()->hasElement(keyString))
        return CombatantFactory::Instance()->getElement(keyString)._default;
    else
        return QString();
}

void CharacterTemplateFrame::handleEditBoxChange(QWidget* editWidget, const QString& value)
{
    if((!_character) || (!editWidget))
        return;

    int widgetIndex = 0;
    QBoxLayout* scrollLayout = nullptr;
    QString scrollKey;

    QWidget* parentWidget = editWidget->parentWidget();

    // Traverse up the widget hierarchy until the QScrollArea is found
    while(parentWidget)
    {
        if(QScrollArea* scrollArea = dynamic_cast<QScrollArea*>(parentWidget))
        {
            if(scrollArea->widget())
            {
                scrollLayout = dynamic_cast<QBoxLayout*>(scrollArea->widget()->layout());
                scrollKey = scrollArea->property(CombatantFactory::TEMPLATE_PROPERTY).toString();
                break;
            }
        }
        parentWidget = parentWidget->parentWidget();
    }

    if(scrollLayout)
        widgetIndex = scrollLayout->indexOf(editWidget->parentWidget());

    QString editWidgetKey = editWidget->property(CombatantFactory::TEMPLATE_PROPERTY).toString();
    if((!scrollKey.isEmpty()) && (widgetIndex >= 0))
        _character->setListValue(scrollKey, widgetIndex, editWidgetKey, value);
    else
        _character->setValue(editWidgetKey, value);
}
