#ifndef BATTLEFRAMEMAPDRAWER_H
#define BATTLEFRAMEMAPDRAWER_H

#include <QObject>
#include <QCursor>

class UndoPath;
class Map;

class BattleFrameMapDrawer : public QObject
{
    Q_OBJECT
public:
    explicit BattleFrameMapDrawer(QObject *parent = nullptr);

    void setMap(Map* map, QPixmap* fow);
    const QCursor& getCursor() const;

signals:
    void fowChanged(const QPixmap& fow);

public slots:
    void handleMouseDown(const QPointF& pos);
    void handleMouseMoved(const QPointF& pos);
    void handleMouseUp(const QPointF& pos);

    void setSize(int size);
    void setScale(int gridScale, int viewScale);
    void resetFoW();
    void clearFoW();
    void setErase(bool erase);
    void setSmooth(bool smooth);
    void setBrushMode(int brushMode);

private:

    void endPath();
    void createCursor();

    bool _mouseDown;
    QPointF _mouseDownPos;
    UndoPath* _undoPath;
    Map* _map;
    QPixmap* _fow;
    QCursor _cursor;

    int _gridScale;
    int _viewScale;
    int _size;
    bool _erase;
    bool _smooth;
    int _brushMode;
};

#endif // BATTLEFRAMEMAPDRAWER_H
