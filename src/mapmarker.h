#ifndef MAPMARKER_H
#define MAPMARKER_H

#include <QPoint>
#include <QString>

class MapMarker
{
public:
    MapMarker(QPoint position, QString title, QString description);
    virtual ~MapMarker();

    virtual QPoint position() const;
    virtual QString title() const;
    virtual QString description() const;

private:
    QPoint _position;
    QString _title;
    QString _description;

};

#endif // MAPMARKER_H
