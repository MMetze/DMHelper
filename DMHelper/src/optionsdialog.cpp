#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QIntValidator>

OptionsDialog::OptionsDialog(OptionsContainer* options, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog),
    _options(options)
{
    ui->setupUi(this);

    ui->cmbInitiativeType->addItem("No Initiative", QVariant(DMHelper::InitiativeType_None));
    ui->cmbInitiativeType->addItem("Icons Only", QVariant(DMHelper::InitiativeType_Image));
    ui->cmbInitiativeType->addItem("Icons and Names", QVariant(DMHelper::InitiativeType_ImageName));

    if(_options)
    {
        updateFileLocations();

        connect(ui->btnBestiary, &QAbstractButton::clicked, this, &OptionsDialog::browseBestiary);
        connect(ui->edtBestiary, &QLineEdit::editingFinished, this, &OptionsDialog::editBestiary);
        connect(ui->btnSpellbook, &QAbstractButton::clicked, this, &OptionsDialog::browseSpellbook);
        connect(ui->edtSpellbook, &QLineEdit::editingFinished, this, &OptionsDialog::editSpellbook);
        connect(ui->btnQuickReference, &QAbstractButton::clicked, this, &OptionsDialog::browseQuickReference);
        connect(ui->edtQuickReference, &QLineEdit::editingFinished, this, &OptionsDialog::editQuickReference);
        connect(ui->btnCalendar, &QAbstractButton::clicked, this, &OptionsDialog::browseCalendar);
        connect(ui->edtCalendar, &QLineEdit::editingFinished, this, &OptionsDialog::editCalendar);
        connect(ui->btnEquipment, &QAbstractButton::clicked, this, &OptionsDialog::browseEquipment);
        connect(ui->edtEquipment, &QLineEdit::editingFinished, this, &OptionsDialog::editEquipment);
        connect(ui->btnShops, &QAbstractButton::clicked, this, &OptionsDialog::browseShops);
        connect(ui->edtShops, &QLineEdit::editingFinished, this, &OptionsDialog::editShops);
        connect(ui->btnTables, &QAbstractButton::clicked, this, &OptionsDialog::browseTables);
        connect(ui->edtTables, &QLineEdit::editingFinished, this, &OptionsDialog::editTables);

        connect(ui->btnResetFileLocations, &QAbstractButton::clicked, this, &OptionsDialog::resetFileLocations);

        QFont font = qApp->font();
        font.setFamily(_options->getFontFamily());
        ui->fontComboBox->setCurrentFont(font);
        ui->spinBoxFontSize->setValue(_options->getFontSize());
        ui->cmbInitiativeType->setCurrentIndex(_options->getInitiativeType());
        ui->chkShowCountdown->setChecked(_options->getShowCountdown());
        ui->edtCountdownDuration->setValidator(new QIntValidator(1, 1000, this));
        ui->edtCountdownDuration->setText(QString::number(_options->getCountdownDuration()));
        ui->edtPointerFile->setText(_options->getPointerFile());
        ui->edtSelectedIcon->setText(_options->getSelectedIcon());
        ui->edtActiveIcon->setText(_options->getActiveIcon());
        ui->edtCombatantFrame->setText(_options->getCombatantFrame());
        ui->edtCountdownFrame->setText(_options->getCountdownFrame());
        ui->edtHeroForgeAccessKey->setText(_options->getHeroForgeToken());
#ifdef INCLUDE_NETWORK_SUPPORT
        ui->chkEnableNetworkClient->setChecked(_options->getNetworkEnabled());
        ui->edtUserName->setText(_options->getUserName());
        ui->edtURL->setText(_options->getURLString());
        ui->chkSavePassword->setChecked(_options->getSavePassword());
        ui->edtPassword->setText(_options->getPassword());
        ui->edtSessionID->setText(_options->getSessionID());
        ui->edtInviteID->setText(_options->getInviteID());

        ui->lblURL->setEnabled(_options->getNetworkEnabled());
        ui->edtURL->setEnabled(_options->getNetworkEnabled());
        ui->lblUsername->setEnabled(_options->getNetworkEnabled());
        ui->edtUserName->setEnabled(_options->getNetworkEnabled());
        ui->lblPassword->setEnabled(_options->getNetworkEnabled());
        ui->chkSavePassword->setEnabled(_options->getNetworkEnabled());
        ui->edtPassword->setEnabled(_options->getNetworkEnabled());
        ui->lblSessionID->setEnabled(_options->getNetworkEnabled());
        ui->edtSessionID->setEnabled(_options->getNetworkEnabled());
        ui->lblInviteID->setEnabled(_options->getNetworkEnabled());
        ui->edtInviteID->setEnabled(_options->getNetworkEnabled());
        ui->btnGenerateInvite->setEnabled(_options->getNetworkEnabled());
#else
        ui->tabWidget->removeTab(2);
#endif

        connect(ui->fontComboBox, SIGNAL(currentFontChanged(const QFont &)), _options, SLOT(setFontFamilyFromFont(const QFont&)));
        connect(ui->spinBoxFontSize, SIGNAL(valueChanged(int)), _options, SLOT(setFontSize(int)));
        connect(ui->cmbInitiativeType, SIGNAL(currentIndexChanged(int)), _options, SLOT(setInitiativeType(int)));
        connect(ui->chkShowCountdown, SIGNAL(clicked(bool)), _options, SLOT(setShowCountdown(bool)));
        connect(ui->edtCountdownDuration, SIGNAL(textChanged(QString)), _options, SLOT(setCountdownDuration(QString)));
        connect(ui->btnPointer, &QAbstractButton::clicked, this, &OptionsDialog::browsePointerFile);
        connect(ui->edtPointerFile, &QLineEdit::editingFinished, this, &OptionsDialog::editPointerFile);
        connect(ui->btnSelectedIcon, &QAbstractButton::clicked, this, &OptionsDialog::browseSelectedIcon);
        connect(ui->edtSelectedIcon, &QLineEdit::editingFinished, this, &OptionsDialog::editSelectedIcon);
        connect(ui->btnActiveIcon, &QAbstractButton::clicked, this, &OptionsDialog::browseActiveIcon);
        connect(ui->edtActiveIcon, &QLineEdit::editingFinished, this, &OptionsDialog::editActiveIcon);
        connect(ui->btnCombatantFrame, &QAbstractButton::clicked, this, &OptionsDialog::browseCombatantFrame);
        connect(ui->edtCombatantFrame, &QLineEdit::editingFinished, this, &OptionsDialog::editCombatantFrame);
        connect(ui->btnCountdownFrame, &QAbstractButton::clicked, this, &OptionsDialog::browseCountdownFrame);
        connect(ui->edtCountdownFrame, &QLineEdit::editingFinished, this, &OptionsDialog::editCountdownFrame);
        connect(ui->edtHeroForgeAccessKey, &QLineEdit::editingFinished, this, &OptionsDialog::heroForgeTokenEdited);

#ifdef INCLUDE_NETWORK_SUPPORT
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), _options, SLOT(setNetworkEnabled(bool)));
        connect(ui->edtURL, SIGNAL(textChanged(QString)), _options, SLOT(setURLString(QString)));
        connect(ui->edtUserName, SIGNAL(textChanged(QString)), _options, SLOT(setUserName(QString)));
        connect(ui->chkSavePassword, SIGNAL(clicked(bool)), _options, SLOT(setSavePassword(bool)));
        connect(ui->edtPassword, SIGNAL(textChanged(QString)), _options, SLOT(setPassword(QString)));
        connect(ui->edtSessionID, SIGNAL(textChanged(QString)), _options, SLOT(setSessionID(QString)));
        connect(ui->edtInviteID, SIGNAL(textChanged(QString)), _options, SLOT(setInviteID(QString)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->lblURL, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->edtURL, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->lblUsername, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->edtUserName, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->lblPassword, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->chkSavePassword, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->edtPassword, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->lblSessionID, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->edtSessionID, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->lblInviteID, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->edtInviteID, SLOT(setEnabled(bool)));
        connect(ui->chkEnableNetworkClient, SIGNAL(clicked(bool)), ui->btnGenerateInvite, SLOT(setEnabled(bool)));
#endif

        ui->chkEnableUpdates->setChecked(_options->isUpdatesEnabled());
        connect(ui->chkEnableUpdates, SIGNAL(clicked(bool)), _options, SLOT(setUpdatesEnabled(bool)));
        ui->chkAllowStatistics->setChecked(_options->isStatisticsAccepted());
        connect(ui->chkAllowStatistics, SIGNAL(clicked(bool)), _options, SLOT(setStatisticsAccepted(bool)));
    }
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

