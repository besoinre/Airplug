#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

//local include
#include "net_controller.h"

namespace Ui
{
    class MainWindow;
}

namespace GameNetApplication
{

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(NetController& controller, QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_snapshotButton_clicked();

private:
    Ui::MainWindow *ui;

    NetController& m_controller;
};

}
#endif // MAINWINDOW_H
