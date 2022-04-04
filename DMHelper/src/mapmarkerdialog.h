#ifndef MAPMARKERDIALOG_H
#define MAPMARKERDIALOG_H

#include <QDialog>
#include <QUuid>
#include <QMenu>
#include "map.h"

namespace Ui {
class MapMarkerDialog;
}

class MapMarkerDialog : public QDialog
{
    Q_OBJECT

public:
    static const int MAPMARKERDIALOG_DELETE = -1;

    explicit MapMarkerDialog(const MapMarker& marker, Map& map, QWidget *parent = nullptr);
    virtual ~MapMarkerDialog() override;


    MapMarker getMarker();
    /*
    QString getTitle() const;
    QString getDescription() const;
    QUuid getEncounter() const;
    QColor getColor() const;
    QString getIconFile() const;
    int getIconScale() const;
    bool isColoredIcon() const;
    int getScale() const;
    */

protected:
    virtual void showEvent(QShowEvent *event) override;

private slots:
    void deleteMarker();
    void iconSelected(const QString& iconFile);
    void selectNewIcon(bool checked);
    void applyColorChecked(bool checked);

private:
    Ui::MapMarkerDialog *ui;

    bool setColoredIcon(const QString& iconFile);
    bool populateIconList();
    void populateCombo(const QUuid& encounter, Map& map);
    void addChildEntry(CampaignObjectBase* object, int depth, const QUuid& encounter);
    QIcon objectIcon(CampaignObjectBase* object);

    QMenu* _menu;
    MapMarker _marker;
    QString _currentIcon;
    QStringList _icons;
    int _iconDim;
};

class MapMarkerDialog_IconAction : public QAction
{
    Q_OBJECT

public:
    explicit MapMarkerDialog_IconAction(const QString& iconFile, const QString& iconText = QString(), QObject *parent = nullptr);

    const QString& getIconFile() const;

signals:
    void iconFileSelected(const QString& iconFile);

protected slots:
    void handleActivated(bool checked);

protected:
    QString _iconFile;

};


#endif // MAPMARKERDIALOG_H
