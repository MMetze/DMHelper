#include "mapmarkerdialog.h"
#include "ui_mapmarkerdialog.h"
#include "campaign.h"
#include "character.h"
#include "audiotrack.h"
#include "optionsaccessor.h"
#include "ribbonframe.h"
#include "mapcolorizefilter.h"
#include <QFileInfo>
#include <QFileDialog>
#include <QPainter>
#include <QMessageBox>

MapMarkerDialog::MapMarkerDialog(const MapMarker& marker, Map& map, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MapMarkerDialog),
    _menu(new QMenu(this)),
    _marker(marker),
    _currentIcon(marker.getIconFile()),
    _icons(),
    _iconDim(40)
{
    ui->setupUi(this);
    connect(ui->btnDelete, &QAbstractButton::clicked, this, &MapMarkerDialog::deleteMarker);

    ui->chkApplyColor->setChecked(marker.isColoredIcon());
    connect(ui->chkApplyColor, &QAbstractButton::clicked, this, &MapMarkerDialog::applyColorChecked);

    ui->edtTitle->setText(marker.getTitle());
    ui->txtDescription->setPlainText(marker.getDescription());
    ui->btnColor->setColor(marker.getColor());
    ui->btnColor->setRotationVisible(false);
    ui->spinScale->setValue(marker.getIconScale());

    ui->btnIcon->setMenu(_menu);

    MapMarkerDialog_IconAction* defaultAction = new MapMarkerDialog_IconAction(QString(":/img/data/icon_pin.png"), QString("Pin"));
    connect(defaultAction, &MapMarkerDialog_IconAction::iconFileSelected, this, &MapMarkerDialog::iconSelected);
    _menu->addAction(defaultAction);

    MapMarkerDialog_IconAction* banner1Action = new MapMarkerDialog_IconAction(QString(":/img/data/icon_banner_1.png"), QString("Banner #1"));
    connect(banner1Action, &MapMarkerDialog_IconAction::iconFileSelected, this, &MapMarkerDialog::iconSelected);
    _menu->addAction(banner1Action);

    MapMarkerDialog_IconAction* banner2Action = new MapMarkerDialog_IconAction(QString(":/img/data/icon_banner_2.png"), QString("Banner #2"));
    connect(banner2Action, &MapMarkerDialog_IconAction::iconFileSelected, this, &MapMarkerDialog::iconSelected);
    _menu->addAction(banner2Action);

    MapMarkerDialog_IconAction* banner3Action = new MapMarkerDialog_IconAction(QString(":/img/data/icon_banner_3.png"), QString("Banner #3"));
    connect(banner3Action, &MapMarkerDialog_IconAction::iconFileSelected, this, &MapMarkerDialog::iconSelected);
    _menu->addAction(banner3Action);

    MapMarkerDialog_IconAction* flagAction = new MapMarkerDialog_IconAction(QString(":/img/data/icon_flag.png"), QString("Flag"));
    connect(flagAction, &MapMarkerDialog_IconAction::iconFileSelected, this, &MapMarkerDialog::iconSelected);
    _menu->addAction(flagAction);

    QAction* chooseAction = new QAction(QString("Choose icon..."));
    connect(chooseAction, &QAction::triggered, this, &MapMarkerDialog::selectNewIcon);
    _menu->addAction(chooseAction);

    _menu->addSeparator();

    if(!populateIconList())
        defaultAction->trigger();

    populateCombo(marker.getEncounter(), map);
}

MapMarkerDialog::~MapMarkerDialog()
{
    delete ui;

    OptionsAccessor settings;

    settings.setValue("lastMapIcon", _currentIcon);

    settings.beginGroup("MapIcons");
        settings.remove("");
        QString keyName;
        int maxIcon = qMin(10, _icons.count());
        for(int i = 0; i < maxIcon; ++i)
        {
            keyName = "Iconfile ";
            keyName.append(QString::number(i));
            settings.setValue(keyName, _icons.at(i));
        }
    settings.endGroup(); // MapIcons
}

