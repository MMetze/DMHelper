#ifndef CAMERASCENE_H
#define CAMERASCENE_H

#include <QGraphicsScene>

class CameraScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit CameraScene(QObject *parent = nullptr);
    virtual ~CameraScene();

    void handleItemChanged(QGraphicsItem* item);

signals:
    void itemChanged(QGraphicsItem* item);

};

#endif // CAMERASCENE_H
