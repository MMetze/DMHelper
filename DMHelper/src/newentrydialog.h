#ifndef NEWENTRYDIALOG_H
#define NEWENTRYDIALOG_H

#include <QDialog>
#include "dmconstants.h"

namespace Ui {
class NewEntryDialog;
}

class Campaign;
class OptionsContainer;
class CampaignObjectBase;
class QLabel;
class VideoPlayerScreenshot;
class Map;

class NewEntryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewEntryDialog(Campaign* campaign, OptionsContainer* options, CampaignObjectBase* currentObject, QWidget *parent = nullptr);
    ~NewEntryDialog();

    void setEntryType(DMHelper::CampaignType type, const QString& filename = QString());
    void setEntryFile(const QString& filename);

    CampaignObjectBase* createNewEntry();
    bool isImportNeeded();
    DMHelper::CampaignType getEntryType();

    QString getNewEntryName() const;
    QString getImportString() const;

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

    CampaignObjectBase* createTextEntry();
    CampaignObjectBase* createLinkedEntry();
    CampaignObjectBase* createPartyEntry();
    CampaignObjectBase* createCharacterEntry();
    CampaignObjectBase* createMediaEntry();
    CampaignObjectBase* createMapEntry();
    CampaignObjectBase* createBattleEntry();

protected slots:
    void validateNewEntry();
    void newPageSelected();

    // Text Page members
    void readTextFile(const QString& filename);

    // Linked Text Page members
    void setLinkedTextFile(const QString& filename);
    void browseLinkedTextFile();

    // Party Page members
    void selectPartyIcon();

    // Character Page members
    void selectCharacterIcon();
    void editCharacterIcon();
    void importHeroForge();
    void loadMonsterList();
    void loadMonsterIcon();

    // Media Page members
    void readMediaFile(const QString& mediaFile);
    void readMediaFileFromEdit();
    void browseMediaFile();

    // Map Page members
    void readMapFile(const QString& mapFile);
    void readMapFileFromEdit();
    void browseMapFile();

    // Combat Page members
    void readCombatFile(const QString& combatFile);
    void readCombatFileFromEdit();
    void browseCombatFile();
    void selectCombatSource();

    // Helper functions
    bool isSelectedEntryValid();

    void readNewFile(const QString& filename, QLabel* label, int width, int height, const QString& defaultIcon);
    void selectNewPrimaryImage(QLabel* label, QPushButton* button, int width, int height, const QString& query, const QString& defaultIcon);
    void setNewPrimaryImage(const QString& newPrimaryImage, QLabel* label, QPushButton* button, int width, int height, const QString& defaultIcon);
    void loadPrimaryImage(QLabel* label, QPushButton* button, int width, int height, const QString& defaultIcon);

    void handleScreenshotReady(const QImage& image);
    void disconnectScreenshot();

private:
    Ui::NewEntryDialog *ui;

    Campaign* _campaign;
    OptionsContainer* _options;
    CampaignObjectBase* _currentObject;
    QString _primaryImageFile;
    VideoPlayerScreenshot* _screenshot;
    DMHelper::FileType _imageType;
    int _gridSizeGuess;

    Map* _referenceMap;
    QSize _imageSize;
    QColor _imageColor;
};

#endif // NEWENTRYDIALOG_H
