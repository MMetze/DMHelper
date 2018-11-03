#ifndef COMPLICATIONDATA_H
#define COMPLICATIONDATA_H

#include <QString>

class ComplicationData
{
public:
    ComplicationData(QString title = QString(""),
                     QString description = QString(""),
                     QString check = QString(""),
                     QString failure = QString(""),
                     QString damage = QString(""),
                     QString obstacle = QString(""),
                     int length = 0,
                     bool sticky = true );
    ComplicationData(const ComplicationData& other);
    ~ComplicationData();

    ComplicationData& operator=(const ComplicationData& other);

    QString _title;
    QString _description;
    QString _check;
    QString _failure;
    QString _damage;
    QString _obstacle;
    int _length;
    bool _sticky;
};

#endif // COMPLICATIONDATA_H
