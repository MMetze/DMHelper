#include "charactertemplateframe.h"
#include "ui_charactertemplateframe.h"
#include "charactertemplateresourcelayout.h"
#include "characterimporter.h"
#include "characterimportheroforgedialog.h"
#include "tokeneditdialog.h"
#include "optionscontainer.h"
#include "combatantfactory.h"
#include <QUiLoader>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QMouseEvent>
#include <QStandardPaths>
#include <QTextEdit>
#include <QCheckBox>
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

    QString defaultFilename("character.ui");
#ifdef Q_OS_MAC
    QDir fileDirPath(QCoreApplication::applicationDirPath());
    fileDirPath.cdUp();
    QString appFile = fileDirPath.path() + QString("/Resources/ui/") + defaultFilename;
#else
    QDir fileDirPath(QCoreApplication::applicationDirPath());
    QString appFile = fileDirPath.path() + QString("/resources/ui/") + defaultFilename;
#endif

    if(!QFileInfo::exists(appFile))
    {
        qDebug() << "[CharacterTemplateFrame] ERROR: UI Template File not found: " << appFile;
    }
    else
    {
        QUiLoader loader;
        QFile file(appFile);
        file.open(QFile::ReadOnly);
        _uiWidget = loader.load(&file, this);
        file.close();

        if(!_uiWidget)
        {
            qDebug() << "[CharacterTemplateFrame] ERROR: UI Template File could not be loaded: " << loader.errorString();
        }
        else
        {
            QVBoxLayout *layout = new QVBoxLayout;
            layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
            layout->addWidget(_uiWidget);
            ui->scrollAreaWidgetContents->setLayout(layout);
        }
    }

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
    if((!_character) || (!_uiWidget))
        return;

    _reading = true;

    // Walk through the loaded UI Widget and allocate the appropriate character values to the UI elements
    QList<QLineEdit*> lineEdits = _uiWidget->findChildren<QLineEdit*>();
    for(auto lineEdit : lineEdits)
    {
        if(!lineEdit)
            continue;

        QString keyString = lineEdit->property(CombatantFactory::TEMPLATE_PROPERTY).toString();
        if(!keyString.isEmpty())
        {
            QString valueString = _character->getValueAsString(keyString);
            if(!valueString.isEmpty())
            {
                lineEdit->setText(valueString);
                connect(lineEdit, &QLineEdit::editingFinished, [this, lineEdit](){handleLineEditFinished(lineEdit);});
            }
        }
    }

    QList<QTextEdit*> textEdits = _uiWidget->findChildren<QTextEdit*>();
    for(auto textEdit : textEdits)
    {
        if(!textEdit)
            continue;

        QString keyString = textEdit->property(CombatantFactory::TEMPLATE_PROPERTY).toString();
        if(!keyString.isEmpty())
        {
            QString valueString = _character->getStringValue(keyString);
            if(!valueString.isEmpty())
            {
                textEdit->setHtml(valueString);
                connect(textEdit, &QTextEdit::textChanged, [this, textEdit](){handleTextEditChanged(textEdit);});
            }
        }
    }

    QList<QFrame*> frames = _uiWidget->findChildren<QFrame*>();
    for(auto frame : frames)
    {
        if((!frame) || (dynamic_cast<QTextEdit*>(frame)) || (dynamic_cast<QScrollArea*>(frame)))
            continue;

        if(QLayout* oldLayout = frame->layout())
        {
            frame->removeEventFilter(oldLayout);
            delete oldLayout;
        }

        QString keyString = frame->property(CombatantFactory::TEMPLATE_PROPERTY).toString();
        if(keyString.isEmpty())
            continue;

        ResourcePair valuePair = _character->getResourceValue(keyString);
        if(valuePair.second == 0)
            continue;

        CharacterTemplateResourceLayout* layout = new CharacterTemplateResourceLayout(keyString, valuePair);
        connect(layout, &CharacterTemplateResourceLayout::resourceValueChanged, _character, &Characterv2::setResourceValue);
        frame->installEventFilter(layout);
        frame->setLayout(layout);
    }

    QList<QScrollArea*> scrollAreas = _uiWidget->findChildren<QScrollArea*>();
    for(auto scrollArea : scrollAreas)
    {
        if(!scrollArea)
            continue;

        QString keyString = scrollArea->property(CombatantFactory::TEMPLATE_PROPERTY).toString();
        QString widgetString = scrollArea->property(CombatantFactory::TEMPLATE_WIDGET).toString();
        if((!keyString.isEmpty()) && (!widgetString.isEmpty()))
        {
            QList<QVariant> listValue = _character->getListValue(keyString);
            if(!listValue.isEmpty())
            {
                if(QLayout* oldLayout = scrollArea->layout())
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

#ifdef Q_OS_MAC
                QDir fileDirPath(QCoreApplication::applicationDirPath());
                fileDirPath.cdUp();
                QString appFile = fileDirPath.path() + QString("/Resources/ui/") + widgetString;
#else
                QDir fileDirPath(QCoreApplication::applicationDirPath());
                QString appFile = fileDirPath.path() + QString("/resources/ui/") + widgetString;
#endif

                if(!QFileInfo::exists(appFile))
                {
                    qDebug() << "[CharacterTemplateFrame] ERROR: UI Widget Template File not found: " << appFile << ", for scroll area: " << keyString;
                }
                else
                {
                    QVBoxLayout* scrollLayout = new QVBoxLayout;
                    scrollLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
                    scrollArea->setLayout(scrollLayout);

                    //for(auto listEntry : listValue)
                    for(int i = 0; i < listValue.count(); ++i)
                    {
                        QVariant listEntry = listValue.at(i);
                        if(listEntry.isNull())
                            continue;

                        QUiLoader loader;
                        QFile file(appFile);
                        file.open(QFile::ReadOnly);
                        QWidget* newWidget = loader.load(&file, this);
                        file.close();

                        if(!newWidget)
                        {
                            qDebug() << "[CharacterTemplateFrame] ERROR: UI Widget Template File could not be loaded: " << appFile << ", for scroll area: " << keyString;
                            continue;
                        }

                        QHash<QString, QVariant> hashValue = listEntry.toHash();

                        // Walk through the loaded UI Widget and allocate the appropriate character values to the UI elements
                        QList<QLineEdit*> lineEdits = newWidget->findChildren<QLineEdit*>();
                        for(auto lineEdit : lineEdits)
                        {
                            QString keyString = lineEdit->property(CombatantFactory::TEMPLATE_PROPERTY).toString();
                            if(!keyString.isEmpty())
                            {
                                QString valueString = hashValue.value(keyString).toString();
                                if(!valueString.isEmpty())
                                    lineEdit->setText(valueString);
                            }
                        }

                        QList<QTextEdit*> textEdits = newWidget->findChildren<QTextEdit*>();
                        for(auto textEdit : textEdits)
                        {
                            QString keyString = textEdit->property(CombatantFactory::TEMPLATE_PROPERTY).toString();
                            if(!keyString.isEmpty())
                            {
                                QString valueString = hashValue.value(keyString).toString();
                                if(!valueString.isEmpty())
                                    textEdit->setHtml(valueString);
                            }
                        }

                        QList<QFrame*> frames = newWidget->findChildren<QFrame*>();
                        for(auto frame : frames)
                        {
                            if(QLayout* oldLayout = frame->layout())
                            {
                                frame->removeEventFilter(oldLayout);
                                delete oldLayout;
                            }

                            QString listKey = frame->property(CombatantFactory::TEMPLATE_PROPERTY).toString();
                            if(listKey.isEmpty())
                                continue;

                            ResourcePair valuePair = hashValue.value(listKey).value<ResourcePair>();
                            if(valuePair.second == 0)
                                continue;

                            CharacterTemplateResourceLayout* layout = new CharacterTemplateResourceLayout(keyString, i, listKey, valuePair);
                            connect(layout, &CharacterTemplateResourceLayout::resourceListValueChanged, _character, &Characterv2::setListValue);
                            frame->installEventFilter(layout);
                            frame->setLayout(layout);

                            /*
                            if((!frame) || (dynamic_cast<QTextEdit*>(frame)))
                                continue;

                            QString keyString = frame->property(CombatantFactory::TEMPLATE_PROPERTY).toString();
                            if(keyString.isEmpty())
                                continue;

                            QHBoxLayout* frameLayout = new QHBoxLayout;
                            frameLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
                            frameLayout->setContentsMargins(0, 0, 0, 0);
                            frame->setLayout(frameLayout);

                            ResourcePair valuePair = hashValue.value(keyString).value<ResourcePair>();
                            for(int i = 0; i < valuePair.second; ++i)
                            {
                                QCheckBox* checkBox = new QCheckBox();
                                checkBox->setChecked(i < valuePair.first);
                                frameLayout->addWidget(checkBox);
                                connect(checkBox, &QCheckBox::stateChanged, [this, frame](){handleResourceChanged(frame);});
                            }
                            */
                        }

                        scrollLayout->addWidget(newWidget);
                    }
                }
            }
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

    // HACK
    /*
    CharacterImporter* importer = new CharacterImporter();
    connect(importer, &CharacterImporter::characterImported, this, &CharacterTemplateFrame::readCharacterData);
    connect(this, &CharacterTemplateFrame::characterChanged, importer, &CharacterImporter::campaignChanged);
    importer->updateCharacter(_character);
    importer->deleteLater();
*/
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

    //ui->edtName->setText(_character->getName());
}

void CharacterTemplateFrame::handleLineEditFinished(QLineEdit* lineEdit)
{
    if((!_character) || (!lineEdit))
        return;

    QString keyString = lineEdit->property(CombatantFactory::TEMPLATE_PROPERTY).toString();
    _character->setValue(keyString, lineEdit->text());
}

void CharacterTemplateFrame::handleTextEditChanged(QTextEdit* textEdit)
{
    if((!_character) || (!textEdit))
        return;

    QString keyString = textEdit->property(CombatantFactory::TEMPLATE_PROPERTY).toString();
    _character->setValue(keyString, textEdit->toHtml());
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