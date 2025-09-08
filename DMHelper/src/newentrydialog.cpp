#include "newentrydialog.h"
#include "ui_newentrydialog.h"
#include "dmconstants.h"
#include "encounterfactory.h"
#include "combatantfactory.h"
#include "encountertext.h"
#include "encountertextlinked.h"
#include "optionscontainer.h"
#include "campaignobjectbase.h"
#include "party.h"
#include "characterv2.h"
#include "tokeneditdialog.h"
#include "characterimportheroforgedialog.h"
#include "bestiary.h"
#include "monsterclassv2.h"
#include "layerimage.h"
#include "layervideo.h"
#include "layerfow.h"
#include "layergrid.h"
#include "layertokens.h"
#include "videoplayerscreenshot.h"
#include "map.h"
#include "mapfactory.h"
#include "encounterbattle.h"
#include "mapselectdialog.h"
#include "mapblankdialog.h"
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QMimeDatabase>
#include <QDebug>

NewEntryDialog::NewEntryDialog(Campaign* campaign, OptionsContainer* options, CampaignObjectBase* currentObject, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewEntryDialog),
    _campaign(campaign),
    _options(options),
    _currentObject(currentObject),
    _primaryImageFile(),
    _screenshot(nullptr),
    _imageType(DMHelper::FileType_Unknown),
    _gridSizeGuess(DMHelper::STARTING_GRID_SCALE),
    _referenceMap(nullptr),
    _imageSize(),
    _imageColor()
{
    ui->setupUi(this);

    connect(ui->edtEntryName, &QLineEdit::textChanged, this, &NewEntryDialog::validateNewEntry);
    connect(ui->edtLinkedFile, &QLineEdit::textChanged, this, &NewEntryDialog::validateNewEntry);

    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, &NewEntryDialog::newPageSelected);

    connect(ui->btnTypeText, &QPushButton::clicked, this, [this]() {ui->stackedWidget->setCurrentWidget(ui->pageEntry);});
    connect(ui->btnTypeLinked, &QPushButton::clicked, this, [this]() {ui->stackedWidget->setCurrentWidget(ui->pageLinkedEntry);});
    connect(ui->btnTypeParty, &QPushButton::clicked, this, [this]() {ui->stackedWidget->setCurrentWidget(ui->pageParty);});
    connect(ui->btnTypeCharacter, &QPushButton::clicked, this, [this]() {ui->stackedWidget->setCurrentWidget(ui->pageCharacter);});
    connect(ui->btnTypeMedia, &QPushButton::clicked, this, [this]() {ui->stackedWidget->setCurrentWidget(ui->pageMedia);});
    connect(ui->btnTypeMap, &QPushButton::clicked, this, [this]() {ui->stackedWidget->setCurrentWidget(ui->pageMap);});
    connect(ui->btnTypeCombat, &QPushButton::clicked, this, [this]() {ui->stackedWidget->setCurrentWidget(ui->pageCombat);});

    ui->btnTypeText->setChecked(true);
    ui->stackedWidget->setCurrentWidget(ui->pageEntry);

    // Linked Page
    connect(ui->btnBrowseLinkedFile, &QPushButton::clicked, this, &NewEntryDialog::browseLinkedTextFile);

    // Party Page
    connect(ui->btnPartyIcon, &QPushButton::clicked, this, &NewEntryDialog::selectPartyIcon);

    // Character Page
    connect(ui->btnCharacterIcon, &QPushButton::clicked, this, &NewEntryDialog::selectCharacterIcon);
    connect(ui->btnCharacterEditIcon, &QPushButton::clicked, this, &NewEntryDialog::editCharacterIcon);
    connect(ui->cmbCharacterMonster, &QComboBox::currentTextChanged, this, &NewEntryDialog::loadMonsterIcon);

    // Media Page
    connect(ui->edtMediaFile, &QLineEdit::editingFinished, this, &NewEntryDialog::readMediaFile);
    connect(ui->btnMediaBrowse, &QPushButton::clicked, this, &NewEntryDialog::browseMediaFile);

    // Map Page
    connect(ui->edtMapFile, &QLineEdit::editingFinished, this, &NewEntryDialog::readMapFile);
    connect(ui->btnMapBrowse, &QPushButton::clicked, this, &NewEntryDialog::browseMapFile);

    // Combat Page
    connect(ui->edtCombatFile, &QLineEdit::editingFinished, this, &NewEntryDialog::readCombatFile);
    connect(ui->btnCombatBrowse, &QPushButton::clicked, this, &NewEntryDialog::browseCombatFile);
    connect(ui->btnCombatSelectMap, &QPushButton::clicked, this, &NewEntryDialog::selectCombatSource);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Create Entry"));
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

