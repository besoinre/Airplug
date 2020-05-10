
//Qt includes
#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>

//local includes
#include "net_controller.h"

using namespace GameNetApplication;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("game_NET");

    NetController controller;
    controller.init(app);

    return app.exec();
}
