#ifndef BATTLEDIALOGMODELEFFECTFACTORY_H
#define BATTLEDIALOGMODELEFFECTFACTORY_H

#include <QObject>

class BattleDialogModelEffect;
class QDomElement;

class BattleDialogModelEffectFactory : public QObject
{
    Q_OBJECT
public:
    explicit BattleDialogModelEffectFactory(QObject *parent = nullptr);

    static BattleDialogModelEffect* createEffect(int effectType);
    static BattleDialogModelEffect* createEffect(const QDomElement& element, bool isImport);

    static BattleDialogModelEffect* createEffectObjectVideo(const QPointF& position, const QSize& size, const QColor& color, const QString& videoFile);
    static BattleDialogModelEffect* createEffectObject(const QPointF& position, const QSize& size, const QColor& color, const QString& imageFile);
    static BattleDialogModelEffect* createEffectRadius(const QPointF& position, int radius, const QColor& color);
    static BattleDialogModelEffect* createEffectCone(const QPointF& position, int size, const QColor& color);
    static BattleDialogModelEffect* createEffectCube(const QPointF& position, int size, const QColor& color);
    static BattleDialogModelEffect* createEffectLine(const QPointF& position, int length, int width, const QColor& color);

    static BattleDialogModelEffect* createEffectShape(int effectType, const QPointF& position, int size, const QColor& color);

signals:

public slots:
};

#endif // BATTLEDIALOGMODELEFFECTFACTORY_H
