#include "net_controller.h"

//Qt includes
#include <QTimer>
#include <QDebug>
#include <algorithm>

// Local include
#include "laiyang_snapshot.h"
#include "router.h"
#include "mutex.h"

using namespace AirPlug;

namespace NetApplication
{

class Q_DECL_HIDDEN NetController::Private
{
public:

    Private()
        : router(nullptr)
    {
    }

    ~Private()
    {
        delete router;
    }

public:
    //LaiYangSnapshot snapshotManager;
    Router*         router;
    Mutex* mutex;
};



NetController::NetController(QObject* parent)
    : ApplicationController(QLatin1String("NET"), parent),
      d(new Private())
{
}

NetController::~NetController()
{
    delete d;
}

void NetController::init(const QCoreApplication &app)
{
    ApplicationController::init(app);

    disconnect(m_communication, SIGNAL(signalMessageReceived(Header, Message)),
               this,            SLOT(slotReceiveMessage(Header, Message)));

    d->router = new Router(m_communication, m_clock->getSiteID());
    d->mutex = new Mutex(m_clock->getSiteID());
}

void NetController::slotReceiveMessage(Header header, Message message)
{
    ACLMessage aclMessage(*(static_cast<ACLMessage*>(&message)));
    ACLMessage::Performative performative = aclMessage.getPerformative();

    // Si message viens du bas
    if(header.what() == "BAS"){


        // Demande de section critique ? OUI/NON
        if( performative == ACLMessage::REQUEST){

           //hi <- hi+1
           d->mutex->date++;
           // Tabi[i]← (requête, hi)
           Mutex::MutexData newdata;
           newdata.type = Mutex::MutexType::request;
           newdata.date =d->mutex->date;
           d->mutex->addMutexData(m_clock->getSiteID(),newdata);

           // TODO envoyer( [requête] hi) à tous les autres sites

        }else if( performative == ACLMessage::RELEASE){
            // Liberation de section critique


           d->mutex->date++;
           Mutex::MutexData newdata;
           newdata.type = Mutex::MutexType::release;
           newdata.date =d->mutex->date;
           d->mutex->addMutexData(m_clock->getSiteID(),newdata);

           // TODO envoyer( [libération] hi) à tous les autres sites

        }

    //Si message vient d'un autre net
    }else if(header.what() == "NET"){

        // Demande de type requête d'un autre net
        if( performative == ACLMessage::REQUEST){

           d->mutex->date = std::max(d->mutex->date ,aclMessage.getEstampille()) +1;

           Mutex::MutexData newdata;
           newdata.type = Mutex::MutexType::request;
           newdata.date = aclMessage.getEstampille();
           d->mutex->addMutexData(aclMessage.getSender(),newdata);

           // TODO envoyer( [accusé] hi) à Sj

           //if(d->mutex->mutexTab[m_clock->getSiteID()].type == ACLMessage::REQUEST &)

    }
        // TO DO CONTINUE
}


}
