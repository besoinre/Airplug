#include "aclmessage.h"

namespace AirPlug
{

ACLMessage::ACLMessage(Performative performative)
    : Message()
{
    addContent(QLatin1String("perfomative"), QString::number(performative));
}

ACLMessage::ACLMessage(const QString& message)
    : Message(message)
{
}

ACLMessage::~ACLMessage()
{
}

void ACLMessage::setPerformative(Performative performative)
{
    addContent(QLatin1String("perfomative"), QString::number(performative));
}

void ACLMessage::setContent(QJsonObject& content)
{
    addContent(QLatin1String("content"), QJsonDocument(content).toJson(QJsonDocument::Compact));
}

void ACLMessage::setTimeStamp(TimeStamp& time_stamp)
{
    addContent(QLatin1String("time_stamp"), QJsonDocument(time_stamp.convertToJson()).toJson(QJsonDocument::Compact));
}

void ACLMessage::setSender(int& siteID)
{
    addContent(QLatin1String("sender"), QString::number(siteID));
}

ACLMessage::Performative ACLMessage::getPerformative()
{
    return static_cast<Performative>(getContents()[QLatin1String("perfomative")].toInt());
}

std::shared_ptr<TimeStamp> ACLMessage::getTimeStamp()
{
    QHash<QString, QString> contents = getContents();

    if (contents.contains(QLatin1String("time_stamp")))
    {
        QJsonObject jsonTimeStamp =  QJsonDocument::fromJson(contents[QLatin1String("time_stamp")].toUtf8()).object();

        return std::make_shared<TimeStamp>(jsonTimeStamp);
    }

    return nullptr;
}

QJsonObject ACLMessage::getContent()
{
    return QJsonDocument::fromJson(getContents()[QLatin1String("content")].toUtf8()).object();
}

int ACLMessage::getSender()
{
    return getContents()[QLatin1String("sender")].toInt();
}

QJsonObject ACLMessage::toJsonObject()
{
    QHash<QString, QString> contents = getContents();

    QJsonObject json;

    json[QLatin1String("perfomative")] = contents[QLatin1String("perfomative")];
    json[QLatin1String("sender")]      = contents[QLatin1String("sender")];

    if (contents.contains(QLatin1String("time_stamp")))
    {
        json[QLatin1String("time_stamp")] = QJsonDocument::fromJson(contents[QLatin1String("time_stamp")].toUtf8()).object();
    }

    json[QLatin1String("content")]     = QJsonDocument::fromJson(getContents()[QLatin1String("content")].toUtf8()).object();

    return json;
}

}
