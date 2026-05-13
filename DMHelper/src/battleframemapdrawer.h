#ifndef BATTLEFRAMEMAPDRAWER_H
#define BATTLEFRAMEMAPDRAWER_H

#include <QObject>
#include <QCursor>
#include <QPolygon>

class UndoFowPath;
class LayerScene;

class BattleFrameMapDrawer : public QObject
{
    Q_OBJECT
public:
    explicit BattleFrameMapDrawer(QObject *parent = nullptr);

    void setScene(LayerScene* scene);
    LayerScene* getScene() const;
    const QCursor& getCursor() const;

signals:
    void dirty();
    void cursorChanged(const QCursor& cursor);
    void polygonChanged(const QPolygonF& polygon);
    void polygonCancelled();
    void selectRectChanged(const QRectF& rect);
    void selectRectCancelled();

public slots:
    void handleMouseDown(const QPointF& pos, const Qt::MouseButtons buttons, const Qt::KeyboardModifiers modifiers);
    void handleMouseMoved(const QPointF& pos, const Qt::MouseButtons buttons, const Qt::KeyboardModifiers modifiers);
    void handleMouseUp(const QPointF& pos, const Qt::MouseButtons buttons, const Qt::KeyboardModifiers modifiers);

    void drawRect(const QRect& rect);

    void setSize(int size);
    void setScale(int gridScale, qreal zoomScale);
    void fillFoW();
    void resetFoW();
    void clearFoW();
    void setErase(bool erase);
    void setSmooth(bool smooth);
    void setBrushMode(int brushMode);
    void cancelPolygon();
    void cancelSelect();

private:

    void endPath();
    void applyPolygon();
    void createCursor();

    bool _mouseDown;
    QPointF _mouseDownPos;
    UndoFowPath* _undoPath;
    QPolygon _polygonPoints;
    bool _selectActive;
    LayerScene* _scene;
    QCursor _cursor;

    int _gridScale;
    qreal _zoomScale;
    int _size;
    bool _erase;
    bool _smooth;
    int _brushMode;
};

#endif // BATTLEFRAMEMAPDRAWER_H
