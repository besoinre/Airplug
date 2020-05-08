#ifndef MUTEX_H
#define MUTEX_H

//Qt include
#include <QObject>

//std includes
#include <memory>

// libapg include
#include "time_stamp.h"
#include "aclmessage.h"

namespace AirPlug
{

class Mutex : public QObject
{
    Q_OBJECT
public:

    Mutex();
    ~Mutex();

public:

    void trylock(const TimeStamp& requester_stamp);

    void lock();

    void unlock();

    void restart();

    void receiveExternalRequest(const TimeStamp& requester_stamp);

    QJsonArray getPendingQueue() const;

    void requestUpdate(TimeStamp message_time_stamp);
    void acknowlegdeUpdatet(TimeStamp message_time_stamp);
    void liberationUpdate(TimeStamp message_time_stamp);

public:

    Q_SIGNAL void signalResponse(const ACLMessage& message);
    Q_SIGNAL void accessAllowed(void);

private:

    class Private;
    std::unique_ptr<Private> d;
};


}
#endif // MUTEX_H
