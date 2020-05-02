#include "bas_controller.h"

//Qt includes
#include <QTimer>
#include <QDebug>


using namespace AirPlug;

namespace BasApplication
{

class Q_DECL_HIDDEN BasController::Private
{
public:

    Private()
        : timer(nullptr),
          nbSequence(0)
    {
        // TODO: get default message ffrom QSettings
        messageToSend = QLatin1String("~");

        mutex         = new RicartLock();
    }

    ~Private()
    {
        delete timer;
        delete mutex;
    }

public:

    QTimer*               timer;
    QString               messageToSend;
    int                   nbSequence;

    RicartLock*           mutex;
};


/* -------------------------------------------------------------------------------------------------------------------------------------------------------------*/

BasController::BasController(QObject* parent)
    : ApplicationController(QLatin1String("BAS"), parent),
      d(new Private)
{
}

BasController::~BasController()
{
    delete d;
}

void BasController::init(const QCoreApplication& app)
{
    ApplicationController::init(app);

    // All Bas will subscribe to local NET
    m_communication->subscribeLocalHost(QLatin1String("NET"));

    if (m_optionParser.autoSend && m_optionParser.delay > 0)
    {
        slotActivateTimer(m_optionParser.delay);
    }

    connect(d->mutex, &RicartLock::signalResponse,
            this, &BasController::slotForwardMutex, Qt::DirectConnection);

    connect(d->mutex, &RicartLock::signalEnterRaceCondition,
            this, &BasController::slotEnterCriticalSection, Qt::DirectConnection);
}

void BasController::pause(bool b)
{
    m_optionParser.start = !b;

    if (b)
    {
        slotDeactivateTimer();
    }
}

void BasController::setMessage(const QString& msg)
{
    d->messageToSend = msg;

    ++(*m_clock);
}

void BasController::slotActivateTimer(int period)
{
    if (! d->timer)
    {
        d->timer = new QTimer(this);

        connect(d->timer, &QTimer::timeout,
                    this, &BasController::slotSendMessage);
    }

    m_optionParser.delay    = period;
    m_optionParser.autoSend = true;

    d->timer->start(period);

    ++(*m_clock);
}

void BasController::slotDeactivateTimer()
{
    m_optionParser.autoSend = false;
    m_optionParser.delay    = 0;

    if (d->timer)
    {
        d->timer->stop();
    }

    ++(*m_clock);
}

void BasController::slotPeriodChanged(int period)
{
    m_optionParser.delay = period;

    if (d->timer)
    {
        d->timer->setInterval(period);
    }

    ++(*m_clock);
}

void BasController::slotSendMessage()
{
    d->mutex->trylock((*m_clock));
}

void BasController::slotReceiveMessage(Header header, Message message)
{
    ACLMessage aclMessage(*(static_cast<ACLMessage*>(&message)));

    if (aclMessage.getPerformative() == ACLMessage::REQUEST_SNAPSHOT)
    {
        qDebug() << siteID() << "record snapshot";

        QJsonObject localState = captureLocalState();

        aclMessage.setPerformative(ACLMessage::INFORM_STATE);
        aclMessage.setTimeStamp(*m_clock);
        aclMessage.setContent(localState);

        sendMessage(aclMessage, QString(), QString(), QString());

        return;
    }
    else if (aclMessage.getPerformative() == ACLMessage::PING)
    {
        aclMessage.setPerformative(ACLMessage::PONG);
        sendMessage(aclMessage, QString(), QString(), QString());

        return;
    }
    else if (aclMessage.getPerformative() == ACLMessage::REQUEST_MUTEX)
    {
        if (aclMessage.getTimeStamp()->getSiteID() != siteID())
        {
            qDebug() << siteID() << "receives mutex request";
            receiveMutexRequest(aclMessage);
        }
    }
    else if (aclMessage.getPerformative() == ACLMessage::ACCEPT_MUTEX)
    {
        qDebug() << siteID() << "enter race condition";
        d->mutex->lock();
    }

    VectorClock* senderClock = aclMessage.getTimeStamp();

    // read sender's clock
    if (senderClock)
    {
        m_clock->updateClock(*senderClock);
    }

    emit signalMessageReceived(header, message);
}

QJsonObject BasController::captureLocalState() const
{
    ++(*m_clock);

    QJsonObject applicationState;

    applicationState[QLatin1String("messageToSend")] = d->messageToSend;
    applicationState[QLatin1String("nbSequence")] = d->nbSequence;

    QJsonObject localState = m_clock->convertToJson();
    localState[QLatin1String("options")] = m_optionParser.convertToJson();
    localState[QLatin1String("state")]   = applicationState;

    return localState;
}

void BasController::receiveMutexRequest(const ACLMessage& request) const
{
    VectorClock* senderClock = request.getTimeStamp();

    m_clock->updateClock(*senderClock);

    d->mutex->receiveExternalRequest(*senderClock);
}

void BasController::slotForwardMutex(const ACLMessage& message)
{
    // broadcast request to enter race condition
    sendMessage(message, QString(), QString(), QString());
}

void BasController::slotEnterCriticalSection()
{
    ++(*m_clock);

    ACLMessage message(ACLMessage::INFORM);

    // attache clock to the message
    message.setTimeStamp(*m_clock);

    QJsonObject contents;

    ++d->nbSequence;

    contents[QLatin1String("payload")] =  d->messageToSend;
    contents[QLatin1String("nseq")] =  d->nbSequence;

    message.setContent(contents);

    // TODO: get what, where, who from user interface
    sendMessage(message, QString(), QString(), QString());

    emit signalSequenceChange(d->nbSequence);

    d->mutex->unlock();
}


}
