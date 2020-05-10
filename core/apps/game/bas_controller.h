/* ============================================================
 *
 * This file is a part of Airplug project
 *
 * Date        : 2020-4-24
 * Description : game airplug controller
 *
 * 2020 by Lucca Rawlyk
 *
 * ============================================================ */

#ifndef BAS_CONTROLLER_H
#define BAS_CONTROLLER_H

//qt include
#include <QObject>
#include <QCoreApplication>

//std includes
#include <memory>

//local include
#include "application_controller.h"
#include "aclmessage.h"

using namespace AirPlug;

namespace GameApplication
{

class BasController: public ApplicationController
{
    Q_OBJECT
public:

    BasController(QCoreApplication &app, QObject* parent = nullptr);
    ~BasController();

    void establishConnections(QString local_player_state);
    void lock(void);
    void unlock(void);
    void sendPlayerUpdate(int site_id, QString player_state);

signals:

    void finishInitialization(void);
    void enterCriticalSection(void);
    void enterCriticalSectionEnd(void);
    void getLocalPlayerForAck(void);
    void playerUpdateReceived(int site_id, QString player_state);

public slots:

    void slotReceiveMessage(Header, Message) override;

    void fowardMutexMessage(ACLMessage message);
    void notifyAccessAllowed(void);

    void handshakeTimeout(void);
    void sendLocalPlayerAck(QString local_player);

private:

    class Private;
    std::unique_ptr<Private> d;
};

}
#endif // BAS_CONTROLLER_H
