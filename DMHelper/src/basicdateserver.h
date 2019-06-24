#ifndef BASICDATESERVER_H
#define BASICDATESERVER_H

#include <QString>
#include <QList>

class BasicDateServer;

class BasicDateServer
{
public:
    BasicDateServer();

    static BasicDateServer* Instance();
    static void Initialize();
    static void Shutdown();

    void setActiveCalendar(const QString& calendarName);
    QString getActiveCalendarName();

    int getDaysInYear(int year);
    int getMonthsInYear(int year);
    int getDaysInMonth(int month, int year);
    int getDaysBeforeMonth(int month, int year);

    QString getMonthName(int month);
    QString getMonthAlternativeName(int month);
    QString getSpecialDayName(int day, int month);
    QString getLeapDayName(int day, int month, int year);

    QStringList getCalendarNames() const;
    QStringList getMonthNames() const;
    QStringList getMonthNamesWithAlternatives() const;

    class BasicDateServer_Calendar;
    class BasicDateServer_Month;
    class BasicDateServer_Day;
    class BasicDateServer_LeapYear;

private:
    void readDateInformation();

    static BasicDateServer* _instance;
    QList<BasicDateServer::BasicDateServer_Calendar> _calendars;
    int _activeIndex;

public:
    class BasicDateServer_Calendar
    {
    public:
        BasicDateServer_Calendar() :
            _daysInYear(0),
            _months(),
            _name(),
            _weekLength(0)
        {}

        int _daysInYear;
        QList<BasicDateServer::BasicDateServer_Month> _months;
        QString _name;
        int _weekLength;
    };

    class BasicDateServer_Month
    {
    public:
        BasicDateServer_Month() :
            _name(),
            _alternativeName(),
            _days(0),
            _specialDays(),
            _leapYears()
        {}

        QString _name;
        QString _alternativeName;
        int _days;

        QList<BasicDateServer::BasicDateServer_Day> _specialDays;
        QList<BasicDateServer::BasicDateServer_LeapYear> _leapYears;
    };

    class BasicDateServer_Day
    {
    public:
        BasicDateServer_Day() :
            _name(),
            _day(0)
        {}

        QString _name;
        int _day;
    };

    class BasicDateServer_LeapYear
    {
    public:
        BasicDateServer_LeapYear() :
            _name(),
            _day(0),
            _period(0)
        {}

        QString _name;
        int _day;
        int _period;
    };
};

#endif // BASICDATESERVER_H
