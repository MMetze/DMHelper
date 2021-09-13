#ifndef MAPMARKERDIALOG_H
#define MAPMARKERDIALOG_H

#include <QDialog>
#include <QUuid>
#include "map.h"

namespace Ui {
class MapMarkerDialog;
}

class MapMarkerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MapMarkerDialog(const QString& title, const QString& description, const QUuid& encounter, Map& map, QWidget *parent = nullptr);
    virtual ~MapMarkerDialog() override;

    QString getTitle() const;
    QString getDescription() const;
    QUuid getEncounter() const;

private:
    Ui::MapMarkerDialog *ui;

    void populateCombo(const QUuid& encounter, Map& map);
    void addChildEntry(CampaignObjectBase* object, int depth, const QUuid& encounter);
    QIcon objectIcon(CampaignObjectBase* object);
};

#endif // MAPMARKERDIALOG_H