OptionsContainer* OptionsDialog::getOptions() const
{
    return _options;
}

void OptionsDialog::browseBestiary()
{
    setBestiary(QFileDialog::getOpenFileName(this, QString("Select Bestiary File"), QString(), QString("XML files (*.xml)")));
}

void OptionsDialog::editBestiary()
{
    setBestiary(ui->edtBestiary->text());
}

void OptionsDialog::setBestiary(const QString& bestiaryFile)
{
    if(bestiaryFile.isEmpty())
        return;

    if(!QFile::exists(bestiaryFile))
    {
        QMessageBox::critical(this, QString("Bestiary file not found"), QString("The selected bestiary file could not be found!") + QChar::LineFeed + bestiaryFile);
        return;
    }

    ui->edtBestiary->setText(bestiaryFile);
    _options->setBestiaryFileName(bestiaryFile);
}

void OptionsDialog::browseSpellbook()
{
    setSpellbook(QFileDialog::getOpenFileName(this, QString("Select Spellbook File"), QString(), QString("XML files (*.xml)")));
}

void OptionsDialog::editSpellbook()
{
    setSpellbook(ui->edtSpellbook->text());
}

void OptionsDialog::setSpellbook(const QString& spellbookFile)
{
    if(spellbookFile.isEmpty())
        return;

    if(!QFile::exists(spellbookFile))
    {
        QMessageBox::critical(this, QString("Spellbook file not found"), QString("The selected spellbook file could not be found!") + QChar::LineFeed + spellbookFile);
        return;
    }

    ui->edtSpellbook->setText(spellbookFile);
    _options->setSpellbookFileName(spellbookFile);
}

