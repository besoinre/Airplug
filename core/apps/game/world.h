/* ============================================================
 *
 * This file is a part of Airplug project
 *
 * Date        : 2020-4-24
 * Description : game world class
 *
 * 2020 by Lucca Rawlyk
 *
 * ============================================================ */

#ifndef WORLD_H
#define WORLD_H

//qt includes
#include <QObject>
#include <QCoreApplication>

//std includes
#include <memory>

namespace GameApplication
{

class World : public QObject
{
    Q_OBJECT

public slots:
    void finishInitialization(void);

    void frameTimeout(void);
    void criticalSection(void);
    void criticalSectionEnd(void);

    void playerUpdateFromMessage(int site_id, QString player_state);
    void sendLocalPlayerForAck(void);

    void getState(void);

signals:
    void signalSendLocalPlayerForAck(QString local_player);
    void returnWorldState(QJsonObject world_state);

public:
    World(QCoreApplication &app);
    ~World();

private:
    class Private;
    std::unique_ptr<Private> d;
};

}
#endif // WORLD_H
