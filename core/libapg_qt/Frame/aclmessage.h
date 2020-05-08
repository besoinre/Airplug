/* ============================================================
 *
 * This file is a part of Airplug project
 *
 * Date        : 2020-4-25
 * Description : simplify version of FIPA ACL Message in Multi-agent system
 *
 * 2020 by Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * ============================================================ */

#ifndef ACLMESSAGE_H
#define ACLMESSAGE_H

//std includes
#include <memory>

// libapg include
#include "message.h"
#include "time_stamp.h"

namespace AirPlug
{

/**
 * @brief The ACLMessage class : organize AirPlug::Message with the structure of FIPA ACL Message
 * The general form of the message will be as example below:
 * {
 *  "performative" : QString,
 *  "timestamp"    : VectorClock
 *  "content"      : JsonObject;
 * }
 */

class ACLMessage : public Message
{
public:

    enum Performative
    {
        // standard FIPA ACL Message Perfomative
        REQUEST = 0,
        INFORM,
        QUERY_IF,
        REFUSE,
        CONFIRM,
        UNKNOWN,
        PING,
        PONG,

        // custom Distributed perfomative
        REQUEST_SNAPSHOT,
        INFORM_STATE,
        PREPOST_MESSAGE,
        SNAPSHOT_RECOVER,
        READY_SNAPSHOT,
        MUTEX_REQUEST,
        MUTEX_ACKNOWLEDGE,
        MUTEX_LIBERATION,
        HANDSHAKE_SYN,
        HANDSHAKE_ACK
    };

public:

    ACLMessage(Performative performative);
    ACLMessage(const QString& message);

    ~ACLMessage();

public:

    void setPerformative(Performative performative);
    void setContent(QJsonObject& content);
    void setTimeStamp(TimeStamp& time_stamp);

    // siteID of sender's NET
    void setSender(int& siteID);

    Performative getPerformative();
    std::shared_ptr<TimeStamp> getTimeStamp();
    QJsonObject  getContent();
    int getSender();

    QJsonObject toJsonObject();
};

}
#endif // ACLMESSAGE_H