NewEntryDialog::~NewEntryDialog()
{
    disconnectScreenshot();
    delete ui;
}

CampaignObjectBase* NewEntryDialog::createNewEntry()
{
    if(getNewEntryName().isEmpty())
    {
        QMessageBox::warning(this, tr("Invalid Entry Name"), tr("Please enter a valid name for the new entry."));
        return nullptr;
    }

    if (ui->buttonGroupType->checkedButton() == ui->btnTypeText)
        return createTextEntry();
    else if (ui->buttonGroupType->checkedButton() == ui->btnTypeLinked)
        return createLinkedEntry();
    else if (ui->buttonGroupType->checkedButton() == ui->btnTypeParty)
        return createPartyEntry();
    else if (ui->buttonGroupType->checkedButton() == ui->btnTypeCharacter)
        return createCharacterEntry();
    else if (ui->buttonGroupType->checkedButton() == ui->btnTypeMedia)
        return createMediaEntry();
    else if (ui->buttonGroupType->checkedButton() == ui->btnTypeMap)
        return createMapEntry();
    else if (ui->buttonGroupType->checkedButton() == ui->btnTypeCombat)
        return createBattleEntry();
    else
        return nullptr;
}

bool NewEntryDialog::isImportNeeded()
{
    return((ui->buttonGroupType->checkedButton() == ui->btnTypeCharacter) &&
           (ui->buttonGroupCharacter->checkedButton() == ui->btnCharacterDnDBeyond) &&
           (!ui->edtCharacterDndBeyond->text().isEmpty()));
}

QString NewEntryDialog::getNewEntryName() const
{
    return ui->edtEntryName->text().trimmed();
}

QString NewEntryDialog::getImportString() const
{
    return ui->edtCharacterDndBeyond->text();
}

CampaignObjectBase* NewEntryDialog::createTextEntry()
{
    EncounterText* encounter = qobject_cast<EncounterText*>(EncounterFactory().createObject(DMHelper::CampaignType_Text, -1, getNewEntryName(), false));
    if(!encounter)
        return nullptr;

    encounter->setText(ui->textBrowserEntry->toHtml());

    return encounter;
}

CampaignObjectBase* NewEntryDialog::createLinkedEntry()
{
    if((ui->edtLinkedFile->text().isEmpty()) || (!QFile::exists(ui->edtLinkedFile->text())))
    {
        QMessageBox::warning(this, tr("Invalid Linked File"), tr("Please enter a valid file name for the linked entry: ") + ui->edtLinkedFile->text());
        return nullptr;
    }

    EncounterTextLinked* encounter = qobject_cast<EncounterTextLinked*>(EncounterFactory().createObject(DMHelper::CampaignType_LinkedText, -1, getNewEntryName(), false));
    if(!encounter)
        return nullptr;

    encounter->setLinkedFile(ui->edtLinkedFile->text().trimmed());

    return encounter;
}

CampaignObjectBase* NewEntryDialog::createPartyEntry()
{
    Party* party = qobject_cast<Party*>(EncounterFactory().createObject(DMHelper::CampaignType_Party, -1, getNewEntryName(), false));
    if(!party)
        return nullptr;

    party->setIcon(_primaryImageFile);

    return party;
}

CampaignObjectBase* NewEntryDialog::createCharacterEntry()
{
    if(!CombatantFactory::Instance())
    {
        qDebug() << "[NewEntryDialog] New character not created because the combatant factory could not be found";
        return nullptr;
    }

    if (ui->buttonGroupCharacter->checkedButton() == ui->btnCharacterNew)
    {
        Characterv2* character = qobject_cast<Characterv2*>(CombatantFactory::Instance()->createObject(DMHelper::CampaignType_Combatant, DMHelper::CombatantType_Character, getNewEntryName(), false));
        if(!character)
            return nullptr;

        if(!_primaryImageFile.isEmpty())
            character->setIcon(_primaryImageFile);

        return character;
    }
    else if (ui->buttonGroupType->checkedButton() == ui->btnCharacterMonster)
    {
        QString selectedMonster = ui->cmbCharacterMonster->currentText();
        if(selectedMonster.isEmpty())
            return nullptr;

        MonsterClassv2* monsterClass = Bestiary::Instance()->getMonsterClass(selectedMonster);
        if(!monsterClass)
            return nullptr;

        Characterv2* character = qobject_cast<Characterv2*>(CombatantFactory::Instance()->createObject(DMHelper::CampaignType_Combatant, DMHelper::CombatantType_Character, getNewEntryName(), false));
        if(!character)
            return nullptr;

        if(!_primaryImageFile.isEmpty())
            character->setIcon(_primaryImageFile);

        character->copyMonsterValues(*monsterClass);
        character->setName(getNewEntryName());

        return character;
    }
    else if (ui->buttonGroupType->checkedButton() == ui->btnCharacterDnDBeyond)
    {
        return nullptr; // Handled in MainWindow with isImportNeeded()
    }
    else
        return nullptr;
}

