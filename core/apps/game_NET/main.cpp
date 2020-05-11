
//Qt includes
#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>

//std includes
#include <memory>

//local includes
#include "mainwindow.h"
#include "net_controller.h"

using namespace GameNetApplication;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("game_NET");

    NetController controller;
    controller.init(app);

    std::unique_ptr<MainWindow> main_window;
    main_window = std::make_unique<MainWindow>(controller);
    main_window->setWindowTitle(QLatin1String("game_NET"));
    main_window->show();
    return app.exec();
}
