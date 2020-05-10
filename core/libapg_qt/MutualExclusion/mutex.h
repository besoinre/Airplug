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

    void addPlayerConnection(int site_id);

    void lock(void);
    void unlock(void);

    void requestUpdate(TimeStamp message_time_stamp);
    void acknowledgeUpdate(TimeStamp message_time_stamp);
    void liberationUpdate(TimeStamp message_time_stamp);

signals:

    void signalResponse(ACLMessage message);
    void accessAllowed(void);

private:

    class Private;
    std::unique_ptr<Private> d;
};


}
#endif // MUTEX_H
