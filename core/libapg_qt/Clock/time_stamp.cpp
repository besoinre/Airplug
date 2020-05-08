#include "time_stamp.h"

#include <QDebug>

namespace AirPlug
{

class Q_DECL_HIDDEN TimeStamp::Private
{
public:

    Private()
    {
    }

    ~Private()
    {
    }

public:

    int time;
    int site_id;
};

TimeStamp::TimeStamp(const int& site_id)
    : d(std::make_unique<Private>())
{
    d->site_id = site_id;
    d->time = 0;
}

TimeStamp::TimeStamp(const int& site_id, const int& time)
    : d(std::make_unique<Private>())
{
    d->site_id = site_id;
    d->time = time;
}

TimeStamp::TimeStamp(const QJsonObject& json)
    : d(std::make_unique<Private>())
{
    d->site_id = json[QString("site_id")].toInt();
    d->time = json[QString("time")].toInt();
}

TimeStamp::TimeStamp(const TimeStamp& other)
    : d(std::make_unique<Private>())
{
    d->site_id = other.d->site_id;
    d->time = other.d->time;
}

TimeStamp::~TimeStamp()
{

}

TimeStamp TimeStamp::operator= (const TimeStamp& other)
{
    d->site_id = other.d->site_id;
    d->time = other.d->time;

    return TimeStamp(*this);
}

TimeStamp TimeStamp::operator++ (int)
{
    d->time++;
    return TimeStamp(*this);
}

void TimeStamp::updateTimeStamp(const TimeStamp& other)
{
    if(d->time < other.d->time)
        d->time = other.d->time;
}

bool TimeStamp::operator< (const TimeStamp& other)
{
    if(d->time != other.d->time)
    {
        if(d->time < other.d->time)
            return true;
        else //if(d->time > other.d->time)
            return false;
    }
    else if(d->site_id < other.d->site_id)
        return true;
    else //if(d->site_id >= other.d->site_id)
        return false;
}

QJsonObject TimeStamp::convertToJson(void)
{
    QJsonObject json;

    json[QString("site_id")] = d->site_id;

    json[QString("time")] = d->time;

    return json;
}

int TimeStamp::getSiteID(void)
{
    return d->site_id;
}

int TimeStamp::getTime(void)
{
    return d->time;
}

}