CampaignObjectBase* NewEntryDialog::createMediaEntry()
{
    if((_primaryImageFile.isEmpty())|| (_imageType == DMHelper::FileType_Unknown))
        return nullptr;

    Layer* mediaLayer;
    if(_imageType == DMHelper::FileType_Image)
    {
        LayerImage* imageLayer = new LayerImage(QString("Media Image: ") + QFileInfo(_primaryImageFile).fileName(), _primaryImageFile);
        mediaLayer = imageLayer;
    }
    else if(_imageType == DMHelper::FileType_Video)
    {
        LayerVideo* videoLayer = new LayerVideo(QString("Media Video: ") + QFileInfo(_primaryImageFile).fileName(), _primaryImageFile);
        mediaLayer = videoLayer;
    }
    else
    {
        return nullptr;
    }

    Map* mediaMap = dynamic_cast<Map*>(MapFactory().createObject(DMHelper::CampaignType_Map, -1, getNewEntryName(), false));
    if(!mediaMap)
    {
        delete mediaLayer;
        return nullptr;
    }

    mediaMap->getLayerScene().appendLayer(mediaLayer);
    mediaMap->getLayerScene().setScale(_gridSizeGuess);

    return mediaMap;
}

CampaignObjectBase* NewEntryDialog::createMapEntry()
{
    if((_primaryImageFile.isEmpty())|| (_imageType == DMHelper::FileType_Unknown))
        return nullptr;

    Layer* mediaLayer;
    if(_imageType == DMHelper::FileType_Image)
    {
        LayerImage* imageLayer = new LayerImage(QString("Media Image: ") + QFileInfo(_primaryImageFile).fileName(), _primaryImageFile);
        mediaLayer = imageLayer;
    }
    else if(_imageType == DMHelper::FileType_Video)
    {
        LayerVideo* videoLayer = new LayerVideo(QString("Media Video: ") + QFileInfo(_primaryImageFile).fileName(), _primaryImageFile);
        mediaLayer = videoLayer;
    }
    else
    {
        return nullptr;
    }

    Map* mediaMap = dynamic_cast<Map*>(MapFactory().createObject(DMHelper::CampaignType_Map, -1, getNewEntryName(), false));
    if(!mediaMap)
    {
        delete mediaLayer;
        return nullptr;
    }

    mediaMap->getLayerScene().appendLayer(mediaLayer);

    if(ui->chkMapFow->isChecked())
    {
        LayerFow* fowLayer = new LayerFow(QString("FoW"));
        mediaMap->getLayerScene().appendLayer(fowLayer);
    }

    mediaMap->getLayerScene().setScale(_gridSizeGuess);

    return mediaMap;
}

