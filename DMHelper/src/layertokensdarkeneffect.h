#ifndef LAYERTOKENSDARKENEFFECT_H
#define LAYERTOKENSDARKENEFFECT_H

#include <QGraphicsEffect>

class LayerTokensDarkenEffect : public QGraphicsEffect
{
public:
    explicit LayerTokensDarkenEffect(QObject *parent = nullptr);

protected:
    QRectF boundingRectFor(const QRectF &sourceRect) const override;
    void draw(QPainter *painter) override;
};

#endif // LAYERTOKENSDARKENEFFECT_H
