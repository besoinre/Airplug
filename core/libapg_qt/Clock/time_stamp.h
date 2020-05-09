#ifndef TIME_STAMP_H
#define TIME_STAMP_H

//std includes
#include <memory>

//qt includes
#include <QString>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

namespace AirPlug
{

class TimeStamp
{
public:

    TimeStamp();
    TimeStamp(const int& siteID);
    TimeStamp(const int& siteID, const int& time);
    TimeStamp(const QJsonObject& json);
    TimeStamp(const TimeStamp& other);

    ~TimeStamp();

public:

    TimeStamp operator= (const TimeStamp& other);
    TimeStamp operator++ (int);
    TimeStamp updateTimeStamp(const TimeStamp& other);
    bool operator< (const TimeStamp& other);

public:

    QJsonObject convertToJson(void);
    int getSiteID(void);
    int getTime(void);

private:

    class Private;
    std::unique_ptr<Private> d;
};

}
#endif // TIME_STAMP_H
