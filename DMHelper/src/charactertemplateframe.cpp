#include "charactertemplateframe.h"
#include "ui_charactertemplateframe.h"
#include "characterimporter.h"
#include "characterimportheroforgedialog.h"
#include "tokeneditdialog.h"
#include "optionscontainer.h"
#include "combatantfactory.h"
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QMouseEvent>
#include <QTextEdit>
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

    //if(_character)
    //    CombatantFactory::Instance()->disconnectWidget(_uiWidget);

    delete _uiWidget;
    if(ui->scrollAreaWidgetContents->layout())
        delete ui->scrollAreaWidgetContents->layout();

    _uiWidget = newWidget;
    _uiFilename = absoluteTemplateFile;

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

    loadCharacterImage();
    enableDndBeyondSync(_character->getDndBeyondID() != -1);

    _reading = false;
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

/*
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
*/

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