CampaignObjectBase* NewEntryDialog::createBattleEntry()
{
    //now this...
    return nullptr;
    /*
    EncounterBattle* battle = dynamic_cast<EncounterBattle*>(EncounterFactory().createObject(DMHelper::CampaignType_Battle, -1, getNewEntryName(), false));
    if(!battle)
        return nullptr;

    battle->createBattleDialogModel();
    if(!battle->getBattleDialogModel())
        return nullptr;

    int gridScale = DMHelper::STARTING_GRID_SCALE;
    LayerGrid* gridLayer = nullptr;
    LayerTokens* monsterTokens = nullptr;
    LayerTokens* pcTokens = nullptr;
    bool hasGrid = false;

    MapSelectDialog mapSelectDlg(*_campaign, currentObject->getID());
    if(mapSelectDlg.exec() == QDialog::Accepted)
    {
        if(mapSelectDlg.isMapSelected())
        {
            Map* battleMap = mapSelectDlg.getSelectedMap();
            if(battleMap)
            {
                battleMap->initialize();
                gridScale = battleMap->getLayerScene().getScale();
                hasGrid = battleMap->getLayerScene().layerCount(DMHelper::LayerType_Grid) > 0;

                // Create a grid after the first image layer, a monster token layer before the FoW
                for(int i = 0; i < battleMap->getLayerScene().layerCount(); ++i)
                {
                    Layer* layer = battleMap->getLayerScene().layerAt(i);
                    if(layer)
                    {
                        if((!monsterTokens) && (layer->getFinalType() == DMHelper::LayerType_Fow))
                        {
                            monsterTokens = new LayerTokens(battle->getBattleDialogModel(), QString("Monster tokens"));
                            battle->getBattleDialogModel()->getLayerScene().appendLayer(monsterTokens);
                        }

                        battle->getBattleDialogModel()->getLayerScene().appendLayer(new LayerReference(battleMap, layer, layer->getOrder()));

                        if((!hasGrid) && ((layer->getFinalType() == DMHelper::LayerType_Image) || (layer->getFinalType() == DMHelper::LayerType_Video)))
                        {
                            gridLayer = new LayerGrid(QString("Grid"));
                            battle->getBattleDialogModel()->getLayerScene().appendLayer(gridLayer);
                            hasGrid = true;
                        }
                    }
                }
            }
        }
        else if(mapSelectDlg.isBlankMap())
        {
            MapBlankDialog blankDlg;
            int result = blankDlg.exec();
            if(result == QDialog::Accepted)
            {
                LayerBlank* blankLayer = new LayerBlank(QString("Blank Layer"), blankDlg.getMapColor());
                blankLayer->setSize(blankDlg.getMapSize());
                battle->getBattleDialogModel()->getLayerScene().appendLayer(blankLayer);
            }
        }
        else if(mapSelectDlg.isNewMapImage())
        {
            Layer* mapLayer = selectMapFile();
            if(mapLayer)
            {
                mapLayer->initialize(QSize());

                if(mapLayer->getType() == DMHelper::LayerType_Image)
                {
                    LayerImage* imageLayer = dynamic_cast<LayerImage*>(mapLayer);
                    if((imageLayer) && (!imageLayer->getImageUnfiltered().isNull()))
                    {
                        int gridSizeGuess = qRound(static_cast<qreal>(imageLayer->getImageUnfiltered().dotsPerMeterX()) * 0.0254);
                        if(gridSizeGuess >= 5)
                            gridScale = gridSizeGuess;
                    }
                }

                battle->getBattleDialogModel()->getLayerScene().appendLayer(mapLayer);
            }
        }
    }

    if((!gridLayer) && (!hasGrid))
    {
        gridLayer = new LayerGrid(QString("Grid"));
        battle->getBattleDialogModel()->getLayerScene().appendLayer(gridLayer);
    }

    if(gridLayer)
        gridLayer->getConfig().setGridScale(gridScale);
    battle->getBattleDialogModel()->getLayerScene().setScale(gridScale);

    if(!monsterTokens)
    {
        monsterTokens = new LayerTokens(battle->getBattleDialogModel(), QString("Monster tokens"));
        battle->getBattleDialogModel()->getLayerScene().appendLayer(monsterTokens);
    }

    pcTokens = new LayerTokens(battle->getBattleDialogModel(), QString("PC tokens"));
    battle->getBattleDialogModel()->getLayerScene().appendLayer(pcTokens);

    // Add the active characters
    battle->getBattleDialogModel()->getLayerScene().setSelectedLayer(pcTokens);
    QPointF mapCenter = battle->getBattleDialogModel()->getLayerScene().boundingRect().center();
    if(mapCenter.isNull())
        mapCenter = QPointF(gridScale, gridScale);
    QPointF multiplePos(gridScale / 10.0, gridScale / 10.0);
    QList<Characterv2*> activeCharacters = _campaign->getActiveCharacters();
    for(int i = 0; i < activeCharacters.count(); ++i)
    {
        BattleDialogModelCharacter* newCharacter = new BattleDialogModelCharacter(activeCharacters.at(i));
        newCharacter->setPosition(mapCenter + (multiplePos * i));
        battle->getBattleDialogModel()->appendCombatant(newCharacter);
    }

    // Select the monster layer as a default to add monsters
    battle->getBattleDialogModel()->getLayerScene().setSelectedLayer(monsterTokens);
    battle->getBattleDialogModel()->setMapRect(battle->getBattleDialogModel()->getLayerScene().boundingRect().toRect());

    addNewObject(encounter);

    _battleFrame->resizeGrid();
*/
}

