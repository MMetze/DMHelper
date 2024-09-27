#ifndef GLOBALSEARCHFRAME_H
#define GLOBALSEARCHFRAME_H

#include <QFrame>

namespace Ui {
class GlobalSearchFrame;
}

class Campaign;
class QTreeWidgetItem;

class GlobalSearchFrame : public QFrame
{
    Q_OBJECT

public:
    explicit GlobalSearchFrame(QWidget *parent = nullptr);
    ~GlobalSearchFrame();

    void setCampaign(Campaign* campaign);

signals:
    void monsterSelected(const QString& monsterClass);

protected slots:
    void executeSearch();
    void handleItemClicked(QTreeWidgetItem *item, int column);
    void handleItemDoubleClicked(QTreeWidgetItem *item, int column);

private:
    Ui::GlobalSearchFrame *ui;

    Campaign* _campaign;
};

#endif // GLOBALSEARCHFRAME_H
