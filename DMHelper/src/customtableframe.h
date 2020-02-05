#ifndef CUSTOMTABLEFRAME_H
#define CUSTOMTABLEFRAME_H

#include <QFrame>
#include <QMap>

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

protected:
    virtual void showEvent(QShowEvent *event);
    virtual void timerEvent(QTimerEvent *event);

protected slots:
    void tableItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void selectItem();

private:

    void readXMLFile(const QString& fileName);

    Ui::CustomTableFrame *ui;

    QString _tableDirectory;
    int _timerId;
    int _index;
    bool _readTriggered;
    QStringList _directoryList;
    QMap<QString, QStringList> _tableList;
};

#endif // CUSTOMTABLEFRAME_H
