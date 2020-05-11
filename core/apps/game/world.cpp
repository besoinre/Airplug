#include "world.h"

//qt include
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QBrush>
#include <QTimer>
#include <QLinearGradient>
#include <QColor>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QDebug>

//std includes
#include <vector>
#include <cmath>
#include <random>
#include <QList>

//local include
#include "state.h"
#include "player.h"
#include "constants.h"
#include "bas_controller.h"

namespace GameApplication
{

class Q_DECL_HIDDEN World::Private
{
public:
    Private(QCoreApplication &app) : view(&scene), bas_controller(app)
    {

    }
    ~Private()
    {

    }

    QGraphicsScene scene;
    QGraphicsView view;

    Player local_player;
    QMap<int, std::shared_ptr<Player>> connected_player;

    QTimer frame_timer;
    int frame = 0;
    int timeouts = 0;
    bool lock_demanded = false;

    void moveAndUpdatePlayer(Player &player);
    void fixCollisions(Player &player);

    bool got_collision_ack = false;
    BasController bas_controller;
};

World::World(QCoreApplication &app) : d(std::make_unique<Private>(app))
{
    connect(&d->bas_controller, SIGNAL(getLocalPlayerForAck()), this, SLOT(sendLocalPlayerForAck()));
    connect(this, SIGNAL(signalSendLocalPlayerForAck(QString)), &d->bas_controller, SLOT(sendLocalPlayerAck(QString)));
    connect(&d->bas_controller, SIGNAL(finishInitialization()), this, SLOT(finishInitialization()));

    connect(&d->bas_controller, SIGNAL(enterCriticalSection()), this, SLOT(criticalSection()));
    connect(&d->bas_controller, SIGNAL(enterCriticalSectionEnd()), this, SLOT(criticalSectionEnd()));
    connect(&d->bas_controller, SIGNAL(playerUpdateReceived(int, QString)), this, SLOT(playerUpdateFromMessage(int, QString)));

    connect(&d->bas_controller, SIGNAL(getWorldState()), this, SLOT(getState()));
    connect(this, SIGNAL(returnWorldState(QJsonObject)), &d->bas_controller, SLOT(receivedWorldState(QJsonObject)));

    //initializing local_player
    d->local_player.setRect(0,0,PLAYER_SIZE,PLAYER_SIZE);

    QLinearGradient linear_grad(QPointF(0, 0), QPointF(PLAYER_SIZE, PLAYER_SIZE));
    linear_grad.setColorAt(0, Qt::red);
    linear_grad.setColorAt(0.35, Qt::yellow);
    linear_grad.setColorAt(0.5, Qt::green);
    linear_grad.setColorAt(0.65, Qt::cyan);
    linear_grad.setColorAt(1, Qt::blue);
    d->local_player.setBrush(QBrush(linear_grad));

    d->local_player.setFlag(QGraphicsItem::ItemIsFocusable);
    d->local_player.setFocus();

    d->scene.addItem(&d->local_player);

    d->scene.setSceneRect(0, 0, VIEW_WIDTH*2, VIEW_HEIGHT*2);

    std::random_device seeder{};
    std::mt19937 twister{seeder()};
    std::uniform_int_distribution<> x_dis(0, VIEW_WIDTH-1-PLAYER_SIZE);
    std::uniform_int_distribution<> y_dis(0, VIEW_HEIGHT-1-PLAYER_SIZE);

    d->local_player.setPos(x_dis(twister), y_dis(twister));

    d->bas_controller.establishConnections(d->local_player.getState().toJsonString());
}

void World::finishInitialization(void)
{
    connect(&d->frame_timer, SIGNAL(timeout()), this, SLOT(frameTimeout()));
    d->frame_timer.start(FRAME_PERIOD_MS);

    d->view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->view.setFixedSize(VIEW_WIDTH, VIEW_HEIGHT);
    d->view.show();
}

World::~World()
{

}

void World::getState(void)
{
    QJsonObject world;
    world["local_player"] = QJsonDocument::fromJson(d->local_player.getState().toJsonString().toUtf8()).object();
    for(auto key : d->connected_player.keys())
    {
        world[QString::number(key)] = QJsonDocument::fromJson(d->connected_player[key]->getState().toJsonString().toUtf8()).object();
    }

    emit returnWorldState(world);
}

void World::frameTimeout(void)
{
    d->frame_timer.stop();
    d->bas_controller.lock();
    d->local_player.setFocus();
}

void World::criticalSection(void)
{
    for(auto key : d->connected_player.keys())
        if(d->connected_player[key]->getState().frame < d->frame)
            return;
    d->local_player.incrementFrame();
    d->frame++;
    d->moveAndUpdatePlayer(d->local_player);
    d->fixCollisions(d->local_player);
    d->bas_controller.sendPlayerUpdate(QCoreApplication::applicationPid(), d->local_player.getState().toJsonString());
    QTimer::singleShot(UPDATE_ACK_TIMEOUT, this, SLOT(criticalSectionEnd()));
}

void World::criticalSectionEnd(void)
{
    d->bas_controller.unlock();
    d->frame_timer.start(FRAME_PERIOD_MS);
}

void World::sendLocalPlayerForAck(void)
{
    emit signalSendLocalPlayerForAck(d->local_player.getState().toJsonString());
}

void World::playerUpdateFromMessage(int site_id, QString player_state)
{
    if(site_id == QCoreApplication::applicationPid())
    {
        QJsonObject obj = QJsonDocument::fromJson(player_state.toUtf8()).object();
        State new_state;
        new_state.loadFromJson(obj);
        d->local_player.setState(new_state);
    }
    else
    {
        if(!d->connected_player.contains(site_id))
        {
            std::random_device seeder{};
            std::mt19937 twister{seeder()};
            std::vector<QColor> color = {Qt::white,Qt::black,Qt::cyan,Qt::red,Qt::magenta,Qt::green,Qt::yellow,Qt::blue,Qt::gray};
            std::uniform_int_distribution<> color_dis(0, color.size()-1);

            d->connected_player[site_id] = std::make_shared<Player>();
            d->connected_player[site_id]->setRect(0,0,PLAYER_SIZE,PLAYER_SIZE);
            d->connected_player[site_id]->setBrush(QBrush(color[color_dis(twister)]));
            d->scene.addItem(d->connected_player[site_id].get());
        }

        QJsonObject obj = QJsonDocument::fromJson(player_state.toUtf8()).object();
        State new_state;
        new_state.loadFromJson(obj);
        d->connected_player[site_id]->setState(new_state);
    }
}

void World::Private::moveAndUpdatePlayer(Player &player)
{
    State state = player.getState();

    //apply speed modifications from controls
    if(state.left == true)
        state.x_speed -= X_CONTROL_ACCELERATION;
    else if(state.right == true)
        state.x_speed += X_CONTROL_ACCELERATION;

    if(abs(state.x_speed) > X_SPEED_LIMIT)
        state.x_speed = (state.x_speed > 0) ? X_SPEED_LIMIT : -1*X_SPEED_LIMIT;

    if(state.up == true)
        state.y_speed -= Y_CONTROL_ACCELERATION;
    else if(state.down == true)
        state.y_speed += Y_CONTROL_ACCELERATION;

    if(abs(state.y_speed) > Y_SPEED_LIMIT)
        state.y_speed = (state.y_speed >= 0) ? Y_SPEED_LIMIT : -1*Y_SPEED_LIMIT;

    //apply speed reduction from friction
    if(state.left == false && state.right == false && state.x_speed != 0)
    {
        if(state.x_speed > 0)
        {
            state.x_speed -= X_FRICTION;
            if(state.x_speed < 0)
                state.x_speed = 0;
        }
        else
        {
            state.x_speed += X_FRICTION;
            if(state.x_speed > 0)
                state.x_speed = 0;
        }
    }

    if(state.up == false && state.down == false && state.y_speed != 0)
    {
        if(state.y_speed > 0)
        {
            state.y_speed -= Y_FRICTION;
            if(state.y_speed < 0)
                state.y_speed = 0;
        }
        else
        {
            state.y_speed += Y_FRICTION;
            if(state.y_speed > 0)
                state.y_speed = 0;
        }
    }

    //write changes
    player.setSpeed(state.x_speed, state.y_speed);
    player.setPos(state.x + state.x_speed, state.y + state.y_speed);
}

void World::Private::fixCollisions(Player &player)
{
    //lazy fixing, might need to be improved
    QList<QGraphicsItem*> colliding_items = player.collidingItems();
    bool collided = false;
    while(colliding_items.size() != 0)
    {
        collided = true;
        auto col = static_cast<Player*>(colliding_items[0]);
        int collided_player_id = -1;
        for(auto key : connected_player.keys())
            if(col == connected_player[key].get())
                collided_player_id = key;

        State p_state = player.getState();
        if(p_state.x_speed > 0)
        {
            if(p_state.y_speed > 0)
            {
                player.setPos(player.x()-1, player.y()-1);
                col->setPos(col->x()+1, col->y()+1);
            }
            else if(p_state.y_speed < 0)
            {
                player.setPos(player.x()-1, player.y()+1);
                col->setPos(col->x()+1, col->y()-1);
            }
            else
            {
                player.setPos(player.x()-1, player.y());
                col->setPos(col->x()+1, col->y());
            }
        }
        else if(p_state.x_speed < 0)
        {
            if(p_state.y_speed > 0)
            {
                player.setPos(player.x()+1, player.y()-1);
                col->setPos(col->x()-1, col->y()+1);
            }
            else if(p_state.y_speed < 0)
            {
                player.setPos(player.x()+1, player.y()+1);
                col->setPos(col->x()-1, col->y()-1);
            }
            else
            {
                player.setPos(player.x()+1, player.y());
                col->setPos(col->x()-1, col->y());
            }
        }
        else
        {
            if(p_state.y_speed > 0)
            {
                player.setPos(player.x(), player.y()-1);
                col->setPos(col->x(), col->y()+1);
            }
            else
            {
                player.setPos(player.x(), player.y()+1);
                col->setPos(col->x(), col->y()-1);
            }
        }
        col->setSpeed(0,0);
        bas_controller.sendPlayerUpdate(collided_player_id, col->getState().toJsonString());
        colliding_items = player.collidingItems();
    }
    if(collided)
    {
        player.setSpeed(0,0);
    }
}

}
