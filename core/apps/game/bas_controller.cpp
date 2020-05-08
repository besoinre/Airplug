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
    bool handshake_timeout = false;
    bool accepting_new_connections = true;
    QMap<int, ACLMessage::Performative> connection_state;
    int established_connections = 0;
};



BasController::BasController(QCoreApplication &app, QObject* parent)
    : ApplicationController(QString("game"), parent),
      d(std::make_unique<Private>())
{
    ApplicationController::init(app);

    //connect mutex signals
    connect(&d->mutex, SIGNAL(signalResponse()), this, SLOT(fowardMutexMessage()));
    connect(&d->mutex, SIGNAL(accessAllowed()), this, SLOT(notifyAccessAllowed()));
}

BasController::~BasController()
{

}

void BasController::establishConnections(QString mp_state)
{
    Message message;
    message.addContent(QString("action"), QString("SYN"));
    message.addContent(QString("id"), QString::number(QCoreApplication::applicationPid()));
    message.addContent(QString("state"), mp_state);
    sendMessage(message, QString(), QString(), QString());

    connect(&d->handshake_timer, SIGNAL(timeout()), this, SLOT(handshakeTimeout()));
    d->handshake_timer.start(HANDSHAKE_TIMEOUT_MS);
}

void BasController::handshakeTimeout(void)
{
    for(auto key : d->connection_state.keys())
    {
        if(d->connection_state[key].second != QString("ACK"))
        {
            qDebug() << QCoreApplication::applicationPid() << ": shit negro, we couldn't connect";
            exit(1);
        }
    }
    d->handshake_timer.stop();
    d->accepting_new_connections = false;
    emit finishInitialization();
}

void BasController::sendPlayerUpdate(QString mp_state)
{
    Message message;
    message.addContent(QString("action"), QString("PLU"));
    message.addContent(QString("id"), QString::number(QCoreApplication::applicationPid()));
    message.addContent(QString("state"), mp_state);
    sendMessage(message, QString(), QString(), QString());
}

void BasController::sendCollisionUpdate(int player_index, QString player_state)
{
    Message message;
    message.addContent(QString("action"), QString("MPU"));
    message.addContent(QString("id"), QString::number(QCoreApplication::applicationPid()));
    message.addContent(QString("state"), player_state);
    sendMessage(message, QString(), QString(), QString());
}

void BasController::slotReceiveMessage(Header header, Message message)
{
    ACLMessage* aclMessage = (static_cast<ACLMessage*>(&message));

    switch (aclMessage->getPerformative())
    {
        case ACLMessage::MUTEX_REQUEST:
            d->mutex.requestUpdate(ACLMessage.getTimeStamp());
            break;
        case ACLMessage::MUTEX_ACKNOWLEDGE:
            d->mutex.acknowlegdeUpdate(ACLMessage.getTimeStamp());
            break;
        case ACLMessage::MUTEX_LIBERATION:
            d->mutex.liberationUpdate(ACLMessage.getTimeStamp());
            break;

        case ACLMessage::HANDSHAKE_SYN:
            //TODO - verifier si c'est le premier syn recu du site
            emit getLocalPlayerAck();
            break;
        case ACLMessage::HANDSHAKE_ACK:
        default:
            break;
    }
}

void BasController::fowardMutexMessage(const ACLMessage& message)
{
    sendMessage(message, QString(), QString(), QString());
}

void BasController::notifyAccessAllowed(void)
{
    emit enterCriticalZone();
}

void sendLocalPlayerAck(QString local_player)
{
    ACLMessage ack_message(ACLMessage::HANDSHAKE_ACK);
    ack_message.setSender(QCoreApplication::applicationPid());
    ack_message.addContent("local_player", local_player);
    sendMessage(message, QString(), QString(), QString());
}

}