MapMarker MapMarkerDialog::getMarker()
{
    _marker.setTitle(ui->edtTitle->text());
    _marker.setDescription(ui->txtDescription->toPlainText());
    _marker.setEncounter(ui->cmbEncounter->currentIndex() == -1 ? QUuid() : QUuid(ui->cmbEncounter->currentData().toString()));
    _marker.setColor(ui->btnColor->getColor());
    _marker.setIconFile(_currentIcon);
    _marker.setIconScale(ui->spinScale->value());
    _marker.setColoredIcon(ui->chkApplyColor->isChecked());

    return _marker;
}

void MapMarkerDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    QFontMetrics metrics = ui->lblScale->fontMetrics();
    int ribbonHeight = RibbonFrame::getRibbonHeight();
    int labelHeight = RibbonFrame::getLabelHeight(metrics, ribbonHeight);
    _iconDim = ribbonHeight - labelHeight;
    RibbonFrame::setButtonSize(*ui->btnIcon, _iconDim * 3 / 2, _iconDim);
    RibbonFrame::setButtonSize(*ui->btnColor, _iconDim, _iconDim);
}

void MapMarkerDialog::deleteMarker()
{
    done(MAPMARKERDIALOG_DELETE);
}

void MapMarkerDialog::iconSelected(const QString& iconFile)
{
    if(!_menu)
        return;

    if(setColoredIcon(iconFile))
        _currentIcon = iconFile;
}

void MapMarkerDialog::selectNewIcon(bool checked)
{
    Q_UNUSED(checked);

    QString iconFile = QFileDialog::getOpenFileName(nullptr, QString("Select new map marker image file"));
    if((!iconFile.isEmpty()) && (!QIcon(iconFile).isNull()))
    {
        MapMarkerDialog_IconAction* newAction = new MapMarkerDialog_IconAction(iconFile);
        connect(newAction, &MapMarkerDialog_IconAction::iconFileSelected, this, &MapMarkerDialog::iconSelected);
        _menu->addAction(newAction);
        _icons.prepend(iconFile);
        newAction->trigger();
    }
}

void MapMarkerDialog::applyColorChecked(bool checked)
{
    Q_UNUSED(checked);

    if(!_currentIcon.isEmpty())
        setColoredIcon(_currentIcon);
}

