#include "mutex.h"

//std includes
#include <utility>

//qt includes
#include <QDebug>
#include <QMap>
#include <QPair>
#include <QCoreApplication>

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

    QMap<int, QPair<ACLMessage::Performative, TimeStamp>> tab;
    int local_site_id;
};

Mutex::Mutex()
    : QObject(nullptr),
      d(std::make_unique<Private>())
{
    d->local_site_id = QCoreApplication::applicationPid();
    QPair<ACLMessage::Performative, TimeStamp> p(ACLMessage::MUTEX_LIBERATION, TimeStamp(d->local_site_id));
    d->tab[d->local_site_id] = p;
}

Mutex::~Mutex()
{

}

void Mutex::addPlayerConnection(int site_id)
{
    QPair<ACLMessage::Performative, TimeStamp> p(ACLMessage::MUTEX_LIBERATION, TimeStamp(site_id));
    d->tab[site_id] = p;
}

void Mutex::lock(void)
{
    d->tab[d->local_site_id].first = ACLMessage::MUTEX_REQUEST;
    d->tab[d->local_site_id].second++;

    ACLMessage req_message(ACLMessage::MUTEX_REQUEST);
    req_message.setTimeStamp(d->tab[d->local_site_id].second);
    emit signalResponse(req_message);
}

void Mutex::unlock(void)
{
    d->tab[d->local_site_id].first = ACLMessage::MUTEX_LIBERATION;
    d->tab[d->local_site_id].second++;

    ACLMessage lib_message(ACLMessage::MUTEX_LIBERATION);
    lib_message.setTimeStamp(d->tab[d->local_site_id].second);
    emit signalResponse(lib_message);
}

void Mutex::requestUpdate(TimeStamp message_time_stamp)
{
    d->tab[message_time_stamp.getSiteID()].first = ACLMessage::MUTEX_REQUEST;
    d->tab[message_time_stamp.getSiteID()].second = (d->tab[d->local_site_id].second.updateTimeStamp(message_time_stamp))++;

    ACLMessage ack_message(ACLMessage::MUTEX_ACKNOWLEDGE);
    ack_message.setTimeStamp(d->tab[d->local_site_id].second);
    emit signalResponse(ack_message);


    if(d->tab[d->local_site_id].first == ACLMessage::MUTEX_REQUEST)
    {
        for(auto key : d->tab.keys())
        {
            if(key != d->local_site_id && !(d->tab[d->local_site_id].second < d->tab[key].second))
                return;
        }
    }

    emit accessAllowed();
}

void Mutex::liberationUpdate(TimeStamp message_time_stamp)
{
    d->tab[message_time_stamp.getSiteID()].first = ACLMessage::MUTEX_LIBERATION;
    d->tab[message_time_stamp.getSiteID()].second = (d->tab[d->local_site_id].second.updateTimeStamp(message_time_stamp))++;

    if(d->tab[d->local_site_id].first == ACLMessage::MUTEX_REQUEST)
    {
        for(auto key : d->tab.keys())
        {
            if(key != d->local_site_id && !(d->tab[d->local_site_id].second < d->tab[key].second))
                return;
        }
    }

    emit accessAllowed();
}

void Mutex::acknowledgeUpdate(TimeStamp message_time_stamp)
{
    d->tab[d->local_site_id].second.updateTimeStamp(message_time_stamp)++;

    if(!(d->tab[message_time_stamp.getSiteID()].first == ACLMessage::MUTEX_REQUEST))
    {
        d->tab[message_time_stamp.getSiteID()].first = ACLMessage::MUTEX_ACKNOWLEDGE;
        d->tab[message_time_stamp.getSiteID()].second = message_time_stamp++;
    }

    if(d->tab[d->local_site_id].first == ACLMessage::MUTEX_REQUEST)
    {
          for(auto key : d->tab.keys())
          {
              if(key != d->local_site_id && !(d->tab[d->local_site_id].second < d->tab[key].second))
                  return;
          }
    }

    emit accessAllowed();
}

}
