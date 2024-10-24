#ifndef CUSTOMTABLEFRAME_H
#define CUSTOMTABLEFRAME_H

#include <QFrame>
#include <QMap>
#include "customtable.h"

class QListWidgetItem;

namespace Ui {
class CustomTableFrame;
}

class CustomTableFrame : public QFrame
{
    Q_OBJECT

public:
    explicit CustomTableFrame(const QString& tableDirectory, QWidget *parent = nullptr);
    ~CustomTableFrame();

    virtual QSize sizeHint() const;

public slots:
    void setTableDirectory(const QString& tableDir);

protected:
    virtual void showEvent(QShowEvent *event);
    virtual void timerEvent(QTimerEvent *event);

protected slots:
    void tableItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void selectItem();

private:
    void readXMLFile(const QString& fileName);
    QString getEntryText(const QString& tableName);

    Ui::CustomTableFrame *ui;

    QString _tableDirectory;
    int _timerId;
    int _index;
    bool _readTriggered;
    QStringList _directoryList;
    QMap<QString, CustomTable> _tableList;
    QStringList _usedTables;
};

#endif // CUSTOMTABLEFRAME_H
