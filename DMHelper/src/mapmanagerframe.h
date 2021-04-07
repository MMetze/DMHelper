#ifndef MAPMANAGERFRAME_H
#define MAPMANAGERFRAME_H

#include <QFrame>
#include <QList>
#include <QMap>
#include <QDir>
#include <QThread>

namespace Ui {
class MapManagerFrame;
}

class QGraphicsScene;
class QGraphicsPixmapItem;
class MapReaderWorker;

class MapManagerFrame : public QFrame
{
    Q_OBJECT

public:
    explicit MapManagerFrame(QWidget *parent = nullptr);
    ~MapManagerFrame();

     virtual bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;

protected slots:
    void imageAvailable(const QString& filename, const QString& md5, QPixmap pixmap);
    void layoutItems();
    void layoutPreview();
    void editDirectories();
    void editCategories();

private:
    void populateTable();
    void populateCategories();
    void scanDirectory(QDir directory);
    int getColumnCount();

    Ui::MapManagerFrame *ui;
    QGraphicsScene* _scene;

    QList<QDir> _directories;
    QStringList _categories;
    int _layoutColumns;
    int _labelSize;
    int _labelGap;
    QPixmap _previewPixmap;
    QMap<QString, QGraphicsPixmapItem*> _items;

    MapReaderWorker* _worker;
    QThread* _workerThread;
};

class MapReaderWorker : public QObject
{
    Q_OBJECT

public:
    MapReaderWorker();

signals:
    void imageReady(const QString& filename, const QString& md5, QPixmap pixmap);
    void processingStopped();
    void processNext();

public slots:
    void addFilename(const QString& filename);
    void stopProcessing();
    void setPixmapSize(int pixmapSize);

protected slots:
    void processFilename();
    void processingComplete();

protected:
    QStringList _filenames;
    bool _processing;
    bool _stopRequested;
    int _pixmapSize;
};

#endif // MAPMANAGERFRAME_H
