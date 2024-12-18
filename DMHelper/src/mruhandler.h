#ifndef MRUHANDLER_H
#define MRUHANDLER_H

#include <QObject>
#include <QStringList>
#include <QSettings>

class QMenu;

class MRUHandler : public QObject
{
    Q_OBJECT
public:
    // Note: supports up to 99 MRU files
    explicit MRUHandler(QMenu* actionsMenu, int mruCount, QObject *parent = nullptr);

    void addMRUFile(const QString &mruFile);

    int getMRUCount() const;
    void setMRUCount(int mruCount);

    QStringList getMRUList() const;
    void setMRUList(const QStringList& mruList);

    QMenu* getActionsMenu() const;
    void setActionsMenu(QMenu* actionsMenu);

    void readMRUFromSettings(QSettings& settings);
    void writeMRUToSettings(QSettings& settings);

signals:
    void triggerMRU(const QString& mruFile);
    void mruListChanged();

public slots:

protected slots:
    void updateMRUActions();

protected:
    void internalSetMRUCount(int mruCount);
    void internalAddMRUFile(const QString &mruFile);

    int _mruCount;
    QStringList _mruFiles;
    QMenu* _actionsMenu;

};

#endif // MRUHANDLER_H
