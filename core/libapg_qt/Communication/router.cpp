#include "router.h"

// Qt includes
#include <QDebug>
#include <QThread>

namespace AirPlug
{

class Q_DECL_HIDDEN Router::Private
{
public:
    Private()
        : communicationMngr(nullptr),
          nbSequence(0),
          snapshot(nullptr)
    {
    }

    ~Private()
    {
    }

public:

    void forwardAppToNet(Header& header, ACLMessage& message);
    void forwardNetToApp(Header& header, ACLMessage& message);
    void forwardStateMessage(ACLMessage& message, bool fromLocal);

public:

    bool isOldMessage(const ACLMessage& messsage);

public:

    CommunicationManager* communicationMngr;

    // using siteID and sequence nb to identify old message
    QString               siteID;
    int                   nbSequence;

    QHash<QString, int>   recentSequences;

    LaiYangSnapshot*      snapshot;
};


void Router::Private::forwardAppToNet(Header& header, ACLMessage& message)
{
    // mark message ID
    message.setSender(siteID);
    message.setNbSequence(++nbSequence);

    QJsonObject contents =  message.getContent();

    // set App name
    contents[QLatin1String("app")] = header.what();

    if (snapshot)
    {
        snapshot->colorMessage(contents);
    }

    message.setContent(contents);

    communicationMngr->send(message, QLatin1String("NET"), QLatin1String("NET"), Header::allHost);
}

void Router::Private::forwardNetToApp(Header& header, ACLMessage& message)
{
    if (! message.getContents().contains(QLatin1String("receiver")))
    {
        // broadcast the message to another net
        communicationMngr->send(message, QLatin1String("NET"), QLatin1String("NET"), header.where());
    }
    else
    {
        // TODO routing
    }

    QJsonObject contents =  message.getContent();

    QString app = contents[QLatin1String("app")].toString();
    contents.remove(QLatin1String("app"));

    if (snapshot)
    {
        QThread::msleep(1);
        snapshot->getColor(contents);
    }

    message.setContent(contents);

    // NOTE: avoid signal lost at receiver
    QThread::msleep(1);

    communicationMngr->send(message, QLatin1String("NET"), app, Header::localHost);
}

void Router::Private::forwardStateMessage(ACLMessage& message, bool fromLocal)
{
    if (snapshot && (snapshot->processStateMessage(message, fromLocal) == false))
    {
        return;
    }

    // Forward to Network
    // mark message ID if state come from local app
    if (fromLocal)
    {
        message.setSender(siteID);
        message.setNbSequence(++nbSequence);
    }


    communicationMngr->send(message, QLatin1String("NET"), QLatin1String("NET"), Header::allHost);
}

bool Router::Private::isOldMessage(const ACLMessage& messsage)
{
    QString sender = messsage.getSender();
    int sequence   = messsage.getNbSequence();

    if (sender == siteID)
    {
        //qDebug() << siteID << "Drop round back message";

        return true;
    }

    if ( recentSequences.contains(sender) && (recentSequences[sender] >= sequence) )
    {
        // Here we suppose the channels are FIFO
        // therefore for each router, by keeing the sequence number of each site, we can identify old repeated message
        //qDebug() << siteID << ": Drop old message because recent sequence = " << recentSequences[sender] << " > this sequence = " << sequence;

        return true;
    }

    // Update recent sequence
    recentSequences[sender] = sequence;

    return false;
}


/*----------------------------------------------------------------------------------------------------------------------------------------------------*/


Router::Router(CommunicationManager* communication, const QString& siteID)
    : QObject(nullptr),
      d(new Private())
{
    if (!communication)
    {
        qFatal("Router: Communication Manager is null");
    }

    d->communicationMngr = communication;
    d->siteID            = siteID;

    setObjectName(QLatin1String("Router"));

    connect(d->communicationMngr, SIGNAL(signalMessageReceived(Header, Message)),
            this,                 SLOT(slotReceiveMessage(Header, Message)));
}

Router::~Router()
{
    delete d;
}

bool Router::addSnapshot(LaiYangSnapshot* snapshot)
{
    if (!snapshot)
    {
        return false;
    }

    d->snapshot = snapshot;

    connect(d->snapshot, &LaiYangSnapshot::signalRequestSnapshot,
            this,        &Router::slotSendMarker, Qt::DirectConnection);

    return true;
}

void Router::slotReceiveMessage(Header header, Message message)
{
    // cast message to ACL format
    ACLMessage aclMessage(*(static_cast<ACLMessage*>(&message)));

    if (header.what() == QLatin1String("NET"))
    {
        if (d->isOldMessage(aclMessage))
        {
            return;
        }

        switch (aclMessage.getPerformative())
        {
            case ACLMessage::INFORM_STATE:
                d->forwardStateMessage(aclMessage, false);

                break;

            default:
                d->forwardNetToApp(header, aclMessage);

                break;
        }
    }
    else
    {
        if (aclMessage.getPerformative() == ACLMessage::INFORM_STATE)
        {
            // process state message
            d->forwardStateMessage(aclMessage, true);
        }
        else
        {
            d->forwardAppToNet(header, aclMessage);
        }
    }
}

void Router::slotSendMarker(const Message* marker)
{
    // broadcast to all app in local site
    d->communicationMngr->send(*marker, QLatin1String("NET"), Header::allApp, Header::localHost);
}

}