void NewEntryDialog::validateNewEntry()
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(isSelectedEntryValid());
}

void NewEntryDialog::newPageSelected()
{
    if (ui->buttonGroupType->checkedButton() == ui->btnTypeText)
    {
        disconnectScreenshot();
    }
    else if (ui->buttonGroupType->checkedButton() == ui->btnTypeLinked)
    {
        disconnectScreenshot();
    }
    else if (ui->buttonGroupType->checkedButton() == ui->btnTypeParty)
    {
        if(_imageType == DMHelper::FileType_Video)
        {
            disconnectScreenshot();
            _primaryImageFile = QString();
            _imageType = DMHelper::FileType_Unknown;
        }

        loadPrimaryImage(nullptr, ui->btnPartyIcon, 128, 128, QString(":/img/data/icon_contentparty.png"));
    }
    else if (ui->buttonGroupType->checkedButton() == ui->btnTypeCharacter)
    {
        if(_imageType == DMHelper::FileType_Video)
        {
            disconnectScreenshot();
            _primaryImageFile = QString();
            _imageType = DMHelper::FileType_Unknown;
        }

        loadPrimaryImage(nullptr, ui->btnCharacterIcon, 180, 260, QString(":/img/data/portrait.png"));
        loadMonsterList();
    }
    else if (ui->buttonGroupType->checkedButton() == ui->btnTypeMedia)
    {
        loadPrimaryImage(ui->lblMediaPreview, nullptr, ui->lblMediaPreview->width() - 20, ui->lblMediaPreview->height() - 20, QString());
        ui->edtMediaFile->setText(_primaryImageFile);
    }
    else if (ui->buttonGroupType->checkedButton() == ui->btnTypeMap)
    {
        loadPrimaryImage(ui->lblMediaPreview, nullptr, ui->lblMediaPreview->width() - 20, ui->lblMediaPreview->height() - 20, QString());
        ui->edtMediaFile->setText(_primaryImageFile);
    }
    else if (ui->buttonGroupType->checkedButton() == ui->btnTypeCombat)
    {
    }

    validateNewEntry();
}

void NewEntryDialog::browseLinkedTextFile()
{
    QString newLinkedFile = QFileDialog::getOpenFileName(this, QString("Select File to link"));
    if(newLinkedFile.isEmpty())
        return;

    ui->edtLinkedFile->setText(newLinkedFile);
}

void NewEntryDialog::selectPartyIcon()
{
    selectNewPrimaryImage(nullptr, ui->btnPartyIcon, 128, 128, tr("Select Party Icon"), QString(":/img/data/icon_contentparty.png"));
}

void NewEntryDialog::selectCharacterIcon()
{
    selectNewPrimaryImage(nullptr, ui->btnCharacterIcon, 180, 260, tr("Select Character Token"), QString(":/img/data/portrait.png"));
}

void NewEntryDialog::editCharacterIcon()
{
    // Use the TokenEditDialog to edit the character icon
    if(!_options)
        return;

    QImage currentToken(_primaryImageFile.isEmpty() ? QString(":/img/data/portrait.png") : _primaryImageFile);

    TokenEditDialog* dlg = new TokenEditDialog(currentToken, *_options, 1.0, QPoint(), false);
    if(dlg->exec() == QDialog::Accepted)
    {
        QImage newToken = dlg->getFinalImage();
        if(newToken.isNull())
            return;

        QString tokenPath = QFileDialog::getExistingDirectory(this, tr("Select Token Directory"), _primaryImageFile.isEmpty() ? QString() : QFileInfo(_primaryImageFile).absolutePath());
        if(tokenPath.isEmpty())
            return;

        QDir tokenDir(tokenPath);

        int fileIndex = 1;
        QString characterNameBase = getNewEntryName().isEmpty() ? QString("Character") : getNewEntryName();
        QString tokenFile = characterNameBase + QString(".png");
        while(tokenDir.exists(tokenFile))
            tokenFile = characterNameBase + QString::number(fileIndex++) + QString(".png");

        QString finalTokenPath = tokenDir.absoluteFilePath(tokenFile);
        newToken.save(finalTokenPath);

        setNewPrimaryImage(finalTokenPath, nullptr, ui->btnCharacterIcon, 180, 260, QString(":/img/data/portrait.png"));

        if(dlg->getEditor())
            dlg->getEditor()->applyEditorToOptions(*_options);

    }

    dlg->deleteLater();
}

