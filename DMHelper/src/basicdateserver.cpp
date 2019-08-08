#include "basicdateserver.h"
#include <QFile>
#include <QTextStream>
#include <QDomDocument>
#include <QDomElement>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>

BasicDateServer* BasicDateServer::_instance = nullptr;

BasicDateServer::BasicDateServer() :
    _calendars(),
    _activeIndex(-1)
{
    readDateInformation();
}

BasicDateServer* BasicDateServer::Instance()
{
    if(!_instance)
        Initialize();

    return _instance;
}

void BasicDateServer::Initialize()
{
    if(_instance)
        return;

    qDebug() << "[BasicDateServer] Initializing BasicDateServer";
    _instance = new BasicDateServer();
}

void BasicDateServer::Shutdown()
{
    delete _instance;
    _instance = nullptr;
}

void BasicDateServer::setActiveCalendar(const QString& calendarName)
{
    if(_calendars.isEmpty())
    {
        qDebug() << "[BasicDateServer] ERROR: no calendars have been loaded! It is not possible to set an active calendar.";
        return;
    }

    for(int i = 0; i < _calendars.count(); ++i)
    {
        if(_calendars.at(i)._name == calendarName)
        {
            if(_activeIndex == i)
            {
                qDebug() << "[BasicDateServer] Found calendar " << calendarName << ". Index was already set to " << i;
            }
            else
            {
                qDebug() << "[BasicDateServer] Found calendar " << calendarName << ". Index set to " << i;
                _activeIndex = i;
            }
            return;
        }
    }

    qDebug() << "[BasicDateServer] ERROR: Calendar " << calendarName << " not found! Calendar set to default " << _calendars.at(0)._name;
    _activeIndex = 0;
}

QString BasicDateServer::getActiveCalendarName()
{
    if((_activeIndex < 0) || (_activeIndex >= _calendars.count()))
       return QString();
    else
        return _calendars.at(_activeIndex)._name;
}

int BasicDateServer::getDaysInYear(int year)
{
    int days = 0;

    if((_activeIndex >= 0) && (_activeIndex < _calendars.count()))
    {
        for(int i = 0; i < _calendars.at(_activeIndex)._months.count(); ++i)
        {
            days += getDaysInMonth(i, year);
        }
    }

    return days;
}

int BasicDateServer::getMonthsInYear(int year)
{
    Q_UNUSED(year);
    if((_activeIndex < 0) || (_activeIndex >= _calendars.count()))
        return 0;
    else
        return _calendars.at(_activeIndex)._months.count();
}

int BasicDateServer::getDaysInMonth(int month, int year)
{
    if((_activeIndex < 0) || (_activeIndex >= _calendars.count()))
        return -1;

    if((month < 1) || (month > _calendars.at(_activeIndex)._months.count()))
        return -1;

    int days = _calendars.at(_activeIndex)._months.at(month - 1)._days;
    for(int j = 0; j < _calendars.at(_activeIndex)._months.at(month - 1)._leapYears.count(); ++j)
    {
        if(year % _calendars.at(_activeIndex)._months.at(month - 1)._leapYears.at(j)._period == 0)
            ++days;
    }

    return days;
}

int BasicDateServer::getDaysBeforeMonth(int month, int year)
{
    if((_activeIndex < 0) || (_activeIndex >= _calendars.count()))
        return -1;

    if((month < 1) || (month > _calendars.at(_activeIndex)._months.count()))
        return -1;

    int days = 0;

    for(int i = 0; i < month - 1; ++i)
    {
        days += getDaysInMonth(i, year);
    }

    return days;
}

QString BasicDateServer::getMonthName(int month)
{
    if((_activeIndex < 0) || (_activeIndex >= _calendars.count()))
        return QString();

    if((month < 1) || (month > _calendars.at(_activeIndex)._months.count()))
        return QString();

    return _calendars.at(_activeIndex)._months.at(month - 1)._name;
}

QString BasicDateServer::getMonthAlternativeName(int month)
{
    if((_activeIndex < 0) || (_activeIndex >= _calendars.count()))
        return QString();

    if((month < 1) || (month > _calendars.at(_activeIndex)._months.count()))
        return QString();

    return _calendars.at(_activeIndex)._months.at(month - 1)._alternativeName;
}

QString BasicDateServer::getSpecialDayName(int day, int month)
{
    if((_activeIndex < 0) || (_activeIndex >= _calendars.count()))
        return QString();

    if((month < 1) || (month > _calendars.at(_activeIndex)._months.count()))
        return QString();

    for(int i = 0; i < _calendars.at(_activeIndex)._months.at(month - 1)._specialDays.count(); ++i)
    {
        if(_calendars.at(_activeIndex)._months.at(month - 1)._specialDays.at(i)._day == day)
            return _calendars.at(_activeIndex)._months.at(month - 1)._specialDays.at(i)._name;
    }

    return QString();
}