void OptionsDialog::browseQuickReference()
{
    setQuickReference(QFileDialog::getOpenFileName(this, QString("Select Quick Reference File"), QString(), QString("XML files (*.xml)")));
}

void OptionsDialog::editQuickReference()
{
    setQuickReference(ui->edtQuickReference->text());
}

void OptionsDialog::setQuickReference(const QString& quickRefFile)
{
    if(quickRefFile.isEmpty())
        return;

    if(!QFile::exists(quickRefFile))
    {
        QMessageBox::critical(this, QString("Quick Reference file not found"), QString("The selected quick reference file could not be found!") + QChar::LineFeed + quickRefFile);
        return;
    }

    ui->edtQuickReference->setText(quickRefFile);
    _options->setQuickReferenceFileName(quickRefFile);
}

void OptionsDialog::browseCalendar()
{
    setCalendar(QFileDialog::getOpenFileName(this, QString("Select Calendar File"), QString(), QString("XML files (*.xml)")));
}

void OptionsDialog::editCalendar()
{
    setCalendar(ui->edtCalendar->text());
}

void OptionsDialog::setCalendar(const QString& calendarFile)
{
    if(calendarFile.isEmpty())
        return;

    if(!QFile::exists(calendarFile))
    {
        QMessageBox::critical(this, QString("Calendar file not found"), QString("The selected calendar file could not be found!") + QChar::LineFeed + calendarFile);
        return;
    }

    ui->edtCalendar->setText(calendarFile);
    _options->setCalendarFileName(calendarFile);
}

void OptionsDialog::browseEquipment()
{
    setEquipment(QFileDialog::getOpenFileName(this, QString("Select Equipment File"), QString(), QString("XML files (*.xml)")));
}

void OptionsDialog::editEquipment()
{
    setEquipment(ui->edtEquipment->text());
}

void OptionsDialog::setEquipment(const QString& equipmentFile)
{
    if(equipmentFile.isEmpty())
        return;

    if(!QFile::exists(equipmentFile))
    {
        QMessageBox::critical(this, QString("Equipment file not found"), QString("The selected equipment file could not be found!") + QChar::LineFeed + equipmentFile);
        return;
    }

    ui->edtEquipment->setText(equipmentFile);
    _options->setEquipmentFileName(equipmentFile);
}

void OptionsDialog::browseShops()
{
    setShops(QFileDialog::getOpenFileName(this, QString("Select Shops File"), QString(), QString("XML files (*.xml)")));

}

void OptionsDialog::editShops()
{
    setShops(ui->edtShops->text());
}

void OptionsDialog::setShops(const QString& shopsFile)
{
    if(shopsFile.isEmpty())
        return;

    if(!QFile::exists(shopsFile))
    {
        QMessageBox::critical(this, QString("Shops file not found"), QString("The selected shops file could not be found!") + QChar::LineFeed + shopsFile);
        return;
    }

    ui->edtShops->setText(shopsFile);
    _options->setShopsFileName(shopsFile);
}

void OptionsDialog::browseTables()
{
    setTables(QFileDialog::getExistingDirectory(this, QString("Select Tables Directory"), QString()));
}

void OptionsDialog::editTables()
{
    setTables(ui->edtTables->text());
}

