#ifndef GLOBALSEARCHFRAME_H
#define GLOBALSEARCHFRAME_H

#include <QFrame>
#include <QUuid>

namespace Ui {
class GlobalSearchFrame;
}

class Campaign;
class CampaignObjectBase;
class QTreeWidgetItem;

class GlobalSearchFrame : public QFrame
{
    Q_OBJECT

public:
    explicit GlobalSearchFrame(QWidget *parent = nullptr);
    ~GlobalSearchFrame();

    void setCampaign(Campaign* campaign);

signals:
    void campaignObjectSelected(const QUuid& itemId);
    void monsterSelected(const QString& monsterClass);
    void spellSelected(const QString& spellName);

protected slots:
    void executeSearch();
    void handleItemClicked(QTreeWidgetItem *item, int column);
    void handleItemDoubleClicked(QTreeWidgetItem *item, int column);

    QTreeWidgetItem* searchCampaignObject(CampaignObjectBase* object, const QString& searchString);

private:
    Ui::GlobalSearchFrame *ui;

    Campaign* _campaign;
};

#endif // GLOBALSEARCHFRAME_H