QString BasicDateServer::getLeapDayName(int day, int month, int year)
{
    if((_activeIndex < 0) || (_activeIndex >= _calendars.count()))
        return QString();

    if((month < 1) || (month > _calendars.at(_activeIndex)._months.count()))
        return QString();

    for(int i = 0; i < _calendars.at(_activeIndex)._months.at(month - 1)._leapYears.count(); ++i)
    {
        if((_calendars.at(_activeIndex)._months.at(month - 1)._leapYears.at(i)._day == day) &&
           (year % _calendars.at(_activeIndex)._months.at(month - 1)._leapYears.at(i)._period == 0))
            return _calendars.at(_activeIndex)._months.at(month - 1)._leapYears.at(i)._name;
    }

    return QString();
}

QStringList BasicDateServer::getCalendarNames() const
{
    QStringList names;

    for(int i = 0; i < _calendars.count(); ++i)
    {
        names.append(_calendars.at(i)._name);
    }

    return names;
}

QStringList BasicDateServer::getMonthNames() const
{
    QStringList names;

    if((_activeIndex >= 0) && (_activeIndex < _calendars.count()))
    {
        for(int i = 0; i < _calendars.at(_activeIndex)._months.count(); ++i)
        {
            names.append(_calendars.at(_activeIndex)._months.at(i)._name);
        }
    }

    return names;
}

QStringList BasicDateServer::getMonthNamesWithAlternatives() const
{
    QStringList names;

    if((_activeIndex >= 0) && (_activeIndex < _calendars.count()))
    {
        for(int i = 0; i < _calendars.at(_activeIndex)._months.count(); ++i)
        {
            QString fullName = _calendars.at(_activeIndex)._months.at(i)._name;
            if(!_calendars.at(_activeIndex)._months.at(i)._alternativeName.isEmpty())
                fullName += " (" + _calendars.at(_activeIndex)._months.at(i)._alternativeName + ")";
            names.append(fullName);
        }
    }

    return names;
}

void BasicDateServer::readDateInformation()
{
    qDebug() << "[BasicDateServer] Reading calendar information";

#ifdef Q_OS_MAC
    QDir fileDirPath(QCoreApplication::applicationDirPath());
    fileDirPath.cdUp();
    fileDirPath.cdUp();
    fileDirPath.cdUp();
    QString calendarFileName = fileDirPath.path() + QString("/calendar.xml");
#else
    QString calendarFileName("calendar.xml");
#endif

    QDomDocument doc("DMHelperDataXML");
    QFile file(calendarFileName);
    qDebug() << "[BasicDateServer] Calendar file: " << QFileInfo(file).filePath();
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[BasicDateServer] Unable to read calendar file: " << calendarFileName;
        return;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString errMsg;
    int errRow;
    int errColumn;
    bool contentResult = doc.setContent(in.readAll(), &errMsg, &errRow, &errColumn);

    file.close();

    if(contentResult == false)
    {
        qDebug() << "[BasicDateServer] Unable to parse the calendar data file.";
        qDebug() << errMsg << errRow << errColumn;
        return;
    }

    QDomElement root = doc.documentElement();
    if((root.isNull()) || (root.tagName() != "root"))
    {
        qDebug() << "[BasicDateServer] Unable to find the root element in the calendar data file.";
        return;
    }

    QDomElement calendarElement = root.firstChildElement("calendar");
    if(calendarElement.isNull())
    {
        qDebug() << "[BasicDateServer] Unable to find any calendar information elements in the calendar data file.";
        return;
    }

    while(!calendarElement.isNull())
    {
        BasicDateServer_Calendar calendar;

        calendar._name = calendarElement.attribute("name");
        calendar._weekLength = calendarElement.attribute("weeklength", QString::number(7)).toInt();

        QDomElement monthElement = calendarElement.firstChildElement("month");
        while(!monthElement.isNull())
        {
            BasicDateServer_Month month;

            month._name = monthElement.attribute("name");
            month._alternativeName = monthElement.attribute("alternatename");
            month._days = monthElement.attribute("days", QString::number(30)).toInt();

            QDomElement specialElement = monthElement.firstChildElement("specialday");
            while(!specialElement.isNull())
            {
                BasicDateServer_Day specialDay;

                specialDay._name = specialElement.attribute("name");
                specialDay._day = specialElement.attribute("day", QString::number(1)).toInt();
                month._specialDays.append(specialDay);

                specialElement = specialElement.nextSiblingElement("specialday");
            }

            QDomElement leapElement = monthElement.firstChildElement("leapyear");
            while(!leapElement.isNull())
            {
                BasicDateServer_LeapYear leapYear;

                leapYear._name = leapElement.attribute("name");
                leapYear._day = leapElement.attribute("day", QString::number(1)).toInt();
                leapYear._period = leapElement.attribute("period", QString::number(4)).toInt();
                month._leapYears.append(leapYear);

                leapElement = leapElement.nextSiblingElement("leapyear");
            }

            calendar._daysInYear += month._days;
            calendar._months.append(month);

            monthElement = monthElement.nextSiblingElement(QString("month"));
        }

        _calendars.append(calendar);
        calendarElement = calendarElement.nextSiblingElement(QString("calendar"));
    }

    _activeIndex = 0;

    qDebug() << "[BasicDateServer] Completed reading calendars";
}