void NewEntryDialog::importHeroForge()
{
    if(!_options)
        return;

    QString token = _options->getHeroForgeToken();
    if(token.isEmpty())
    {
        token = QInputDialog::getText(this, QString("Enter Hero Forge Access Key"), QString("Please enter your Hero Forge Access Key. You can find this in your Hero Forge account information."));
        if(!token.isEmpty())
        {
            if(QMessageBox::question(this,
                                      QString("Confirm Store Access Key"),
                                      QString("Should DMHelper store your access key for ease of use in the future?") + QChar::LineFeed + QChar::LineFeed + QString("Please note: the Access Key will be stored locally on your computer without encryption, it is possible that other applications will be able to access it.")) == QMessageBox::Yes)
            {
                _options->setHeroForgeToken(token);
            }
        }
    }

    if(token.isEmpty())
    {
        qDebug() << "[NewEntryDialog] No Hero Forge token provided, importer can't be started.";
        return;
    }

    CharacterImportHeroForgeDialog importDialog(token);
    importDialog.resize(width(), height());
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

    setNewPrimaryImage(filename, nullptr, ui->btnCharacterIcon, 180, 260, QString(":/img/data/portrait.png"));
}

void NewEntryDialog::loadMonsterList()
{
    if(ui->cmbCharacterMonster->count() > 0)
        return; // Monster list already loaded

    ui->cmbCharacterMonster->addItems(Bestiary::Instance()->getMonsterList());
}

void NewEntryDialog::loadMonsterIcon()
{
    if(!_primaryImageFile.isEmpty())
        return;

    QString selectedMonster = ui->cmbCharacterMonster->currentText();
    if(selectedMonster.isEmpty())
        return;

    MonsterClassv2* monsterClass = Bestiary::Instance()->getMonsterClass(selectedMonster);
    if(!monsterClass)
        return;

    setNewPrimaryImage(monsterClass->getIcon(), nullptr, ui->btnCharacterIcon, 180, 260, QString(":/img/data/portrait.png"));
}

void NewEntryDialog::readMediaFile()
{
    readNewFile(ui->edtMediaFile->text().trimmed(), ui->lblMediaPreview, ui->lblMediaPreview->width() - 20, ui->lblMediaPreview->height() - 20, QString(":/img/data/icon_media.png"));
}

void NewEntryDialog::browseMediaFile()
{
    QString newMediaFile = QFileDialog::getOpenFileName(this, QString("Select Media File"));
    if(newMediaFile.isEmpty())
        return;

    ui->edtMediaFile->setText(newMediaFile);
    readMediaFile();
}

void NewEntryDialog::readMapFile()
{
    readNewFile(ui->edtMapFile->text().trimmed(), ui->lblMapPreview, ui->lblMapPreview->width() - 20, ui->lblMapPreview->height() - 20, QString(":/img/data/icon_map.png"));
}

void NewEntryDialog::browseMapFile()
{
    QString newMapFile = QFileDialog::getOpenFileName(this, QString("Select Map File"));
    if(newMapFile.isEmpty())
        return;

    ui->edtMapFile->setText(newMapFile);
    readMapFile();
}

void NewEntryDialog::readCombatFile()
{
    readNewFile(ui->edtCombatFile->text().trimmed(), ui->lblCombatPreview, ui->lblCombatPreview->width() - 20, ui->lblCombatPreview->height() - 20, QString(":/img/data/icon_combat.png"));
    ui->edtCombatGrid->setText(QString::number(_gridSizeGuess));
}

void NewEntryDialog::browseCombatFile()
{
    QString newCombatFile = QFileDialog::getOpenFileName(this, QString("Select Combat Map File"));
    if(newCombatFile.isEmpty())
        return;

    ui->edtCombatFile->setText(newCombatFile);
    ui->edtCombatFile->setReadOnly(false);
    readMapFile();
}

