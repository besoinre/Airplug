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

using namespace AirPlug;

namespace GameApplication
{

class BasController: public ApplicationController
{
    Q_OBJECT
public:

    // BasController(QCoreApplication &app, QObject* parent = nullptr);
    // ~BasController();
    //
    // void establishConnections(QString mp_state);
    // void sendPlayerUpdate(QString mp_state);
    // void sendCollisionUpdate(int player_index, QString player_state);

signals:
    // void updatePlayer(int player_index, QString player_state);
    // void updateMainPlayer(QString mp_state);
    // void finishInitialization(void);
    void enterCriticalSection(void);
    void getLocalPlayerForAck(void);

public slots:

    void slotReceiveMessage(Header, Message) override;
    // void handshakeTimeout(void);
    void fowardMutexMessage(const ACLMessage& message);
    void notifyAccessAllowed(void);
    void sendLocalPlayerAck(QString local_player);

private:

    class Private;
    std::unique_ptr<Private> d;
};

}
#endif // BAS_CONTROLLER_H
