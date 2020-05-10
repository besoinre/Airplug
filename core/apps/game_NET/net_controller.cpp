#include "net_controller.h"

//Qt includes
#include <QDebug>
#include <QMap>
#include <QCoreApplication>

// Local include

using namespace AirPlug;

namespace GameNetApplication
{

class Q_DECL_HIDDEN NetController::Private
{
public:

    Private()
    {
    }

    ~Private()
    {

    }

public:
    QMap<int, int> id_nseq_map;
    QMap<int, std::shared_ptr<QMap<int, Message>>> future_message;
    int net_id;
    int sequence_number = 0;
};



NetController::NetController(QObject* parent)
    : ApplicationController(QString("NET"), parent),
      d(std::make_shared<Private>())
{

}

NetController::~NetController()
{

}

void NetController::init(const QCoreApplication &app)
{
    ApplicationController::init(app);
    d->net_id = QCoreApplication::applicationPid();
}

void NetController::slotReceiveMessage(Header header, Message message)
{
    auto contents = message.getContents();
    if(message.getContents()["header_who"] == "game" && !contents.contains(QString("net_id")))
    {
        message.addContent("net_id", QString::number(d->net_id));
        message.addContent("sequence_number", QString::number(d->sequence_number));
        d->sequence_number++;
        message.addContent(QString("header_who"), QString("game_NET"));
        sendMessage(message, QString(), QString(), QString());
    }
    else if(message.getContents()["header_who"] == "game_NET")
    {
        int sender_net_id = contents["net_id"].toInt();
        int sender_nseq = contents["sequence_number"].toInt();
        if(!(sender_net_id == d->net_id))
        {
            if(!d->id_nseq_map.contains(sender_net_id))
            {
                d->id_nseq_map[sender_net_id] = sender_nseq;
                message.addContent(QString("header_who"), QString("game"));
                sendMessage(message, QString(), QString(), QString());
            }
            else if(sender_nseq > d->id_nseq_map[sender_net_id])
            {
                if(sender_nseq == d->id_nseq_map[sender_net_id] + 1)
                {
                    bool send_message_queue = false;
                    do
                    {
                        //foward to other NETs
                        message.addContent(QString("header_who"), QString("game_NET"));
                        sendMessage(message, QString(), QString("game"), QString());
                        //foward to game
                        message.removeContent(QString("header_who"));
                        message.addContent(QString("header_who"), QString("game"));
                        sendMessage(message, QString(), QString(), QString());
                        d->id_nseq_map[sender_net_id]++;
                        // check if there are future messages in the queue
                        if(d->future_message.contains(sender_net_id))
                        {
                            if((d->future_message[sender_net_id])->contains(d->id_nseq_map[sender_net_id]+1))
                            {
                                message = (*d->future_message[sender_net_id])[d->id_nseq_map[sender_net_id]+1];
                                d->future_message[sender_net_id]->remove(d->id_nseq_map[sender_net_id]+1);
                                send_message_queue = true;
                            }
                            else
                                send_message_queue = false;
                        }
                        else
                            send_message_queue = false;
                    }
                    while(send_message_queue);
                }
                else
                {
                    if(!d->future_message.contains(sender_net_id))
                    {
                        d->future_message[sender_net_id] = std::make_shared<QMap<int, Message>>();
                    }
                    (*d->future_message[sender_net_id])[sender_nseq] = message;
                }
            }
        }
    }
}

}
