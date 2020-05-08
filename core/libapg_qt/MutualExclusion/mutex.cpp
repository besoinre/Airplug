#include "mutex.h"

#include <QDebug>
#include <QMap>

namespace AirPlug
{

class Q_DECL_HIDDEN Mutex::Private
{
public:

    Private()
        : clock(nullptr)
    {

    }

    ~Private()
    {

    }

public:

    bool isLessPriority(const VectorClock& requesterClock) const;

public:

    QMap<int, std::pair<ACLMessage::Perfomative, TimeStamp> tab;
    int local_site_id;
};

Mutex::Mutex()
    : QObject(nullptr),
      d(std::make_unique<Private>())
{

}

Mutex::~Mutex()
{

}

void Mutex::requestUpdate(TimeStamp message_time_stamp)
{
    tab[message_time_stamp.getSiteID()].first = ACLMessage::MUTEX_REQUEST;
    tab[message_time_stamp.getSiteID()].second = (tab[local_site_id].second.updateTimeStamp(message_time_stamp))++;

    ACLMessage ack_message(ACLMessage::MUTEX_ACKNOWLEDGE);
    ack_message.setTimeStamp(tab[local_site_id].second);

    emit signalResponse(ack_message);

    if(tab[local_site_id].first == ACLMessage::MUTEX_REQUEST)
    {
        for(auto key : tab.keys())
        {
            if(!(tab[local_site_id].second < tab[key].second]))
                return;
        }
    }

    //message entrer en section critique a faire
    emit accessAllowed();
}

void Mutex::liberationUpdate(TimeStamp message_time_stamp)
{
    tab[message_time_stamp.getSiteID()].first = ACLMessage::MUTEX_LIBERATION;
    tab[message_time_stamp.getSiteID()].second = (tab[local_site_id].second.updateTimeStamp(message_time_stamp))++;

    if(tab[local_site_id].first == ACLMessage::MUTEX_REQUEST)
    {
        for(auto key : tab.keys())
        {
            if(!(tab[local_site_id].second < tab[key].second]))
                return;
        }
    }

    emit accessAllowed();
}

void Mutex::acknowledgeUpdate(TimeStamp message_time_stamp)
{
    tab[local_site_id].second.updateTimeStamp(message_time_stamp)++;

    if(!(tab[message_time_stamp.getSiteID()].first == ACLMessage::MUTEX_REQUEST))
    {
        tab[message_time_stamp.getSiteID()].first = ACLMessage::MUTEX_ACKNOWLEDGE;
        tab[message_time_stamp.getSiteID()].second = message_time_stamp.getTimeStamp()++;
    }

    if(tab[local_site_id].first == ACLMessage::MUTEX_REQUEST)
    {
          for(auto key : tab.keys())
          {
              if(!(tab[local_site_id].second < tab[key].second]))
                  return;
          }
    }

    emit accessAllowed();
}

}
