#include "complicationdata.h"

ComplicationData::ComplicationData(QString title, QString description, QString check, QString failure, QString damage, QString obstacle, int length, bool sticky ) :
    _title(title),
    _description(description),
    _check(check),
    _failure(failure),
    _damage(damage),
    _obstacle(obstacle),
    _length(length),
    _sticky(sticky)
{}

ComplicationData::ComplicationData(const ComplicationData& other) :
    _title(other._title),
    _description(other._description),
    _check(other._check),
    _failure(other._failure),
    _damage(other._damage),
    _obstacle(other._obstacle),
    _length(other._length),
    _sticky(other._sticky)
{}

ComplicationData::~ComplicationData()
{
}

ComplicationData& ComplicationData::operator=(const ComplicationData& other)
{
    _title = other._title;
    _description = other._description;
    _check = other._check;
    _failure = other._failure;
    _damage = other._damage;
    _obstacle = other._obstacle;
    _length = other._length;
    _sticky = other._sticky;
    return *this;
}
