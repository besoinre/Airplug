#include "bas_controller.h"

//Qt includes
#include <QCoreApplication>
#include <QTimer>
#include <QMap>
#include <QPair>
#include <QDebug>

//local includes
#include "constants.h"
#include "mutex.h"

using namespace AirPlug;

namespace GameApplication
{

class Q_DECL_HIDDEN BasController::Private
{
public:

    Private()
    {

    }

    ~Private()
    {

    }

public:

    Mutex mutex;

    QTimer handshake_timer;
    bool accepting_new_connections = true;
    QMap<int, ACLMessage::Performative> connection_state;
};



BasController::BasController(QCoreApplication &app, QObject* parent)
    : ApplicationController(QString(), parent),
      d(std::make_unique<Private>())
{
    ApplicationController::init(app);

    //connect mutex signals
    connect(&d->mutex, SIGNAL(signalResponse(ACLMessage)), this, SLOT(fowardMutexMessage(ACLMessage)));
    connect(&d->mutex, SIGNAL(accessAllowed()), this, SLOT(notifyAccessAllowed()));
}

BasController::~BasController()
{

}

void BasController::establishConnections(QString local_player_state)
{
    ACLMessage message(ACLMessage::HANDSHAKE_SYN);
    message.setSender(QCoreApplication::applicationPid());
    message.addContent(QString("state"), local_player_state);
    message.addContent(QString("header_who"), QString("game"));
    sendMessage(message, QString(), QString(), QString());

    connect(&d->handshake_timer, SIGNAL(timeout()), this, SLOT(handshakeTimeout()));
    d->handshake_timer.start(HANDSHAKE_TIMEOUT_MS);
}

void BasController::handshakeTimeout(void)
{
    if(d->connection_state.empty())
    {
        qDebug() << "pid:" << QCoreApplication::applicationPid() << ": oops we couldn't connect";
        exit(1);
    }

    for(auto key : d->connection_state.keys())
    {
        if(d->connection_state[key] != ACLMessage::HANDSHAKE_ACK)
        {
            qDebug() << "pid:" << QCoreApplication::applicationPid() << ": oops we couldn't connect";
            exit(1);
        }
    }
    d->handshake_timer.stop();
    d->accepting_new_connections = false;
    emit finishInitialization();
}

void BasController::lock(void)
{
    d->mutex.lock();
}

void BasController::unlock(void)
{
    d->mutex.unlock();
}

void BasController::sendLocalPlayerAck(QString local_player)
{
    ACLMessage ack_message(ACLMessage::HANDSHAKE_ACK);
    ack_message.setSender(QCoreApplication::applicationPid());
    ack_message.addContent("state", local_player);
    ack_message.addContent(QString("header_who"), QString("game"));
    sendMessage(ack_message, QString(), QString(), QString());
}

void BasController::sendPlayerUpdate(int site_id, QString player_state)
{
    ACLMessage message(ACLMessage::UPDATE);
    message.setSender(QCoreApplication::applicationPid());
    message.addContent("site_id", QString::number(site_id));
    message.addContent("state", player_state);
    message.addContent(QString("header_who"), QString("game"));
    sendMessage(message, QString(), QString(), QString());
}

void BasController::slotReceiveMessage(Header header, Message message)
{
    if(message.getContents()["header_who"] == "game")
    {
        ACLMessage acl_message = *(static_cast<ACLMessage*>(&message));
        switch (acl_message.getPerformative())
        {
            case ACLMessage::MUTEX_REQUEST:
            {
                d->mutex.requestUpdate(acl_message.getTimeStamp());
                break;
            }
            case ACLMessage::MUTEX_ACKNOWLEDGE:
            {
                d->mutex.acknowledgeUpdate(acl_message.getTimeStamp());
                break;
            }
            case ACLMessage::MUTEX_LIBERATION:
            {
                d->mutex.liberationUpdate(acl_message.getTimeStamp());
                break;
            }

            case ACLMessage::HANDSHAKE_SYN:
            {
                if(d->accepting_new_connections)
                {
                    if(!d->connection_state.contains(acl_message.getSender()))
                    {
                        d->connection_state[acl_message.getSender()] = ACLMessage::HANDSHAKE_SYN;
                        d->mutex.addPlayerConnection(acl_message.getSender());
                        emit getLocalPlayerForAck();
                    }
                }
                break;
            }

            case ACLMessage::HANDSHAKE_ACK:
            {
                if(d->accepting_new_connections)
                {
                    int id = acl_message.getSender();
                    d->connection_state[id] = ACLMessage::HANDSHAKE_ACK;
                    emit playerUpdateReceived(id, acl_message.getContents()["state"]);
                }
                break;
            }

            case ACLMessage::UPDATE:
            {
                ACLMessage ack_message(ACLMessage::UPDATE_ACK);
                ack_message.addContent("acked_id", QString::number(acl_message.getSender()));
                message.addContent(QString("header_who"), QString("game"));
                sendMessage(ack_message, QString(), QString(), QString());
                emit playerUpdateReceived(acl_message.getContents()["site_id"].toInt(), acl_message.getContents()["state"]);
                break;
            }
            case ACLMessage::UPDATE_ACK:
            {
                emit enterCriticalSectionEnd();
                break;
            }
            default:
                break;
        }
    }
}

void BasController::fowardMutexMessage(ACLMessage message)
{
    message.addContent(QString("header_who"), QString("game"));
    sendMessage(message, QString(), QString(), QString());
}

void BasController::notifyAccessAllowed(void)
{
    emit enterCriticalSection();
}

}
