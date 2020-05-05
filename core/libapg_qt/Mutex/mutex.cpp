#include "mutex.h"

namespace AirPlug
{

class Q_DECL_HIDDEN Mutex::Private
{
public:

    Private()
    {
    }

    ~Private()
    {
    }

public:

    // because it's more difficult to keep track of site's order
    QHash<QString, MutexData> mutexTab;
    QString             siteID;
    int date;
};

Mutex::Mutex(const QString& siteID)
    : d(new Private())
{
    d->siteID = siteID;
    d->mutexTab.insert(siteID, {release,0});
}

Mutex::Mutex(const QString& siteID, const QString* otherSiteID)
    : d(new Private())
{
    d->siteID = siteID;
    for(int i=0; i<otherSiteID->size(); i++){
        d->mutexTab.insert(otherSiteID[i],{release,0});
    }
    d->mutexTab.detach();
}

void Mutex::addMutexData(const QString& siteID, const MutexData data) {
  d->mutexTab[siteID] = data;
}

Mutex::~Mutex()
{
    delete d;
}


}


//VectorClock::VectorClock(const QJsonObject& json)
//    : d(new Private())
//{
//    d->siteID = json[QLatin1String("siteID")].toString();

//    QJsonArray clock = json[QLatin1String("clock")].toArray();

//    for (int i = 0; i < clock.size(); ++i)
//    {
//        QJsonObject object = clock[i].toObject();

//        d->localClock.insert(object.constBegin().key(), object.constBegin().value().toInt());
//    }
//}

//VectorClock::VectorClock(const VectorClock& other)
//    : d(new Private())
//{
//    d->siteID     = other.d->siteID;

//    d->localClock = other.d->localClock;
//    d->localClock.detach();
//}


//VectorClock VectorClock::operator= (const VectorClock& other)
//{
//    d->siteID     = other.d->siteID;

//    d->localClock = other.d->localClock;
//    d->localClock.detach();
//}

//VectorClock VectorClock::operator++ ()
//{
//    ++(d->localClock[d->siteID]);

//    return VectorClock(*this);
//}

//VectorClock VectorClock::operator++ (int)
//{
//    VectorClock clock = VectorClock(*this);

//    ++(d->localClock[d->siteID]);

//    return clock;
//}

//void VectorClock::updateClock(const VectorClock& other)
//{
//    // increment local clock
//    ++(d->localClock[d->siteID]);

//    // update other clocks
//    for (QHash<QString, int>::const_iterator iter  = other.d->localClock.cbegin();
//                                             iter != other.d->localClock.cend();
//                                             ++iter)
//    {
//        if (d->siteID != iter.key())
//        {
//            if ( (! d->localClock.contains(iter.key()))      ||
//                 (d->localClock[iter.key()] < iter.value()) )
//            {
//                d->localClock[iter.key()] = iter.value();
//            }
//        }
//    }
//}

//bool VectorClock::operator < (const VectorClock& other)
//{
//    for (QHash<QString, int>::const_iterator iter  = d->localClock.cbegin();
//                                             iter != d->localClock.cend();
//                                             ++iter)
//    {
//        if ( (! other.d->localClock.contains(iter.key()) && iter.value() != 0) ||
//             (iter.value() > other.d->localClock[iter.key()]) )
//        {
//            return false;
//        }
//    }

//    return true;
//}

//QJsonObject VectorClock::convertToJson() const
//{
//    QJsonObject json;

//    json[QLatin1String("siteID")] = d->siteID;

//    QJsonArray clock;

//    for (QHash<QString, int>::const_iterator iter  = d->localClock.cbegin();
//                                             iter != d->localClock.cend();
//                                             ++iter)
//    {
//            QJsonObject localClock;
//            localClock[iter.key()] = iter.value();

//            clock.append(localClock);
//    }

//    json["clock"] = clock;

//    return json;
//}

//QString VectorClock::getSiteID() const
//{
//    return d->siteID;
//}

//int VectorClock::getValue(const QString& siteID) const
//{
//    if (d->localClock.contains(siteID))
//    {
//        return d->localClock[siteID];
//    }

//    return 0;
//}

//}
