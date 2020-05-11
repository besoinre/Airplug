#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSpinBox>
#include <QDebug>

using namespace Ui;

namespace GameNetApplication
{

MainWindow::MainWindow(NetController& controller, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_controller(controller)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_snapshotButton_clicked()
{
    m_controller.takeSnapshot();
}


}