bool MapMarkerDialog::setColoredIcon(const QString& iconFile)
{
    QImage iconScaled = QImage(iconFile).scaled(_iconDim, _iconDim, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    if(iconScaled.isNull())
        return false;

    QImage iconColored;
    if(ui->chkApplyColor->isChecked())
    {
        MapColorizeFilter filter;
        QColor filterColor = ui->btnColor->getColor();

        filter._r2r = 0.33 * filterColor.redF();
        filter._g2r = 0.33 * filterColor.redF();
        filter._b2r = 0.33 * filterColor.redF();
        filter._r2g = 0.33 * filterColor.greenF();
        filter._g2g = 0.33 * filterColor.greenF();
        filter._b2g = 0.33 * filterColor.greenF();
        filter._r2b = 0.33 * filterColor.blueF();
        filter._g2b = 0.33 * filterColor.blueF();
        filter._b2b = 0.33 * filterColor.blueF();

        iconColored = filter.apply(iconScaled);
    }
    else
    {
        iconColored = iconScaled;
    }

     ui->btnIcon->setIcon(QIcon(QPixmap::fromImage(iconColored)));
     return true;
}

bool MapMarkerDialog::populateIconList()
{
    if(!_menu)
        return 0;

    QStringList keys;
    bool defaultFound = false;

    _icons.clear();

    OptionsAccessor settings;

    if(_currentIcon.isEmpty())
        _currentIcon = settings.value("lastMapIcon").toString();

    settings.beginGroup("MapIcons");

        keys = settings.allKeys();
        for(QString key : keys)
        {
            QString fileValue = settings.value(key).toString();
            if((!fileValue.isEmpty()) && (QFile::exists(fileValue)))
            {
                MapMarkerDialog_IconAction* keyAction = new MapMarkerDialog_IconAction(fileValue);
                connect(keyAction, &MapMarkerDialog_IconAction::iconFileSelected, this, &MapMarkerDialog::iconSelected);
                _menu->addAction(keyAction);
                _icons.append(fileValue);

                if(fileValue == _currentIcon)
                {
                    defaultFound = true;
                    keyAction->trigger();
                }
            }
        }

    settings.endGroup(); // MapIcons

    return defaultFound;
}

void MapMarkerDialog::populateCombo(const QUuid& encounter, Map& map)
{
    Campaign* campaign = dynamic_cast<Campaign*>(map.getParentByType(DMHelper::CampaignType_Campaign));
    if(!campaign)
        return;

    QList<CampaignObjectBase*> childList = campaign->getChildObjects();
    for(CampaignObjectBase* childObject : childList)
    {
        addChildEntry(childObject, 0, encounter);
    }
}

void MapMarkerDialog::addChildEntry(CampaignObjectBase* object, int depth, const QUuid& encounter)
{
    if(!object)
        return;

    QString itemName;
    if(depth > 3)
        itemName.fill(' ', depth - 3);
    itemName.prepend("   ");
    if(depth > 0)
        itemName.append(QString("|--"));
    itemName.append(object->getName());
    ui->cmbEncounter->addItem(objectIcon(object), itemName, QVariant(object->getID().toString()));
    if(object->getID() == encounter)
        ui->cmbEncounter->setCurrentIndex(ui->cmbEncounter->count() - 1);

    QList<CampaignObjectBase*> childList = object->getChildObjects();
    for(CampaignObjectBase* childObject : childList)
    {
        addChildEntry(childObject, depth + 3, encounter);
    }
}

QIcon MapMarkerDialog::objectIcon(CampaignObjectBase* object)
{
    if(!object)
        return QIcon();

    switch(object->getObjectType())
    {
        case DMHelper::CampaignType_Party:
            return QIcon(":/img/data/icon_contentparty.png");
        case DMHelper::CampaignType_Combatant:
            {
                Character* character = dynamic_cast<Character*>(object);
                return ((character) && (character->isInParty())) ? QIcon(":/img/data/icon_contentcharacter.png") : QIcon(":/img/data/icon_contentnpc.png");
            }
        case DMHelper::CampaignType_Map:
            return QIcon(":/img/data/icon_contentmap.png");
        case DMHelper::CampaignType_Text:
            return QIcon(":/img/data/icon_contenttextencounter.png");
        case DMHelper::CampaignType_Battle:
            return QIcon(":/img/data/icon_contentbattle.png");
        case DMHelper::CampaignType_ScrollingText:
            return QIcon(":/img/data/icon_contentscrollingtext.png");
        case DMHelper::CampaignType_AudioTrack:
            {
                AudioTrack* track = dynamic_cast<AudioTrack*>(object);
                if((track) && (track->getAudioType() == DMHelper::AudioType_Syrinscape))
                    return QIcon(":/img/data/icon_syrinscape.png");
                else if((track) && (track->getAudioType() == DMHelper::AudioType_Youtube))
                    return QIcon(":/img/data/icon_playerswindow.png");
                else
                    return QIcon(":/img/data/icon_soundboard.png");
            }
        default:
            return QIcon();
    }
}





MapMarkerDialog_IconAction::MapMarkerDialog_IconAction(const QString& iconFile, const QString& iconText, QObject *parent) :
    QAction(parent),
    _iconFile(iconFile)
{
    connect(this, &QAction::triggered, this, &MapMarkerDialog_IconAction::handleActivated);

    setIcon(QIcon(iconFile));
    setText(iconText.isEmpty() ? QFileInfo(iconFile).baseName() : iconText);
}

const QString& MapMarkerDialog_IconAction::getIconFile() const
{
    return _iconFile;
}

void MapMarkerDialog_IconAction::handleActivated(bool checked)
{
    Q_UNUSED(checked);
    emit iconFileSelected(_iconFile);
}
