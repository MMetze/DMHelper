#ifndef BATTLEFRAMEMAPDRAWER_H
#define BATTLEFRAMEMAPDRAWER_H

#include <QObject>
#include <QCursor>

class UndoFowPath;
class Map;

class BattleFrameMapDrawer : public QObject
{
    Q_OBJECT
public:
    explicit BattleFrameMapDrawer(QObject *parent = nullptr);

    void setMap(Map* map, QPixmap* fow, QImage* glFow);
    Map* getMap() const;
    const QCursor& getCursor() const;

signals:
    void fowEdited(const QPixmap& fow);
    void fowChanged(const QImage& glFow);
    void cursorChanged(const QCursor& cursor);

public slots:
    void handleMouseDown(const QPointF& pos);
    void handleMouseMoved(const QPointF& pos);
    void handleMouseUp(const QPointF& pos);

    void drawRect(const QRect& rect);

    void setSize(int size);
    void setScale(int gridScale, int viewWidth);
    void fillFoW();
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
    UndoFowPath* _undoPath;
    Map* _map;
    QPixmap* _fow;
    QImage* _glFow;
    QCursor _cursor;

    int _gridScale;
    int _viewWidth;
    int _size;
    bool _erase;
    bool _smooth;
    int _brushMode;
};

#endif // BATTLEFRAMEMAPDRAWER_H
