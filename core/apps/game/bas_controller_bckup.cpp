#include "bas_controller.h"

//Qt includes
#include <QCoreApplication>
#include <QTimer>
#include <QMap>
#include <QPair>
#include <QDebug>

//local includes
#include "constants.h"

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

    QTimer handshake_timer;
    bool handshake_timeout = false;
    bool accepting_new_connections = true;
    QMap<int,QPair<int,QString>> connection_state;
    int established_connections = 0;
};



BasController::BasController(QCoreApplication &app, QObject* parent)
    : ApplicationController(QString("game"), parent),
      d(std::make_unique<Private>())
{
    ApplicationController::init(app);
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
    QHash<QString, QString> contents = message.getContents();
    qDebug() << QCoreApplication::applicationPid() << ": message received";
    if(contents.contains(QString("action")))
    {
        if(contents[QString("action")] == "SYN")
        {
            qDebug() << QCoreApplication::applicationPid() << ": it's a syn message";
            if(d->accepting_new_connections)
            {
                if(contents.contains(QString("id")) && contents.contains(QString("state")))
                {
                    int id = contents[QString("id")].toInt();
                    if(!d->connection_state.contains(id))
                    {
                        d->connection_state[id] = QPair<int, QString>(d->established_connections, contents[QString("action")]);
                        d->established_connections++;
                        emit updatePlayer(d->connection_state[id].first, contents[QString("state")]);

                        Message ack_message;
                        ack_message.addContent(QString("action"), QString("ACK"));
                        ack_message.addContent(QString("id"), QString::number(QCoreApplication::applicationPid()));
                        qDebug() << QCoreApplication::applicationPid() << ": sending ack message";
                        sendMessage(ack_message, QString(), QString(), QString());
                    }
                }
            }
        }
        else if(contents[QString("action")] == "ACK")
        {
            qDebug() << QCoreApplication::applicationPid() << ": it's an ack message";
            if(d->accepting_new_connections)
            {
                if(contents.contains(QString("id")))
                {
                    int id = contents[QString("id")].toInt();
                    if(d->connection_state.contains(id))
                        d->connection_state[id].second = "ACK";
                }
            }
        }
        else if(contents[QString("action")] == "PLU")
        {
            qDebug() << QCoreApplication::applicationPid() << ": it's a plu message";
            if(contents.contains(QString("id")) && contents.contains(QString("state")))
            {
                int id = contents[QString("id")].toInt();
                if(d->connection_state.contains(id))
                {
                    emit updatePlayer(d->connection_state[id].first, contents[QString("state")]);
                }
            }
        }
        else if(contents[QString("action")] == "MPU")
        {
            qDebug() << QCoreApplication::applicationPid() << ": it's a mpu message";
            if(contents.contains(QString("state")))
            {
                emit updateMainPlayer(contents[QString("state")]);
            }
        }
    }
}

}