void NewEntryDialog::selectCombatSource()
{
    if(!_campaign)
        return;

    _referenceMap = nullptr;

    MapSelectDialog mapSelectDlg(*_campaign, (_currentObject ? _currentObject->getID() : QUuid()));
    if(mapSelectDlg.exec() != QDialog::Accepted)
        return;

    if(mapSelectDlg.isMapSelected())
    {
        _referenceMap = mapSelectDlg.getSelectedMap();
        if(!_referenceMap)
            return;

        ui->edtCombatFile->setText(QString("Map: ") + _referenceMap->getName());
        ui->edtCombatFile->setReadOnly(true);
        ui->btnCombatBrowse->setEnabled(false);

        readNewFile(_referenceMap->getFileName(), ui->lblCombatPreview, ui->lblCombatPreview->width() - 20, ui->lblCombatPreview->height() - 20, QString(":/img/data/icon_combat.png"));

        LayerGrid* gridLayer = dynamic_cast<LayerGrid*>(_referenceMap->getLayerScene().getFirst(DMHelper::LayerType_Grid));
        ui->chkCombatGrid->setChecked(gridLayer != nullptr);
        ui->edtCombatGrid->setText(QString::number(gridLayer ? gridLayer->getConfig().getGridScale() : _gridSizeGuess));
        ui->chkCombatGrid->setEnabled(gridLayer == nullptr);

        LayerFow* fowLayer = dynamic_cast<LayerFow*>(_referenceMap->getLayerScene().getFirst(DMHelper::LayerType_Fow));
        ui->chkCombatFow->setChecked(fowLayer != nullptr);
        ui->chkCombatFow->setEnabled(fowLayer == nullptr);
    }
    else if(mapSelectDlg.isBlankMap())
    {
        _imageColor = Qt::white;
        _imageSize = QSize(400, 300);
        MapBlankDialog blankDlg;
        int result = blankDlg.exec();
        if(result == QDialog::Accepted)
        {
            _imageColor = blankDlg.getMapColor();
            _imageSize = blankDlg.getMapSize();
        }

        QPixmap blankPixmap(_imageSize);
        blankPixmap.fill(_imageColor);

        _primaryImageFile = QString();
        _imageType = DMHelper::FileType_Unknown;
        _gridSizeGuess = DMHelper::STARTING_GRID_SCALE;

        ui->edtCombatFile->setText(QString("Blank Map"));
        ui->edtCombatFile->setReadOnly(true);
        ui->btnCombatBrowse->setEnabled(false);
        ui->chkCombatFow->setEnabled(true);
        ui->chkCombatGrid->setEnabled(true);
        ui->chkCombatGrid->setChecked(false);
        ui->edtCombatGrid->setText(QString::number(_gridSizeGuess));
    }
    else if(mapSelectDlg.isNewMapImage())
    {
        browseCombatFile();

        ui->btnCombatBrowse->setEnabled(true);
        ui->chkCombatFow->setEnabled(true);
        ui->chkCombatGrid->setEnabled(true);
        ui->chkCombatGrid->setChecked(false);
    }

    validateNewEntry();
}

bool NewEntryDialog::isSelectedEntryValid()
{
    if(getNewEntryName().isEmpty())
        return false;

    if (ui->buttonGroupType->checkedButton() == ui->btnTypeText)
    {
        // No further checks
    }
    else if (ui->buttonGroupType->checkedButton() == ui->btnTypeLinked)
    {
        if(ui->edtLinkedFile->text().isEmpty())
            return false;
    }
    else if (ui->buttonGroupType->checkedButton() == ui->btnTypeParty)
    {
        // No further checks
    }
    else if (ui->buttonGroupType->checkedButton() == ui->btnTypeCharacter)
    {
        if(((ui->buttonGroupType->checkedButton() == ui->btnCharacterMonster) && (ui->cmbCharacterMonster->currentText().isEmpty())) ||
           ((ui->buttonGroupType->checkedButton() == ui->btnCharacterDnDBeyond) && (ui->edtCharacterDndBeyond->text().isEmpty())))
            return false;
    }
    else if (ui->buttonGroupType->checkedButton() == ui->btnTypeMedia)
    {
        if((ui->edtMediaFile->text().isEmpty()) || (_imageType == DMHelper::FileType_Unknown))
            return false;
    }
    else if (ui->buttonGroupType->checkedButton() == ui->btnTypeMap)
    {
        if((ui->edtMapFile->text().isEmpty()) || (_imageType == DMHelper::FileType_Unknown))
            return false;
    }
    else if (ui->buttonGroupType->checkedButton() == ui->btnTypeCombat)
    {
        if(ui->edtCombatFile->text().isEmpty())
            return false;
    }

    return true;
}

