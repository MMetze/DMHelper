#ifndef MAPMANAGERDIALOG_H
#define MAPMANAGERDIALOG_H

#include <QDialog>
#include <QSortFilterProxyModel>

namespace Ui {
class MapManagerDialog;
}

class OptionsContainer;
class QStandardItemModel;
class QStandardItem;
class QItemSelection;
class QDomDocument;
class QDomElement;

class MapManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MapManagerDialog(OptionsContainer& options, QWidget *parent = nullptr);
    ~MapManagerDialog();

    struct MapFileMetadata
    {
        int _type;
        QString _filePath;
        QStringList _tags;
    };

protected slots:
    virtual void showEvent(QShowEvent *event) override;

    void selectItem(const QItemSelection &current, const QItemSelection &previous);
    void openPreviewDialog(const QModelIndex &current);

    void browsePath();
    void findMaps();
    void scanNextDirectory();
    void scanDirectory(QStandardItem* parent, const QString& absolutePath);

    void addTags();
    void addTagsToItem(QStandardItem& item);
    void browseTags();
    void handleTagsEdited();

    void readModel();
    void writeModel();

private:
    void clearModel();
    QStandardItem* containsEntry(QStandardItem& item, const QString& fullPath);
    void setCurrentPath(const QString& path);
    void inputItemXML(QDomElement &element, QStandardItem& parent);
    void outputItemXML(QDomDocument &doc, QDomElement &parent, QStandardItem& item);

    void registerTag(const QString& tag);
    void registerTags(const QStringList& tags);

    QStringList proposeTags(const QString &filename) const;

    class TagFilterProxyModel;

    Ui::MapManagerDialog *ui;

    QStandardItemModel* _model;
    TagFilterProxyModel* _proxy;
    OptionsContainer& _options;
    QString _currentPath;
    QList<QPair<QStandardItem*, QString>> _searchList;
    QSet<QString> _tagList;

    // ------------------ KEYWORD → TAGS ------------------
    const QHash<QString, QStringList> keywordToTags {
        // Environments
        {"cave", {"cave", "dungeon"}},
        {"cavern", {"cave"}},
        {"forest", {"forest", "outdoor"}},
        {"swamp", {"swamp", "outdoor"}},
        {"desert", {"desert", "outdoor"}},
        {"arctic", {"snow", "outdoor"}},
        {"snow", {"snow", "outdoor"}},
        {"mountain", {"mountain", "outdoor"}},
        {"waterfall", {"waterfall", "water"}},
        {"river", {"river", "water"}},
        {"lake", {"lake", "water"}},
        {"coast", {"coast", "water"}},
        {"shore", {"shore", "water"}},
        {"reef", {"reef", "water"}},

        // Urban
        {"town", {"town", "settlement"}},
        {"village", {"village", "settlement"}},
        {"city", {"city", "urban"}},
        {"market", {"market", "urban"}},
        {"gate", {"gate", "urban"}},
        {"tower", {"tower", "building"}},
        {"castle", {"castle", "fortification"}},
        {"palace", {"palace", "urban"}},

        // Dungeons / ruins
        {"crypt", {"crypt", "dungeon"}},
        {"catacombs", {"catacombs", "dungeon"}},
        {"ruins", {"ruins"}},
        {"sewer", {"sewer", "underground"}},
        {"mine", {"mine", "underground"}},

        // Thematic
        {"lava", {"lava", "hazard"}},
        {"infernal", {"hell", "infernal"}},
        {"hell", {"hell", "infernal"}},
        {"haunted", {"haunted"}},
        {"spooky", {"haunted"}},

        // Locations / modules
        {"barovia", {"barovia", "curseofstrahd"}},
        {"ravenloft", {"ravenloft", "curseofstrahd"}},
        {"phandalin", {"phandalin", "lmop"}},
        {"cragmaw", {"cragmaw", "lmop"}},
        {"skullport", {"skullport", "underdark"}},
        {"velkynvelve", {"velkynvelve", "underdark"}},
        {"darklake", {"darklake", "underdark"}},

        // Misc gameplay locations
        {"arena", {"arena"}},
        {"camp", {"camp"}},
        {"bridge", {"bridge"}},
        {"airship", {"airship", "vehicle"}},
        {"ship", {"ship", "vehicle"}},
        {"nautiloid", {"nautiloid", "spelljammer"}}
    };

    // ------------------ TIME TAGS ------------------
    const QHash<QString, QString> timeTags {
        {"day", "day"},
        {"light", "day"},
        {"night", "night"},
        {"dark", "night"},
        {"dawn", "dawn"},
        {"dusk", "dusk"},
        {"torchlight", "torchlight"}
    };

    // ------------------ GRID DETECTION ------------------
    const QHash<QString, QString> gridWords {
        {"grid", "grid"},
        {"gridded", "grid"},
        {"nogrid", "no-grid"},
        {"no_grid", "no-grid"},
        {"gridless", "no-grid"},
        {"40", "40-grid"},
        {"50", "50-grid"}
    };

    // ------------------ EXTENSION DETECTION ------------------
    const QHash<QString, QString> extensionTags {
        {"jpg",  "image"},
        {"jpeg", "image"},
        {"png",  "image"},
        {"gif",  "animated"},
        {"mp4",  "video"},
        {"m4v",  "video"},
        {"webm", "video"},
        {"pdf",  "document"},
        {"docx", "document"},
        {"url",  "document"}
    };

    // ------------------ CREATOR PREFIXES ------------------
    const QList<QPair<QString, QString>> creators {
        {"ADMaps", "ADMaps"},
        {"ChibbinGrove", "ChibbinGrove"},
        {"Czepeku", "Czepeku"},
        {"Domille", "Domille"},
        {"EightfoldPaper", "EightfoldPaper"},
        {"Elsewhere", "ElsewhereCompany"},
        {"FableKit", "FableKit"},
        {"Fantasy Atlas", "FantasyAtlas"},
        {"GoAdventure", "GoAdventure"},
        {"Loke", "LokeBattleMats"},
        {"MapGuffin", "MapGuffin"},
        {"Seafoot", "SeafootGames"},
        {"Sirinkman", "Sirinkman"},
        {"Spellarena", "Spellarena"},
        {"Tehox", "TehoxMaps"},
        {"Tom Cartos", "TomCartos"}
    };

    class TagFilterProxyModel : public QSortFilterProxyModel
    {
    public:
        QStringList getRequiredTags();
        void setRequiredTags(const QStringList& tags);
        void clearRequiredTags();

    protected:
        virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
        bool listContainsAllTags(const QStringList& list) const;

    private:
        QStringList _requiredTags;
    };

};

Q_DECLARE_METATYPE(MapManagerDialog::MapFileMetadata);

#endif // MAPMANAGERDIALOG_H
