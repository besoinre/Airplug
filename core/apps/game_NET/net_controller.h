#ifndef NET_CONTROLLER_H
#define NET_CONTROLLER_H

// Qt include
#include <QObject>

//std includes
#include <memory>

// Local include
#include "application_controller.h"


using namespace AirPlug;

namespace GameNetApplication
{

class NetController: public ApplicationController
{
    Q_OBJECT
public:

    NetController(QObject* parent = nullptr);
    ~NetController();

    void init(const QCoreApplication &app) override;

    void takeSnapshot() const;

private:

    // main notification handler
    Q_SLOT void slotReceiveMessage(Header, Message) override;

private:

    class Private;
    std::shared_ptr<Private> d;
};

}
#endif // NET_CONTROLLER_H
