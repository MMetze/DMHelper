#ifndef BASICDATE_H
#define BASICDATE_H

#include <QString>

class BasicDate
{
public:
    BasicDate(int d, int m, int y);
    BasicDate(const QString &ddmmyyyy);
    BasicDate(const BasicDate &d);
    virtual ~BasicDate();

    virtual bool isValid() const;

    virtual void addDay();
    virtual void addMonths(int nmonths);
    virtual void addYears(int nyears);

    virtual int day() const;
    virtual int month() const;
    virtual int year() const;

    virtual int daysInMonth() const;
    virtual int daysUntilMonth() const;
    virtual int dayOfYear() const;
    virtual int daysInYear() const;

    virtual void getDate(int *year, int *month, int *day) const;
    virtual void setDate(int year, int month, int day);

    virtual QString toStringDDMMYYYY() const;
    virtual void fromStringDDMMYYYY(const QString &ddmmyyyy);

    virtual BasicDate& operator=(const BasicDate &d);

    virtual bool operator!=(const BasicDate &d) const;
    virtual bool operator<(const BasicDate &d) const;
    virtual bool operator<=(const BasicDate &d) const;
    virtual bool operator==(const BasicDate &d) const;
    virtual bool operator>(const BasicDate &d) const;
    virtual bool operator>=(const BasicDate &d) const;

protected:
    int _day;
    int _month;
    int _year;
};

#endif // BASICDATE_H