void NewEntryDialog::readNewFile(const QString& filename, QLabel* label, int width, int height, const QString& defaultIcon)
{
    if((filename == _primaryImageFile) && (!label))
        return;

    disconnectScreenshot();

    QMimeType mimeType = QMimeDatabase().mimeTypeForFile(filename);
    if(mimeType.name().startsWith("image/"))
    {
        setNewPrimaryImage(filename, label, nullptr, width, height, defaultIcon);
    }
    else if(mimeType.name().startsWith("video/"))
    {
        label->setPixmap(QPixmap());

        _screenshot = new VideoPlayerScreenshot(filename);
        connect(_screenshot, &VideoPlayerScreenshot::screenshotReady, this, &NewEntryDialog::handleScreenshotReady);
        connect(_screenshot, &QObject::destroyed, this, &NewEntryDialog::disconnectScreenshot);
        _screenshot->retrieveScreenshot();

        _primaryImageFile = filename;
        _imageType = DMHelper::FileType_Video;
    }
    else
    {
        label->setPixmap(QPixmap(defaultIcon).scaled(width, height, Qt::KeepAspectRatio));
        _imageType = DMHelper::FileType_Unknown;
    }

    validateNewEntry();
}

void NewEntryDialog::selectNewPrimaryImage(QLabel* label, QPushButton* button, int width, int height, const QString& query, const QString& defaultIcon)
{
    setNewPrimaryImage(QFileDialog::getOpenFileName(this, query), label, button, width, height, defaultIcon);
}

void NewEntryDialog::setNewPrimaryImage(const QString& newPrimaryImage, QLabel* label, QPushButton* button, int width, int height, const QString& defaultIcon)
{
    _primaryImageFile = QString();
    _imageType = DMHelper::FileType_Unknown;
    _gridSizeGuess = DMHelper::STARTING_GRID_SCALE;

    QPixmap newPrimaryImagePixmap;

    if(!newPrimaryImage.isEmpty())
    {
        QImage tempLoadImage(newPrimaryImage);
        if(!tempLoadImage.isNull())
        {
            newPrimaryImagePixmap.fromImage(tempLoadImage);
            if(!newPrimaryImagePixmap.isNull())
            {
                _primaryImageFile = newPrimaryImage;
                _imageType = DMHelper::FileType_Image;
                _gridSizeGuess = qRound(static_cast<qreal>(tempLoadImage.dotsPerMeterX()) * 0.0254);
            }
        }
    }

    if(newPrimaryImagePixmap.isNull())
        newPrimaryImagePixmap = QPixmap(defaultIcon);

    newPrimaryImagePixmap = newPrimaryImagePixmap.scaled(width, height, Qt::KeepAspectRatio);

    if(label)
        label->setPixmap(newPrimaryImagePixmap);

    if(button)
        button->setIcon(QIcon(newPrimaryImagePixmap));

    validateNewEntry();
}

void NewEntryDialog::loadPrimaryImage(QLabel* label, QPushButton* button, int width, int height, const QString& defaultIcon)
{
    QPixmap primaryImagePixmap = QPixmap(_primaryImageFile.isEmpty() ? defaultIcon : _primaryImageFile).scaled(width, height, Qt::KeepAspectRatio);

    if(label)
        label->setPixmap(primaryImagePixmap);

    if(button)
        button->setIcon(QIcon(primaryImagePixmap));
}

void NewEntryDialog::handleScreenshotReady(const QImage& image)
{
    if(image.isNull())
    {
        _imageType = DMHelper::FileType_Unknown;
        validateNewEntry();
        return;
    }

    QLabel* targetLabel = nullptr;

    if(ui->buttonGroupType->checkedButton() == ui->btnTypeMedia)
        targetLabel = ui->lblMediaPreview;
    else if(ui->buttonGroupType->checkedButton() == ui->btnTypeMap)
        targetLabel = ui->lblMapPreview;
    else if(ui->buttonGroupType->checkedButton() == ui->btnTypeCombat)
        targetLabel = ui->lblCombatPreview;

    if(!targetLabel)
        return;

    _gridSizeGuess = qRound(static_cast<qreal>(image.dotsPerMeterX()) * 0.0254);
    QPixmap imagePixmap = QPixmap::fromImage(image).scaled(targetLabel->width() - 20, targetLabel->height() - 20, Qt::KeepAspectRatio);
    targetLabel->setPixmap(imagePixmap);
}

void NewEntryDialog::disconnectScreenshot()
{
    if(!_screenshot)
        return;

    disconnect(_screenshot, &VideoPlayerScreenshot::screenshotReady, this, &NewEntryDialog::handleScreenshotReady);
    disconnect(_screenshot, &QObject::destroyed, this, &NewEntryDialog::disconnectScreenshot);

    _screenshot = nullptr;
}