void OptionsDialog::setTables(const QString& tablesDirectory)
{
    if(tablesDirectory.isEmpty())
        return;

    if(!QDir(tablesDirectory).exists())
    {
        QMessageBox::critical(this, QString("Tables directory not found"), QString("The selected tables directory could not be found!") + QChar::LineFeed + tablesDirectory);
        return;
    }

    ui->edtTables->setText(tablesDirectory);
    _options->setTablesDirectory(tablesDirectory);
}

void OptionsDialog::browsePointerFile()
{
    setPointerFile(QFileDialog::getOpenFileName(this, QString("Select Pointer File")));
}

void OptionsDialog::editPointerFile()
{
    setPointerFile(ui->edtPointerFile->text());
}

void OptionsDialog::setPointerFile(const QString& pointerFile)
{
    ui->edtPointerFile->setText(pointerFile);
    _options->setPointerFileName(pointerFile);
}

void OptionsDialog::browseSelectedIcon()
{
    setSelectedIcon(QFileDialog::getOpenFileName(this, QString("Select Icon for Selected Tokens")));
}

void OptionsDialog::editSelectedIcon()
{
    setSelectedIcon(ui->edtSelectedIcon->text());
}

void OptionsDialog::setSelectedIcon(const QString& selectedIcon)
{
    ui->edtSelectedIcon->setText(selectedIcon);
    _options->setSelectedIcon(selectedIcon);
}

void OptionsDialog::browseActiveIcon()
{
    setActiveIcon(QFileDialog::getOpenFileName(this, QString("Select Icon for Active Combatants")));
}

void OptionsDialog::editActiveIcon()
{
    setActiveIcon(ui->edtActiveIcon->text());
}

void OptionsDialog::setActiveIcon(const QString& activeIcon)
{
    ui->edtActiveIcon->setText(activeIcon);
    _options->setActiveIcon(activeIcon);
}

void OptionsDialog::browseCombatantFrame()
{
    setCombatantFrame(QFileDialog::getOpenFileName(this, QString("Select Image for the Combatant Frame")));
}

void OptionsDialog::editCombatantFrame()
{
    setCombatantFrame(ui->edtCombatantFrame->text());
}

void OptionsDialog::setCombatantFrame(const QString& combatantFrame)
{
    ui->edtCombatantFrame->setText(combatantFrame);
    _options->setCombatantFrame(combatantFrame);
}

void OptionsDialog::browseCountdownFrame()
{
    setCountdownFrame(QFileDialog::getOpenFileName(this, QString("Select Image for the Countdown Frame")));
}

void OptionsDialog::editCountdownFrame()
{
    setCountdownFrame(ui->edtCountdownFrame->text());
}

void OptionsDialog::setCountdownFrame(const QString& countdownFrame)
{
    ui->edtCountdownFrame->setText(countdownFrame);
    _options->setCountdownFrame(countdownFrame);
}

void OptionsDialog::heroForgeTokenEdited()
{
    QString newToken = ui->edtHeroForgeAccessKey->text();
    if(_options->getHeroForgeToken() == newToken)
        return;

    if(_options->getHeroForgeToken().isEmpty())
    {
        if(QMessageBox::question(this,
                                 QString("Confirm Store Access Key"),
                                 QString("DMHelper will store your access key for ease of use in the future.") + QChar::LineFeed + QChar::LineFeed + QString("The Access Key will be stored locally on your computer without encryption, it is possible that other applications will be able to access it.") + QChar::LineFeed + QChar::LineFeed + QString("Are you sure?")) != QMessageBox::Yes)
        {
            ui->edtHeroForgeAccessKey->setText(QString());
            return;
        }
    }

    _options->setHeroForgeToken(newToken);
}

void OptionsDialog::updateFileLocations()
{
    if(!_options)
        return;

    ui->edtBestiary->setText(_options->getBestiaryFileName());
    ui->edtSpellbook->setText(_options->getSpellbookFileName());
    ui->edtQuickReference->setText(_options->getQuickReferenceFileName());
    ui->edtCalendar->setText(_options->getCalendarFileName());
    ui->edtEquipment->setText(_options->getEquipmentFileName());
    ui->edtShops->setText(_options->getShopsFileName());
    ui->edtTables->setText(_options->getTablesDirectory());
}

void OptionsDialog::resetFileLocations()
{
    if(!_options)
        return;

    QMessageBox::StandardButton result = QMessageBox::critical(this,
                                                               QString("Confirm File Location Reset"),
                                                               QString("All file paths will be reset to their default locations. None of your current files will be deleted, but if default files do not exist, they will be created.") + QChar::LineFeed + QString("Are you sure you want to do this?"),
                                                               QMessageBox::Yes | QMessageBox::No);

    if(result != QMessageBox::Yes)
        return;

    _options->resetFileSettings();
    updateFileLocations();
}
